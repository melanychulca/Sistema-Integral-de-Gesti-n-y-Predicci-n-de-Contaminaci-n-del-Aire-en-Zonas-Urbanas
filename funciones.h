#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ZONAS 20
#define MAX_NOMBRE 50
#define MAX_HISTORICO 30
#define MAX_REGISTROS (MAX_ZONAS * MAX_HISTORICO)
#define NOMBRE_ARCHIVO "zonas.dat"
#define NOMBRE_ARCHIVO_REGISTROS "registros.dat"
#define ALTITUD_QUITO 2850.0f
#define FACTOR_PICO_PLACA 0.85f

typedef struct {
    int dia;
    int mes;
    int anio;
} Fecha;

typedef struct {
    int id_zona;
    float co2;
    float so2;
    float no2;
    float pm25;
    float velocidadViento;
    float humedad;
    float temperaturaActual;
    Fecha fecha;
} Registro;

typedef struct {
    int id;
    char nombre[MAX_NOMBRE];
    int activo;
} Zona;

typedef struct {
    Zona zonas[MAX_ZONAS];
    Registro registros[MAX_REGISTROS];
    int cantidadZonas;
    int cantidadRegistros;
} SistemaZonas;

void inicializarSistema(SistemaZonas *sistema);

int cargarDatosBinario(SistemaZonas *sistema, const char *nombreArchivoZonas);
int guardarDatosBinario(const SistemaZonas *sistema, const char *nombreArchivoZonas);

int cargarZonasBinario(SistemaZonas *sistema, const char *nombreArchivo);
int guardarZonasBinario(const SistemaZonas *sistema, const char *nombreArchivo);
int cargarRegistrosBinario(SistemaZonas *sistema, const char *nombreArchivo);
int guardarRegistroBinario(const Registro *registro, const char *nombreArchivo);

int buscarZonaPorId(const SistemaZonas *sistema, int id);
int buscarZonaPorNombre(const SistemaZonas *sistema, const char *nombre);
int agregarZona(SistemaZonas *sistema, const Zona *zona);
int eliminarZona(SistemaZonas *sistema, int id);

void mostrarZona(const SistemaZonas *sistema, const Zona *zona);
void mostrarEstadoActual(const SistemaZonas *sistema);
void generarPrediccion24h(const SistemaZonas *sistema, const Fecha *fechaActual);
void mostrarPromediosHistoricos(const SistemaZonas *sistema);
void buscarZonasCoincidentes(const SistemaZonas *sistema, const char *texto);
void ingresarActualizarZona(SistemaZonas *sistema);

int validarCadena(const char *cadena);
int validarValorFlotante(float valor, float min, float max);
int leerEntero(const char *mensaje, int min, int max);
float leerFlotante(const char *mensaje, float min, float max);

#endif // FUNCIONES_H
