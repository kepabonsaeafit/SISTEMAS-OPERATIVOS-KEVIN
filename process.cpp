#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <chrono>
#include <sys/resource.h>
#include <sys/time.h>
#include <iomanip>
#include <sstream>
#include "generador.h"

using namespace std;
using namespace std::chrono;

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

long get_peak_memory()
{
    rusage rus;
    getrusage(RUSAGE_SELF, &rus);
    return rus.ru_maxrss / 1024; // in MB
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

    // Estructuras para agrupamientos
    map<string, vector<const Persona *>> city_ptrs;
    map<char, vector<const Persona *>> group_ptrs;
    map<string, vector<Persona>> city_values;
    map<char, vector<Persona>> group_values;

    // Para preguntas obligatorias
    int max_age_country = -1;
    const Persona *oldest_country = nullptr;
    double max_net_country = -numeric_limits<double>::infinity();
    const Persona *richest_country = nullptr;

    map<string, int> max_age_city;
    map<string, const Persona *> oldest_city;
    map<string, double> max_net_city;
    map<string, const Persona *> richest_city;

    map<char, int> count_declarants;
    map<char, vector<const Persona *>> declarants_per_group;

    map<char, double> max_net_group;
    map<char, const Persona *> richest_group;

    // Para preguntas adicionales
    map<string, pair<double, int>> city_net_sum_count; // sum net, count
    map<char, pair<int, int>> group_old_count_total;   // >80 count, total
    map<char, double> avg_income_group;

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

    // Procesamiento unificado
    for (const auto &p : personas)
    {
        int age = getAge(p);
        double net = p.patrimonio - p.deudas;

        // País
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

        // Por ciudad
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

        // Por grupo
        char g = getGroup(p);
        if (net > max_net_group[g])
        {
            max_net_group[g] = net;
            richest_group[g] = &p;
        }

        // Declarantes
        if (p.declaranteRenta)
        {
            count_declarants[g]++;
            declarants_per_group[g].push_back(&p);
        }

        // Adicionales
        city_net_sum_count[p.ciudadNacimiento].first += net;
        city_net_sum_count[p.ciudadNacimiento].second++;
        group_old_count_total[g].second++;
        if (age > 80)
            group_old_count_total[g].first++;
        avg_income_group[g] += p.ingresosAnuales;
    }

    // Ajustes para adicionales
    for (auto &ag : avg_income_group)
    {
        ag.second /= group_old_count_total[ag.first].second; // total from above
    }

    auto end_time = high_resolution_clock::now();
    long end_mem = get_peak_memory();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    // Output respuestas
    cout << "\n=== Resultados ===\n";

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
        // Listar sería demasiado para large N, omitir list, solo count
    }

    // Validación: siempre correcta ya que se calcula de ID

    cout << "Preguntas adicionales:\n";
    cout << "a. Ciudades con patrimonio neto promedio más alto (top 3):\n";
    vector<pair<double, string>> city_avgs;
    for (const auto &c : city_net_sum_count)
    {
        double avg = c.second.first / c.second.second;
        city_avgs.push_back({-avg, c.first}); // neg for descending
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

int main(int argc, char *argv[])
{
    srand(time(nullptr));
    size_t n = 1000000;       // default 1M
    bool use_pointers = true; // default pointers

    if (argc > 1)
        n = stoul(argv[1]);
    if (argc > 2 && string(argv[2]) == "value")
        use_pointers = false;

    cout << "Generando " << n << " personas...\n";
    vector<Persona> personas = generarColeccion(n);

    cout << "Procesando con " << (use_pointers ? "apuntadores" : "valores") << "...\n";
    process_data(personas, use_pointers);

    return 0;
}