#ifndef GENERADOR_C_H
#define GENERADOR_C_H

#ifdef __cplusplus
extern "C"
{
#endif

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
    } PersonaC;

    long get_peak_memory(void);
    char *generarFechaNacimientoC();
    char *generarIDC();
    double randomDoubleC(double min, double max);
    void generarPersonaC(PersonaC *p);
    PersonaC *generarColeccionC(int n);
    void freeColeccionC(PersonaC *personas);
    void process_data_c(PersonaC *personas, int n, int use_pointers);

#ifdef __cplusplus
}
#endif

#endif // GENERADOR_C_H