#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <limits.h>

typedef struct
{
    char nombre[50];
    char apellido[100];
    char id[20];
    char ciudadNacimiento[50];
    char fechaNacimiento[11];
    double ingresosAnuales;
    double patrimonio;
    double deudas;
    int declaranteRenta;
} Persona;

// Bases de datos
const char *nombresFemeninos[] = {
    "María", "Luisa", "Carmen", "Ana", "Sofía", "Isabel", "Laura", "Andrea", "Paula", "Valentina",
    "Camila", "Daniela", "Carolina", "Fernanda", "Gabriela", "Patricia", "Claudia", "Diana", "Lucía", "Ximena"};
int num_femeninos = sizeof(nombresFemeninos) / sizeof(char *);

const char *nombresMasculinos[] = {
    "Juan", "Carlos", "José", "James", "Andrés", "Miguel", "Luis", "Pedro", "Alejandro", "Ricardo",
    "Felipe", "David", "Jorge", "Santiago", "Daniel", "Fernando", "Diego", "Rafael", "Martín", "Óscar",
    "Edison", "Sofia", "Camila", "Juana", "Ana", "Laura", "Karla", "Andrea", "Daniela", "Alejandra", "Martina",
    "Nelly", "María", "Nestor", "Trinidad", "Fernanda", "Carolina", "Lina", "Gertridis"};
int num_masculinos = sizeof(nombresMasculinos) / sizeof(char *);

const char *apellidos[] = {
    "Gómez", "Rodríguez", "Martínez", "López", "García", "Pérez", "González", "Sánchez", "Ramírez", "Torres",
    "Díaz", "Vargas", "Castro", "Ruiz", "Álvarez", "Romero", "Suárez", "Rojas", "Moreno", "Muñoz", "Valencia"};
int num_apellidos = sizeof(apellidos) / sizeof(char *);

const char *ciudadesColombia[] = {
    "Bogotá", "Medellín", "Cali", "Barranquilla", "Cartagena", "Bucaramanga", "Pereira", "Santa Marta", "Cúcuta", "Ibagué",
    "Manizales", "Pasto", "Neiva", "Villavicencio", "Armenia", "Sincelejo", "Valledupar", "Montería", "Popayán", "Tunja"};
int num_ciudades = sizeof(ciudadesColombia) / sizeof(char *);

// Funciones generadoras
char *generarFechaNacimiento()
{
    int dia = 1 + rand() % 28;
    int mes = 1 + rand() % 12;
    int anio = 1960 + rand() % 50;
    char *f = (char *)malloc(16);                      // Aumentar a 16 bytes para mayor seguridad
    snprintf(f, 16, "%02d/%02d/%04d", dia, mes, anio); // Usar snprintf con límite mayor
    return f;
}

char *generarID()
{
    static long contador = 1000000000;
    char *id = (char *)malloc(20);
    sprintf(id, "%ld", contador++);
    return id;
}

double randomDouble(double min, double max)
{
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

void generarPersona(Persona *p)
{
    int esHombre = rand() % 2;
    const char *nombre = esHombre ? nombresMasculinos[rand() % num_masculinos] : nombresFemeninos[rand() % num_femeninos];
    strncpy(p->nombre, nombre, 49);
    p->nombre[49] = '\0';

    const char *app1 = apellidos[rand() % num_apellidos];
    strncpy(p->apellido, app1, 99);
    p->apellido[99] = '\0';
    strcat(p->apellido, " ");
    const char *app2 = apellidos[rand() % num_apellidos];
    strcat(p->apellido, app2);

    char *id = generarID();
    strncpy(p->id, id, 19);
    p->id[19] = '\0';
    free(id);

    const char *ciudad = ciudadesColombia[rand() % num_ciudades];
    strncpy(p->ciudadNacimiento, ciudad, 49);
    p->ciudadNacimiento[49] = '\0';

    char *fecha = generarFechaNacimiento();
    strncpy(p->fechaNacimiento, fecha, 10);
    p->fechaNacimiento[10] = '\0';
    free(fecha);

    p->ingresosAnuales = randomDouble(10000000, 500000000);
    p->patrimonio = randomDouble(0, 2000000000);
    p->deudas = randomDouble(0, p->patrimonio * 0.7);
    p->declaranteRenta = (p->ingresosAnuales > 50000000) && (rand() % 100 > 30);
}

Persona *generarColeccion(int n)
{
    Persona *personas = (Persona *)malloc(n * sizeof(Persona));
    for (int i = 0; i < n; i++)
    {
        generarPersona(&personas[i]);
    }
    return personas;
}

int getAge(const Persona *p)
{
    int day, month, year;
    sscanf(p->fechaNacimiento, "%d/%d/%d", &day, &month, &year);
    int curr_day = 17, curr_month = 8, curr_year = 2025;
    int age = curr_year - year;
    if (month > curr_month || (month == curr_month && day > curr_day))
        age--;
    return age;
}

char getGroup(const Persona *p)
{
    char last2[3];
    strncpy(last2, p->id + strlen(p->id) - 2, 2);
    last2[2] = '\0';
    int dig = atoi(last2);
    if (dig <= 39)
        return 'A';
    else if (dig <= 79)
        return 'B';
    else
        return 'C';
}

long get_peak_memory()
{
    struct rusage rus;
    getrusage(RUSAGE_SELF, &rus);
    return rus.ru_maxrss / 1024; // MB
}

void printPersonSummary(const Persona *p, int age)
{
    if (p)
    {
        double net = p->patrimonio - p->deudas;
        printf("%s %s (ID: %s, Ciudad: %s, Edad: %d, Patrimonio neto: $%.2f)\n",
               p->nombre, p->apellido, p->id, p->ciudadNacimiento,
               (age >= 0 ? age : getAge(p)), net);
    }
}

void process_data(Persona *personas, int n, int use_pointers)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    long start_mem = get_peak_memory();

    int max_age_country = -1;
    Persona *oldest_country = NULL;
    double max_net_country = -1.0;
    Persona *richest_country = NULL;

    char unique_cities[20][50];
    int num_unique_cities = 0;
    int max_age_city[20] = {0};
    Persona *oldest_city[20] = {NULL};
    double max_net_city[20];
    Persona *richest_city[20] = {NULL};
    double max_net_group[3] = {0};
    Persona *richest_group[3] = {NULL};
    int count_declarants[3] = {0};
    double city_net_sum[20] = {0};
    int city_count[20] = {0};
    int group_old[3] = {0};
    int group_total[3] = {0};
    double avg_income_sum[3] = {0};

    for (int i = 0; i < 20; i++)
    {
        max_age_city[i] = -1;
        max_net_city[i] = -1.0;
    }
    for (int i = 0; i < 3; i++)
    {
        max_net_group[i] = -1.0;
    }

    for (int i = 0; i < n; i++)
    {
        Persona *p = &personas[i];
        int age = getAge(p);
        double net = p->patrimonio - p->deudas;

        // Country
        if (age > max_age_country)
        {
            max_age_country = age;
            oldest_country = p;
        }
        if (net > max_net_country)
        {
            max_net_country = net;
            richest_country = p;
        }

        // City index
        int city_idx = -1;
        for (int j = 0; j < num_unique_cities; j++)
        {
            if (strcmp(unique_cities[j], p->ciudadNacimiento) == 0)
            {
                city_idx = j;
                break;
            }
        }
        if (city_idx == -1)
        {
            strcpy(unique_cities[num_unique_cities], p->ciudadNacimiento);
            city_idx = num_unique_cities++;
        }

        // Per city
        if (age > max_age_city[city_idx])
        {
            max_age_city[city_idx] = age;
            oldest_city[city_idx] = p;
        }
        if (net > max_net_city[city_idx])
        {
            max_net_city[city_idx] = net;
            richest_city[city_idx] = p;
        }

        // Per group
        char g = getGroup(p);
        int g_idx = g - 'A';
        if (net > max_net_group[g_idx])
        {
            max_net_group[g_idx] = net;
            richest_group[g_idx] = p;
        }

        // Declarants
        if (p->declaranteRenta)
        {
            count_declarants[g_idx]++;
        }

        // Additional
        city_net_sum[city_idx] += net;
        city_count[city_idx]++;
        group_total[g_idx]++;
        if (age > 80)
            group_old[g_idx]++;
        avg_income_sum[g_idx] += p->ingresosAnuales;
    }

    for (int i = 0; i < 3; i++)
    {
        if (group_total[i] > 0)
        {
            avg_income_sum[i] /= group_total[i];
        }
    }

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long micros = end.tv_usec - start.tv_usec;
    long duration = seconds * 1000 + micros / 1000;
    long end_mem = get_peak_memory();

    printf("\n=== Resultados ===\n");

    printf("1. Persona más longeva en el país:\n");
    printPersonSummary(oldest_country, max_age_country);

    printf("Persona más longeva por ciudad:\n");
    for (int i = 0; i < num_unique_cities; i++)
    {
        printf("%s: ", unique_cities[i]);
        printPersonSummary(oldest_city[i], max_age_city[i]);
    }

    printf("2. Persona con mayor patrimonio neto en el país:\n");
    printPersonSummary(richest_country, -1);

    printf("Persona con mayor patrimonio neto por ciudad:\n");
    for (int i = 0; i < num_unique_cities; i++)
    {
        printf("%s: ", unique_cities[i]);
        printPersonSummary(richest_city[i], -1);
    }

    printf("Persona con mayor patrimonio neto por grupo:\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%c: ", 'A' + i);
        printPersonSummary(richest_group[i], -1);
    }

    printf("3. Declarantes de renta por grupo:\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%c: %d personas\n", 'A' + i, count_declarants[i]);
    }

    printf("Preguntas adicionales:\n");
    printf("a. Ciudades con patrimonio neto promedio más alto (top 3):\n");
    typedef struct
    {
        double avg;
        int idx;
    } CityAvg;
    CityAvg avgs[20];
    for (int i = 0; i < num_unique_cities; i++)
    {
        avgs[i].avg = city_count[i] > 0 ? city_net_sum[i] / city_count[i] : 0;
        avgs[i].idx = i;
    }
    for (int i = 0; i < num_unique_cities; i++)
    {
        for (int j = i + 1; j < num_unique_cities; j++)
        {
            if (avgs[i].avg < avgs[j].avg)
            {
                CityAvg temp = avgs[i];
                avgs[i] = avgs[j];
                avgs[j] = temp;
            }
        }
    }
    for (int i = 0; i < 3 && i < num_unique_cities; i++)
    {
        printf("%s: $%.2f\n", unique_cities[avgs[i].idx], avgs[i].avg);
    }

    printf("b. Porcentaje de personas >80 años por grupo:\n");
    for (int i = 0; i < 3; i++)
    {
        double perc = group_total[i] > 0 ? 100.0 * group_old[i] / group_total[i] : 0;
        printf("%c: %.2f%%\n", 'A' + i, perc);
    }

    printf("c. Ingreso promedio por grupo:\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%c: $%.2f\n", 'A' + i, avg_income_sum[i]);
    }

    printf("\nMétricas:\n");
    printf("Tiempo de ejecución: %ld ms\n", duration);
    printf("Uso de memoria pico: %ld MB (incremento: %ld MB)\n", end_mem, (end_mem - start_mem));
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int n = 1000000;
    int use_pointers = 1;

    if (argc > 1)
        n = atoi(argv[1]);
    if (argc > 2 && strcmp(argv[2], "value") == 0)
        use_pointers = 0;

    printf("Generando %d personas...\n", n);
    Persona *personas = generarColeccion(n);

    printf("Procesando con %s...\n", use_pointers ? "apuntadores" : "valores");
    process_data(personas, n, use_pointers);

    free(personas);
    return 0;
}