// servidor_ext.c
// Extensión no invasiva del servidor base: hilos por sala que hacen broadcast.
// Requiere el mismo entorno (System V message queues) y las mismas estructuras.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define MAX_SALAS 10
#define MAX_USUARIOS_POR_SALA 20
#define MAX_TEXTO 256
#define MAX_NOMBRE 50

// ====== Copia de estructuras del base (no modificar) ======
struct mensaje
{
    long mtype; // Tipo de mensaje
    char remitente[MAX_NOMBRE];
    char texto[MAX_TEXTO];
    char sala[MAX_NOMBRE];
};

struct sala
{
    char nombre[MAX_NOMBRE];
    int cola_id; // ID de la cola de mensajes de la sala
    int num_usuarios;
    char usuarios[MAX_USUARIOS_POR_SALA][MAX_NOMBRE];
};

// ====== Estado global (mismo espíritu que el base) ======
static struct sala salas[MAX_SALAS];
static int num_salas = 0;

static int cola_global = -1;

// ====== Utilidades ======
static int buscar_sala(const char *nombre)
{
    for (int i = 0; i < num_salas; i++)
    {
        if (strcmp(salas[i].nombre, nombre) == 0)
        {
            return i;
        }
    }
    return -1;
}

// EXTENSIÓN: crear sala **con proj_id** derivado del nombre numérico
// para alinear con el cliente que hace: key_t k = ftok("/tmp", atoi(sala));
static int crear_sala_con_proj(const char *nombre)
{
    if (num_salas >= MAX_SALAS)
        return -1;

    // Validar que sea número positivo
    char *end = NULL;
    long proj = strtol(nombre, &end, 10);
    if (end == nombre || *end != '\0' || proj <= 0 || proj > 255)
    {
        // Si no es numérica válida, caer en un proj incremental
        proj = num_salas + 1;
    }

    key_t key = ftok("/tmp", (int)proj);
    int cola_id = msgget(key, IPC_CREAT | 0666);
    if (cola_id == -1)
    {
        perror("Error al crear la cola de la sala");
        return -1;
    }

    strcpy(salas[num_salas].nombre, nombre);
    salas[num_salas].cola_id = cola_id;
    salas[num_salas].num_usuarios = 0;

    int idx = num_salas;
    num_salas++;
    return idx;
}

static int agregar_usuario_a_sala(int indice_sala, const char *nombre_usuario)
{
    if (indice_sala < 0 || indice_sala >= num_salas)
        return -1;
    struct sala *s = &salas[indice_sala];

    if (s->num_usuarios >= MAX_USUARIOS_POR_SALA)
        return -1;

    for (int i = 0; i < s->num_usuarios; i++)
    {
        if (strcmp(s->usuarios[i], nombre_usuario) == 0)
        {
            return 0; // ya estaba inscrito: no es error
        }
    }
    strcpy(s->usuarios[s->num_usuarios], nombre_usuario);
    s->num_usuarios++;
    return 0;
}

static void reenviar_a_todos_en_sala(int indice_sala, struct mensaje *msg)
{
    if (indice_sala < 0 || indice_sala >= num_salas)
        return;
    struct sala *s = &salas[indice_sala];

    
    for (int i = 0; i < s->num_usuarios; i++)
    {
        if (msgsnd(s->cola_id, msg, sizeof(struct mensaje) - sizeof(long), 0) == -1)
        {
            perror("Error al enviar mensaje a la sala");
        }
    }
}

// ====== Hilo de escucha por sala ======
struct hilo_args
{
    int indice_sala;
};

static void *hilo_sala(void *arg)
{
    struct hilo_args *ha = (struct hilo_args *)arg;
    int idx = ha->indice_sala;
    free(ha);

    if (idx < 0 || idx >= num_salas)
        return NULL;

    int q = salas[idx].cola_id;
    char nombre_sala[MAX_NOMBRE];
    strcpy(nombre_sala, salas[idx].nombre);

    struct mensaje msg;

    // Este hilo escucha **mensajes de tipo 3 (MSG)** que los clientes envían a la cola de la sala
    // y luego hace broadcast reenviando N copias a la misma cola.
    while (1)
    {
        // recibir cualquier tipo, luego filtramos; o directamente tipo=3:
        ssize_t r = msgrcv(q, &msg, sizeof(struct mensaje) - sizeof(long), 0, 0);
        if (r == -1)
        {
            if (errno == EINTR)
                continue;
            perror("msgrcv sala");
            usleep(100000);
            continue;
        }

        if (msg.mtype == 3)
        {
            // Asegurar que la sala en el mensaje coincide (defensivo)
            if (strcmp(msg.sala, nombre_sala) != 0)
            {
                // si no coincide, podemos normalizarla:
                strncpy(msg.sala, nombre_sala, MAX_NOMBRE - 1);
                msg.sala[MAX_NOMBRE - 1] = '\0';
            }
            // Reenviar a todos (incluye al emisor; el cliente filtrará su propio remitente)
            reenviar_a_todos_en_sala(idx, &msg);
        }
        else
        {
            // Si llegara otro tipo, lo ignoramos o lo re-inyectamos si aplica.
            // En esta extensión, lo ignoramos.
        }
    }
    return NULL;
}

// ====== Hilo principal: atiende JOIN por cola global ======
int main(void)
{
    // Cola global misma clave que el cliente usa (ftok("/tmp", 'A')):
    key_t key_global = ftok("/tmp", 'A');
    cola_global = msgget(key_global, IPC_CREAT | 0666);
    if (cola_global == -1)
    {
        perror("Error al crear la cola global");
        exit(1);
    }

    printf("Servidor de chat (ext) iniciado. Esperando clientes...\n");

    struct mensaje msg;

    while (1)
    {
        // Espera JOIN (mtype==1) u otros futuros mensajes de control
        if (msgrcv(cola_global, &msg, sizeof(struct mensaje) - sizeof(long), 0, 0) == -1)
        {
            perror("Error al recibir en cola_global");
            continue;
        }

        if (msg.mtype == 1)
        {
            // JOIN
            printf("JOIN: sala '%s' por '%s'\n", msg.sala, msg.remitente);

            int idx = buscar_sala(msg.sala);
            if (idx == -1)
            {
                idx = crear_sala_con_proj(msg.sala);
                if (idx == -1)
                {
                    fprintf(stderr, "No se pudo crear la sala %s\n", msg.sala);
                    continue;
                }
                printf("Nueva sala creada: %s (cola_id=%d)\n", salas[idx].nombre, salas[idx].cola_id);

                // Lanzar hilo de escucha para esta sala
                pthread_t th;
                struct hilo_args *ha = (struct hilo_args *)malloc(sizeof(*ha));
                ha->indice_sala = idx;
                if (pthread_create(&th, NULL, hilo_sala, ha) != 0)
                {
                    perror("pthread_create hilo_sala");
                }
                else
                {
                    pthread_detach(th);
                }
            }

            // Agregar usuario
            if (agregar_usuario_a_sala(idx, msg.remitente) == 0)
            {
                // Confirmación por cola_global (mtype 2 como en base)
                struct mensaje resp;
                resp.mtype = 2;
                snprintf(resp.remitente, sizeof(resp.remitente), "%s", "server");
                snprintf(resp.sala, sizeof(resp.sala), "%s", salas[idx].nombre);
                snprintf(resp.texto, sizeof(resp.texto), "Te has unido a la sala: %s", salas[idx].nombre);
                if (msgsnd(cola_global, &resp, sizeof(struct mensaje) - sizeof(long), 0) == -1)
                {
                    perror("Error al enviar confirmación");
                }
            }
            else
            {
                fprintf(stderr, "No se pudo agregar usuario '%s' a sala '%s'\n", msg.remitente, msg.sala);
            }
        }
        else
        {
            // otros tipos en global (no usados en base); ignorar
        }
    }

    return 0;
}
