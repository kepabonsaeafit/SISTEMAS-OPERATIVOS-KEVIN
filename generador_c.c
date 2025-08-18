#include "generador_c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

const char *nombresFemeninos[] = {
    "María", "Luisa", "Carmen", "Ana", "Sofía", "Isabel", "Laura", "Andrea", "Paula", "Valentina",
    "Camila", "Daniela", "Carolina", "Fernanda", "Gabriela", "Patricia", "Claudia", "Diana", "Lucía", "Ximena"};
int num_femeninos = sizeof(nombresFemeninos) / sizeof(char *);

const char *nombresMasculinos[] = {
    "Juan", "Carlos", "José", "Andrés", "Miguel", "Luis", "Pedro", "Alejandro", "Ricardo",
    "Felipe", "David", "Jorge", "Santiago", "Daniel", "Fernando", "Diego", "Rafael", "Martín", "Óscar"};
int num_masculinos = sizeof(nombresMasculinos) / sizeof(char *);

const char *apellidos[] = {
    "Gómez", "Rodríguez", "Martínez", "López", "García", "Pérez", "González", "Sánchez", "Ramírez", "Torres",
    "Díaz", "Vargas", "Castro", "Ruiz", "Álvarez", "Romero", "Suárez", "Rojas", "Moreno", "Muñoz", "Valencia"};
int num_apellidos = sizeof(apellidos) / sizeof(char *);

const char *ciudadesColombia[] = {
    "Bogotá", "Medellín", "Cali", "Barranquilla", "Cartagena", "Bucaramanga", "Pereira", "Santa Marta", "Cúcuta", "Ibagué",
    "Manizales", "Pasto", "Neiva", "Villavicencio", "Armenia", "Sincelejo", "Valledupar", "Montería", "Popayán", "Tunja"};
int num_ciudades = sizeof(ciudadesColombia) / sizeof(char *);

long get_peak_memory()
{
    struct rusage rus;
    getrusage(RUSAGE_SELF, &rus);
    return rus.ru_maxrss / 1024; // in MB
}

char *generarFechaNacimientoC()
{
    int dia = 1 + rand() % 28;
    int mes = 1 + rand() % 12;
    int anio = 1960 + rand() % 50;
    char *f = (char *)malloc(16);
    snprintf(f, 16, "%02d/%02d/%04d", dia, mes, anio);
    return f;
}

char *generarIDC()
{
    static long contador = 1000000000;
    char *id = (char *)malloc(20);
    sprintf(id, "%ld", contador++);
    return id;
}

double randomDoubleC(double min, double max)
{
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

void generarPersonaC(PersonaC *p)
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

    char *id = generarIDC();
    strncpy(p->id, id, 19);
    p->id[19] = '\0';
    free(id);

    const char *ciudad = ciudadesColombia[rand() % num_ciudades];
    strncpy(p->ciudadNacimiento, ciudad, 49);
    p->ciudadNacimiento[49] = '\0';

    char *fecha = generarFechaNacimientoC();
    strncpy(p->fechaNacimiento, fecha, 10);
    p->fechaNacimiento[10] = '\0';
    free(fecha);

    p->ingresosAnuales = randomDoubleC(10000000, 500000000);
    p->patrimonio = randomDoubleC(0, 2000000000);
    p->deudas = randomDoubleC(0, p->patrimonio * 0.7);
    p->declaranteRenta = (p->ingresosAnuales > 50000000) && (rand() % 100 > 30);
}

PersonaC *generarColeccionC(int n)
{
    PersonaC *personas = (PersonaC *)malloc(n * sizeof(PersonaC));
    for (int i = 0; i < n; i++)
    {
        generarPersonaC(&personas[i]);
    }
    return personas;
}

void freeColeccionC(PersonaC *personas)
{
    free(personas);
}

void printPersonSummaryC(const PersonaC *p, int age)
{
    if (p)
    {
        double net = p->patrimonio - p->deudas;
        if (age == -1)
        {
            printf("%s %s (ID: %s, Ciudad: %s, Patrimonio neto: $%.2f)\n",
                   p->nombre, p->apellido, p->id, p->ciudadNacimiento, net);
        }
        else
        {
            printf("%s %s (ID: %s, Ciudad: %s, Edad: %d, Patrimonio neto: $%.2f)\n",
                   p->nombre, p->apellido, p->id, p->ciudadNacimiento, age, net);
        }
    }
}

int getAgeC(const PersonaC *p)
{
    int day, month, year;
    sscanf(p->fechaNacimiento, "%d/%d/%d", &day, &month, &year);
    int curr_day = 17, curr_month = 8, curr_year = 2025;
    int age = curr_year - year;
    if (month > curr_month || (month == curr_month && day > curr_day))
        age--;
    return age;
}

char getGroupC(const PersonaC *p)
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

void process_data_c(PersonaC *personas, int n, int use_pointers)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    long start_mem = get_peak_memory();

    int max_age_country = -1;
    PersonaC *oldest_country = NULL;
    double max_net_country = -1.0;
    PersonaC *richest_country = NULL;

    char unique_cities[20][50];
    int num_unique_cities = 0;
    int max_age_city[20] = {0};
    PersonaC *oldest_city[20] = {NULL};
    double max_net_city[20];
    PersonaC *richest_city[20] = {NULL};
    double max_net_group[3] = {0};
    PersonaC *richest_group[3] = {NULL};
    int count_declarants[3] = {0};
    double city_net_sum[20] = {0};
    int city_count[20] = {0};
    int group_old[3] = {0};
    int group_total[3] = {0};
    double avg_income_sum[3] = {0};

    typedef struct
    {
        PersonaC *arr;
        int size;
        int capacity;
    } DynArray;
    DynArray city_values[20] = {{NULL, 0, 0}};
    DynArray group_values[3] = {{NULL, 0, 0}};

    for (int i = 0; i < 20; i++)
    {
        max_age_city[i] = -1;
        max_net_city[i] = -1.0;
        if (!use_pointers)
        {
            city_values[i].capacity = n / 20 + 1;
            city_values[i].arr = (PersonaC *)malloc(city_values[i].capacity * sizeof(PersonaC));
            city_values[i].size = 0;
        }
    }
    for (int i = 0; i < 3; i++)
    {
        max_net_group[i] = -1.0;
        if (!use_pointers)
        {
            group_values[i].capacity = n / 3 + 1;
            group_values[i].arr = (PersonaC *)malloc(group_values[i].capacity * sizeof(PersonaC));
            group_values[i].size = 0;
        }
    }

    PersonaC **city_ptrs[20] = {NULL};
    PersonaC **group_ptrs[3] = {NULL};
    int city_ptrs_size[20] = {0};
    int group_ptrs_size[3] = {0};
    int city_ptrs_capacity[20] = {0};
    int group_ptrs_capacity[3] = {0};

    if (use_pointers)
    {
        for (int i = 0; i < 20; i++)
        {
            city_ptrs_capacity[i] = n / 20 + 1;
            city_ptrs[i] = (PersonaC **)malloc(city_ptrs_capacity[i] * sizeof(PersonaC *));
            city_ptrs_size[i] = 0;
        }
        for (int i = 0; i < 3; i++)
        {
            group_ptrs_capacity[i] = n / 3 + 1;
            group_ptrs[i] = (PersonaC **)malloc(group_ptrs_capacity[i] * sizeof(PersonaC *));
            group_ptrs_size[i] = 0;
        }

        for (int i = 0; i < n; i++)
        {
            PersonaC *p = &personas[i];
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
            if (city_ptrs_size[city_idx] < city_ptrs_capacity[city_idx])
            {
                city_ptrs[city_idx][city_ptrs_size[city_idx]++] = p;
            }

            char g = getGroupC(p);
            int g_idx = g - 'A';
            if (group_ptrs_size[g_idx] < group_ptrs_capacity[g_idx])
            {
                group_ptrs[g_idx][group_ptrs_size[g_idx]++] = p;
            }
        }
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            PersonaC *p = &personas[i];
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
            if (city_values[city_idx].size < city_values[city_idx].capacity)
            {
                city_values[city_idx].arr[city_values[city_idx].size++] = *p;
            }

            char g = getGroupC(p);
            int g_idx = g - 'A';
            if (group_values[g_idx].size < group_values[g_idx].capacity)
            {
                group_values[g_idx].arr[group_values[g_idx].size++] = *p;
            }
        }
    }

    for (int i = 0; i < n; i++)
    {
        PersonaC *p = &personas[i];
        int age = getAgeC(p);
        double net = p->patrimonio - p->deudas;

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

        char g = getGroupC(p);
        int g_idx = g - 'A';
        if (net > max_net_group[g_idx])
        {
            max_net_group[g_idx] = net;
            richest_group[g_idx] = p;
        }

        if (p->declaranteRenta)
        {
            count_declarants[g_idx]++;
        }

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

    printf("\n=== Resultados C %s ===\n", use_pointers ? "Apuntadores" : "Valores");
    printf("1. Persona más longeva en el país:\n");
    printPersonSummaryC(oldest_country, max_age_country);
    printf("Persona más longeva por ciudad:\n");
    for (int i = 0; i < num_unique_cities; i++)
    {
        printf("%s: ", unique_cities[i]);
        printPersonSummaryC(oldest_city[i], max_age_city[i]);
    }
    printf("2. Persona con mayor patrimonio neto en el país:\n");
    printPersonSummaryC(richest_country, -1);
    printf("Persona con mayor patrimonio neto por ciudad:\n");
    for (int i = 0; i < num_unique_cities; i++)
    {
        printf("%s: ", unique_cities[i]);
        printPersonSummaryC(richest_city[i], -1);
    }
    printf("Persona con mayor patrimonio neto por grupo:\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%c: ", 'A' + i);
        printPersonSummaryC(richest_group[i], -1);
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

    if (use_pointers)
    {
        for (int i = 0; i < 20; i++)
        {
            if (city_ptrs[i])
                free(city_ptrs[i]);
        }
        for (int i = 0; i < 3; i++)
        {
            if (group_ptrs[i])
                free(group_ptrs[i]);
        }
    }
    else
    {
        for (int i = 0; i < 20; i++)
        {
            if (city_values[i].arr)
                free(city_values[i].arr);
        }
        for (int i = 0; i < 3; i++)
        {
            if (group_values[i].arr)
                free(group_values[i].arr);
        }
    }
}