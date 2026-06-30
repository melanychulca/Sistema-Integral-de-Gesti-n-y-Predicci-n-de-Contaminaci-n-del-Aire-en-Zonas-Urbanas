#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ZONAS 50
#define MAX_NOMBRE 50
#define MAX_HISTORICO 30
#define NOMBRE_ARCHIVO "zonas.dat"

typedef struct {
    int dia;
    int mes;
    int anio;
} Fecha;

typedef struct {
    int id;
    char nombre[MAX_NOMBRE];
    int activo;
    float co2;
    float so2;
    float no2;
    float pm25;
    float velocidadViento;
    float humedad;
    float temperaturaActual;
    Fecha fecha;
    float historicoContaminacion[MAX_HISTORICO];
    int diasHistorico;
} Zona;

typedef struct {
    Zona zonas[MAX_ZONAS];
    int cantidad;
} SistemaZonas;


void inicializarSistema(SistemaZonas *sistema);


int cargarDatosBinario(SistemaZonas *sistema, const char *nombreArchivo);


int guardarDatosBinario(const SistemaZonas *sistema, const char *nombreArchivo);


int buscarZonaPorId(const SistemaZonas *sistema, int id);


int buscarZonaPorNombre(const SistemaZonas *sistema, const char *nombre);


int agregarOActualizarZona(SistemaZonas *sistema, const Zona *zona);


int eliminarZona(SistemaZonas *sistema, int id);


void mostrarZona(const Zona *zona);


void mostrarEstadoActual(const SistemaZonas *sistema);


void generarPrediccion24h(const SistemaZonas *sistema);


float calcularPromedioHistorico(const Zona *zona);


float calcularPromedioPonderado(const Zona *zona);

// Obtiene un índice general de contaminación promedio a partir de los contaminantes.
float obtenerIndiceContaminacion(const Zona *zona);

// Muestra los promedios históricos de todas las zonas.
void mostrarPromediosHistoricos(const SistemaZonas *sistema);

// Busca y muestra zonas que contengan un texto en el nombre.
void buscarZonasCoincidentes(const SistemaZonas *sistema, const char *texto);

// Actualiza el registro histórico de contaminación de una zona.
void actualizarRegistroHistorico(Zona *zona, float nuevoValor);

// Registro y actualización de zonas desde la entrada estándar.
void ingresarActualizarZona(SistemaZonas *sistema);

// Validación y lectura de datos desde la entrada estándar.
int validarCadena(const char *cadena);
int leerEntero(const char *mensaje, int min, int max);
float leerFlotante(const char *mensaje, float min, float max);

#endif // FUNCIONES_H
