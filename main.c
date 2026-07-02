#include <stdio.h>
#include <time.h>
#include "funciones.h"

static void mostrarMenu(void) {
    printf("\nGESTION DE ZONAS CLIMATICAS\n");
    printf("1. Cargar datos desde archivo\n");
    printf("2. Ingresar/actualizar datos de zonas\n");
    printf("3. Ver estado actual (con alertas)\n");
    printf("4. Generar prediccion a 24h\n");
    printf("5. Mostrar promedios historicos (ultimos 30 dias)\n");
    printf("6. Buscar una zona especifica por nombre/ID\n");
    printf("7. Eliminar zona\n");
    printf("8. Guardar y salir\n");
}

static Fecha obtenerFechaActual(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    Fecha fecha = {local->tm_mday, local->tm_mon + 1, local->tm_year + 1900};
    return fecha;
}

static void buscarZona(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }

    printf("Buscar por (1) ID o (2) Nombre? ");
    int opcion = leerEntero("Seleccione una opcion (1-2): ", 1, 2);
    if (opcion == 1) {
        int id = leerEntero("Ingrese ID de la zona: ", 1, MAX_ZONAS);
        int indice = buscarZonaPorId(sistema, id);
        if (indice == -1) {
            printf("Zona con ID %d no encontrada.\n", id);
        } else {
            printf("\nZona encontrada:\n");
            printf("-------------------------------------------------------------------------------------------------------------------------\n");
            printf("| %-3s | %-15s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-10s | %-11s | %-12s |\n", "ID", "Nombre", "CO2", "SO2", "NO2", "PM2.5", "Viento", "Humedad", "Temp", "Fecha", "ICA", "Recomendacion");
            printf("-------------------------------------------------------------------------------------------------------------------------\n");
            mostrarZona(sistema, &sistema->zonas[indice]);
            printf("-------------------------------------------------------------------------------------------------------------------------\n");
        }
    } else {
        char nombreBusqueda[MAX_NOMBRE];
        printf("Ingrese nombre exacto de la zona: ");
        if (fgets(nombreBusqueda, sizeof(nombreBusqueda), stdin) == NULL) {
            return;
        }
        size_t len = strlen(nombreBusqueda);
        if (len > 0 && nombreBusqueda[len - 1] == '\n') {
            nombreBusqueda[len - 1] = '\0';
        }
        int indice = buscarZonaPorNombre(sistema, nombreBusqueda);
        if (indice == -1) {
            printf("Zona con nombre '%s' no encontrada.\n", nombreBusqueda);
        } else {
            printf("\nZona encontrada:\n");
            printf("-------------------------------------------------------------------------------------------------------------------------\n");
            printf("| %-3s | %-15s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-10s | %-11s | %-12s |\n", "ID", "Nombre", "CO2", "SO2", "NO2", "PM2.5", "Viento", "Humedad", "Temp", "Fecha", "ICA", "Recomendacion");
            printf("-------------------------------------------------------------------------------------------------------------------------\n");
            mostrarZona(sistema, &sistema->zonas[indice]);
            printf("-------------------------------------------------------------------------------------------------------------------------\n");
        }
    }
}

static void eliminarZonaUsuario(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    int id = leerEntero("Ingrese ID de la zona a eliminar: ", 1, MAX_ZONAS);
    if (eliminarZona(sistema, id)) {
        printf("Zona %d eliminada correctamente.\n", id);
    } else {
        printf("No se encontro la zona o ya estaba eliminada.\n");
    }
}

int main(void) {
    SistemaZonas sistema;
    inicializarSistema(&sistema);
    if (!cargarDatosBinario(&sistema, NOMBRE_ARCHIVO)) {
        printf("No se pudo cargar la configuracion inicial. Se inicia sistema vacio.\n");
    }

    int ejecutando = 1;
    while (ejecutando) {
        mostrarMenu();
        int opcion = leerEntero("Seleccione una opcion: ", 1, 8);
        switch (opcion) {
            case 1:
                if (cargarDatosBinario(&sistema, NOMBRE_ARCHIVO)) {
                    printf("Datos cargados correctamente desde '%s'.\n", NOMBRE_ARCHIVO);
                } else {
                    printf("No se pudo cargar los datos desde '%s'.\n", NOMBRE_ARCHIVO);
                }
                break;
            case 2:
                ingresarActualizarZona(&sistema);
                break;
            case 3:
                mostrarEstadoActual(&sistema);
                break;
            case 4: {
                Fecha fechaActual = obtenerFechaActual();
                generarPrediccion24h(&sistema, &fechaActual);
                break;
            }
            case 5:
                mostrarPromediosHistoricos(&sistema);
                break;
            case 6:
                buscarZona(&sistema);
                break;
            case 7:
                eliminarZonaUsuario(&sistema);
                break;
            case 8:
                if (guardarDatosBinario(&sistema, NOMBRE_ARCHIVO)) {
                    printf("Datos guardados correctamente en '%s'.\n", NOMBRE_ARCHIVO);
                } else {
                    printf("Error al guardar los datos en '%s'.\n", NOMBRE_ARCHIVO);
                }
                ejecutando = 0;
                break;
            default:
                printf("Opcion invalida. Intente de nuevo.\n");
                break;
        }
    }
    return 0;
}
