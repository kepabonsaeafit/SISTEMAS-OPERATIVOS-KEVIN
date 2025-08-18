#include "generador.h"
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sys/resource.h>
#include <map>
#include <limits>

using namespace std;
using namespace std::chrono;

// --- Bases de datos para generación realista ---

const std::vector<std::string> nombresFemeninos = {
    "María", "Luisa", "Carmen", "Ana", "Sofía", "Isabel", "Laura", "Andrea", "Paula", "Valentina",
    "Camila", "Daniela", "Carolina", "Fernanda", "Gabriela", "Patricia", "Claudia", "Diana", "Lucía", "Ximena"};

const std::vector<std::string> nombresMasculinos = {
    "Juan", "Carlos", "José", "Andrés", "Miguel", "Luis", "Pedro", "Alejandro", "Ricardo",
    "Felipe", "David", "Jorge", "Santiago", "Daniel", "Fernando", "Diego", "Rafael", "Martín", "Óscar"};

const std::vector<std::string> apellidos = {
    "Gómez", "Rodríguez", "Martínez", "López", "García", "Pérez", "González", "Sánchez", "Ramírez", "Torres",
    "Díaz", "Vargas", "Castro", "Ruiz", "Álvarez", "Romero", "Suárez", "Rojas", "Moreno", "Muñoz", "Valencia"};

const std::vector<std::string> ciudadesColombia = {
    "Bogotá", "Medellín", "Cali", "Barranquilla", "Cartagena", "Bucaramanga", "Pereira", "Santa Marta", "Cúcuta", "Ibagué",
    "Manizales", "Pasto", "Neiva", "Villavicencio", "Armenia", "Sincelejo", "Valledupar", "Montería", "Popayán", "Tunja"};

std::string generarFechaNacimiento()
{
    int dia = 1 + rand() % 28;
    int mes = 1 + rand() % 12;
    int anio = 1960 + rand() % 50;
    return std::to_string(dia) + "/" + std::to_string(mes) + "/" + std::to_string(anio);
}

std::string generarID()
{
    static long contador = 1000000000;
    return std::to_string(contador++);
}

double randomDouble(double min, double max)
{
    static std::mt19937 generator(time(nullptr));
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

Persona generarPersona()
{
    Persona p;
    bool esHombre = rand() % 2;
    p.nombre = esHombre ? nombresMasculinos[rand() % nombresMasculinos.size()] : nombresFemeninos[rand() % nombresFemeninos.size()];
    p.apellido = apellidos[rand() % apellidos.size()] + " " + apellidos[rand() % apellidos.size()];
    p.id = generarID();
    p.ciudadNacimiento = ciudadesColombia[rand() % ciudadesColombia.size()];
    p.fechaNacimiento = generarFechaNacimiento();
    p.ingresosAnuales = randomDouble(10000000, 500000000);
    p.patrimonio = randomDouble(0, 2000000000);
    p.deudas = randomDouble(0, p.patrimonio * 0.7);
    p.declaranteRenta = (p.ingresosAnuales > 50000000) && (rand() % 100 > 30);
    return p;
}

std::vector<Persona> generarColeccion(int n)
{
    std::vector<Persona> personas;
    personas.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        personas.push_back(generarPersona());
    }
    return personas;
}

const Persona *buscarPorID(const std::vector<Persona> &personas, const std::string &id)
{
    for (const auto &persona : personas)
    {
        if (persona.id == id)
        {
            return &persona;
        }
    }
    return nullptr;
}

long get_peak_memory()
{
    rusage rus;
    getrusage(RUSAGE_SELF, &rus);
    return rus.ru_maxrss / 1024; // in MB
}

int getAge(const Persona &p)
{
    int day, month, year;
    sscanf(p.fechaNacimiento.c_str(), "%d/%d/%d", &day, &month, &year);
    int curr_day = 17, curr_month = 8, curr_year = 2025;
    int age = curr_year - year;
    if (month > curr_month || (month == curr_month && day > curr_day))
        age--;
    return age;
}

char getGroup(const Persona &p)
{
    string last2 = p.id.substr(p.id.size() - 2);
    int dig = stoi(last2);
    if (dig <= 39)
        return 'A';
    else if (dig <= 79)
        return 'B';
    else
        return 'C';
}

void printPersonSummary(const Persona *p, int age = -1)
{
    if (p)
    {
        double net = p->patrimonio - p->deudas;
        cout << p->nombre << " " << p->apellido << " (ID: " << p->id << ", Ciudad: " << p->ciudadNacimiento
             << ", Edad: " << (age >= 0 ? age : getAge(*p)) << ", Patrimonio neto: $" << fixed << setprecision(2) << net << ")\n";
    }
}

void process_data(const vector<Persona> &personas, bool use_pointers)
{
    auto start_time = high_resolution_clock::now();
    long start_mem = get_peak_memory();

    map<string, vector<const Persona *>> city_ptrs;
    map<char, vector<const Persona *>> group_ptrs;
    map<string, vector<Persona>> city_values;
    map<char, vector<Persona>> group_values;

    int max_age_country = -1;
    const Persona *oldest_country = nullptr;
    double max_net_country = -numeric_limits<double>::max();
    const Persona *richest_country = nullptr;

    map<string, int> max_age_city;
    map<string, const Persona *> oldest_city;
    map<string, double> max_net_city;
    map<string, const Persona *> richest_city;

    map<char, int> count_declarants;
    map<char, double> max_net_group;
    map<char, const Persona *> richest_group;
    map<string, pair<double, int>> city_net_sum_count;
    map<char, pair<int, int>> group_old_count_total;
    map<char, double> avg_income_group;

    // Primera pasada: llenar estructuras
    if (use_pointers)
    {
        for (const auto &p : personas)
        {
            city_ptrs[p.ciudadNacimiento].push_back(&p);
            char g = getGroup(p);
            group_ptrs[g].push_back(&p);
        }
    }
    else
    {
        for (const auto &p : personas)
        {
            city_values[p.ciudadNacimiento].push_back(p);
            char g = getGroup(p);
            group_values[g].push_back(p);
        }
    }

    // Segunda pasada: procesar métricas
    for (const auto &p : personas)
    {
        int age = getAge(p);
        double net = p.patrimonio - p.deudas;

        if (age > max_age_country)
        {
            max_age_country = age;
            oldest_country = &p;
        }
        if (net > max_net_country)
        {
            max_net_country = net;
            richest_country = &p;
        }

        if (age > max_age_city[p.ciudadNacimiento])
        {
            max_age_city[p.ciudadNacimiento] = age;
            oldest_city[p.ciudadNacimiento] = &p;
        }
        if (net > max_net_city[p.ciudadNacimiento])
        {
            max_net_city[p.ciudadNacimiento] = net;
            richest_city[p.ciudadNacimiento] = &p;
        }

        char g = getGroup(p);
        if (net > max_net_group[g])
        {
            max_net_group[g] = net;
            richest_group[g] = &p;
        }

        if (p.declaranteRenta)
        {
            count_declarants[g]++;
        }

        city_net_sum_count[p.ciudadNacimiento].first += net;
        city_net_sum_count[p.ciudadNacimiento].second++;
        group_old_count_total[g].second++;
        if (age > 80)
            group_old_count_total[g].first++;
        avg_income_group[g] += p.ingresosAnuales;
    }

    for (auto &ag : avg_income_group)
    {
        ag.second /= group_old_count_total[ag.first].second;
    }

    auto end_time = high_resolution_clock::now();
    long end_mem = get_peak_memory();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    cout << "\n=== Resultados C++ " << (use_pointers ? "Apuntadores" : "Valores") << " ===\n";
    cout << "1. Persona más longeva en el país:\n";
    printPersonSummary(oldest_country, max_age_country);
    cout << "Persona más longeva por ciudad:\n";
    for (const auto &c : oldest_city)
    {
        cout << c.first << ": ";
        printPersonSummary(c.second, max_age_city[c.first]);
    }
    cout << "2. Persona con mayor patrimonio neto en el país:\n";
    printPersonSummary(richest_country);
    cout << "Persona con mayor patrimonio neto por ciudad:\n";
    for (const auto &c : richest_city)
    {
        cout << c.first << ": ";
        printPersonSummary(c.second);
    }
    cout << "Persona con mayor patrimonio neto por grupo:\n";
    for (const auto &g : richest_group)
    {
        cout << g.first << ": ";
        printPersonSummary(g.second);
    }
    cout << "3. Declarantes de renta por grupo:\n";
    for (const auto &g : count_declarants)
    {
        cout << g.first << ": " << g.second << " personas\n";
    }
    cout << "Preguntas adicionales:\n";
    cout << "a. Ciudades con patrimonio neto promedio más alto (top 3):\n";
    vector<pair<double, string>> city_avgs;
    for (const auto &c : city_net_sum_count)
    {
        double avg = c.second.first / c.second.second;
        city_avgs.push_back({-avg, c.first});
    }
    sort(city_avgs.begin(), city_avgs.end());
    for (int i = 0; i < min(3, (int)city_avgs.size()); i++)
    {
        cout << city_avgs[i].second << ": $" << fixed << setprecision(2) << -city_avgs[i].first << "\n";
    }
    cout << "b. Porcentaje de personas >80 años por grupo:\n";
    for (const auto &g : group_old_count_total)
    {
        double perc = 100.0 * g.second.first / g.second.second;
        cout << g.first << ": " << fixed << setprecision(2) << perc << "%\n";
    }
    cout << "c. Ingreso promedio por grupo:\n";
    for (const auto &g : avg_income_group)
    {
        cout << g.first << ": $" << fixed << setprecision(2) << g.second << "\n";
    }
    cout << "\nMétricas:\n";
    cout << "Tiempo de ejecución: " << duration.count() << " ms\n";
    cout << "Uso de memoria pico: " << end_mem << " MB (incremento: " << (end_mem - start_mem) << " MB)\n";
}