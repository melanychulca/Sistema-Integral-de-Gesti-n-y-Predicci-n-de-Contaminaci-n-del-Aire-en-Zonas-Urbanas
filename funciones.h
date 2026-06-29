#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ZONAS 5
#define MAX_NOMBRE 50
#define MAX_HISTORICO 30
#define NOMBRE_ARCHIVO "zonas.dat"

typedef struct {
    int id;
    char nombre[MAX_NOMBRE];
    int activo;
    float contaminacionActual;
    float temperaturaActual;
    float humedadActual;
    float historicoContaminacion[MAX_HISTORICO];
    int diasHistorico;
} Zona;

typedef struct {
    Zona zonas[MAX_ZONAS];
    int cantidad;
} SistemaZonas;

// Inicializa un sistema con zonas vacías.
void inicializarSistema(SistemaZonas *sistema);

// Carga desde un archivo binario y devuelve 1 si tuvo éxito.
int cargarDatosBinario(SistemaZonas *sistema, const char *nombreArchivo);

// Guarda en un archivo binario y devuelve 1 si tuvo éxito.
int guardarDatosBinario(const SistemaZonas *sistema, const char *nombreArchivo);

// Busca zona por ID y devuelve el índice, o -1 si no existe.
int buscarZonaPorId(const SistemaZonas *sistema, int id);

// Busca zona por nombre exacto y devuelve el índice, o -1 si no existe.
int buscarZonaPorNombre(const SistemaZonas *sistema, const char *nombre);

// Agrega o actualiza una zona. Devuelve 1 si se agregó/actualizó, 0 si no hay espacio.
int agregarOActualizarZona(SistemaZonas *sistema, const Zona *zona);

// Elimina una zona por ID. Devuelve 1 si se eliminó correctamente.
int eliminarZona(SistemaZonas *sistema, int id);

// Muestra la zona formateada.
void mostrarZona(const Zona *zona);

// Muestra el estado actual de todas las zonas con alertas.
void mostrarEstadoActual(const SistemaZonas *sistema);

// Genera una predicción de contaminación a 24 horas con promedio ponderado.
void generarPrediccion24h(const SistemaZonas *sistema);

// Calcula el promedio histórico de una zona.
float calcularPromedioHistorico(const Zona *zona);

// Calcula el promedio ponderado usando los días históricos más recientes.
float calcularPromedioPonderado(const Zona *zona);

// Muestra los promedios históricos de todas las zonas.
void mostrarPromediosHistoricos(const SistemaZonas *sistema);

// Busca y muestra zonas que contengan un texto en el nombre.
void buscarZonasCoincidentes(const SistemaZonas *sistema, const char *texto);

// Actualiza el registro histórico de contaminación de una zona.
void actualizarRegistroHistorico(Zona *zona, float nuevoValor);

// Validación y lectura de datos desde la entrada estándar.
int validarCadena(const char *cadena);
int leerEntero(const char *mensaje, int min, int max);
float leerFlotante(const char *mensaje, float min, float max);

#endif // FUNCIONES_H
