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
    printf("8. Buscar zonas por texto en nombre\n");
    printf("9. Guardar y salir\n");
}

static void ingresarActualizarZona(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }

    int id = leerEntero("Ingrese ID de zona (1-5): ", 1, MAX_ZONAS);
    Zona nuevaZona;
    nuevaZona.id = id;
    nuevaZona.activo = 1;

    printf("Ingrese nombre de la zona: ");
    if (fgets(nuevaZona.nombre, sizeof(nuevaZona.nombre), stdin) == NULL) {
        return;
    }
    size_t len = strlen(nuevaZona.nombre);
    if (len > 0 && nuevaZona.nombre[len - 1] == '\n') {
        nuevaZona.nombre[len - 1] = '\0';
    }
    while (!validarCadena(nuevaZona.nombre)) {
        printf("Nombre invalido. Intente nuevamente: ");
        if (fgets(nuevaZona.nombre, sizeof(nuevaZona.nombre), stdin) == NULL) {
            return;
        }
        len = strlen(nuevaZona.nombre);
        if (len > 0 && nuevaZona.nombre[len - 1] == '\n') {
            nuevaZona.nombre[len - 1] = '\0';
        }
    }

    nuevaZona.contaminacionActual = leerFlotante("Ingrese contaminacion actual (0-500): ", 0.0f, 500.0f);
    nuevaZona.temperaturaActual = leerFlotante("Ingrese temperatura actual en C (-40.0-60.0): ", -40.0f, 60.0f);
    nuevaZona.humedadActual = leerFlotante("Ingrese humedad actual en % (0.0-100.0): ", 0.0f, 100.0f);

    int indiceExistente = buscarZonaPorId(sistema, id);
    if (indiceExistente != -1) {
        Zona *zonaExistente = &sistema->zonas[indiceExistente];
        nuevaZona.diasHistorico = zonaExistente->diasHistorico;
        memcpy(nuevaZona.historicoContaminacion, zonaExistente->historicoContaminacion, sizeof(zonaExistente->historicoContaminacion));
        actualizarRegistroHistorico(&nuevaZona, nuevaZona.contaminacionActual);
        sistema->zonas[indiceExistente] = nuevaZona;
        printf("Zona actualizada correctamente.\n");
    } else {
        nuevaZona.diasHistorico = 0;
        memset(nuevaZona.historicoContaminacion, 0, sizeof(nuevaZona.historicoContaminacion));
        actualizarRegistroHistorico(&nuevaZona, nuevaZona.contaminacionActual);
        if (agregarOActualizarZona(sistema, &nuevaZona)) {
            printf("Zona agregada correctamente.\n");
        } else {
            printf("No se pudo agregar la zona. El sistema ya contiene el maximo de zonas.\n");
        }
    }
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
            printf("--------------------------------------------------\n");
            printf("| ID  | Nombre              | Contaminacion | Temperatura | Humedad | Dias Historico |\n");
            printf("--------------------------------------------------\n");
            mostrarZona(&sistema->zonas[indice]);
            printf("--------------------------------------------------\n");
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
            printf("--------------------------------------------------\n");
            printf("| ID  | Nombre              | Contaminacion | Temperatura | Humedad | Dias Historico |\n");
            printf("--------------------------------------------------\n");
            mostrarZona(&sistema->zonas[indice]);
            printf("--------------------------------------------------\n");
        }
    }
}

static void buscarZonasTexto(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    char texto[MAX_NOMBRE];
    printf("Ingrese texto para buscar en nombres de zona: ");
    if (fgets(texto, sizeof(texto), stdin) == NULL) {
        return;
    }
    size_t len = strlen(texto);
    if (len > 0 && texto[len - 1] == '\n') {
        texto[len - 1] = '\0';
    }
    buscarZonasCoincidentes(sistema, texto);
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
    int ejecutando = 1;

    while (ejecutando) {
        mostrarMenu();
        int opcion = leerEntero("Seleccione una opcion: ", 1, 9);
        switch (opcion) {
            case 1:
                if (cargarDatosBinario(&sistema, NOMBRE_ARCHIVO)) {
                    printf("Datos cargados correctamente desde '%s'.\n", NOMBRE_ARCHIVO);
                } else {
                    printf("No se pudo cargar el archivo '%s'. Asegurese de que exista.\n", NOMBRE_ARCHIVO);
                }
                break;
            case 2:
                ingresarActualizarZona(&sistema);
                break;
            case 3:
                mostrarEstadoActual(&sistema);
                break;
            case 4:
                generarPrediccion24h(&sistema);
                break;
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
                buscarZonasTexto(&sistema);
                break;
            case 9:
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
