#include "funciones.h"

void inicializarSistema(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    sistema->cantidad = 0;
    for (int i = 0; i < MAX_ZONAS; i++) {
        sistema->zonas[i].activo = 0;
        sistema->zonas[i].id = i + 1;
        sistema->zonas[i].contaminacionActual = 0.0f;
        sistema->zonas[i].temperaturaActual = 0.0f;
        sistema->zonas[i].humedadActual = 0.0f;
        sistema->zonas[i].diasHistorico = 0;
        sistema->zonas[i].nombre[0] = '\0';
    }
}

int cargarDatosBinario(SistemaZonas *sistema, const char *nombreArchivo) {
    if (sistema == NULL || nombreArchivo == NULL) {
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        printf("[INFO] Archivo no encontrado. Creando nuevo archivo de base de datos...\n");
        archivo = fopen(nombreArchivo, "w");
        if (archivo == NULL) {
            perror("Error al crear el archivo");
            return 0;
        }

        if (fputs("0\n", archivo) == EOF) {
            perror("Error al escribir en el archivo");
            fclose(archivo);
            return 0;
        }

        if (fclose(archivo) != 0) {
            perror("Error al cerrar el archivo");
            return 0;
        }

        inicializarSistema(sistema);
        return 1;
    }

    char linea[16];
    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        if (fclose(archivo) != 0) {
            perror("Error al cerrar el archivo");
        }
        inicializarSistema(sistema);
        return 1;
    }

    if (fclose(archivo) != 0) {
        perror("Error al cerrar el archivo");
        return 0;
    }

    if (strcmp(linea, "0\n") == 0 || strcmp(linea, "0") == 0) {
        inicializarSistema(sistema);
        return 1;
    }

    archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL) {
        perror("Error al abrir el archivo para lectura binaria");
        return 0;
    }

    size_t leidos = fread(sistema, sizeof(SistemaZonas), 1, archivo);
    if (fclose(archivo) != 0) {
        perror("Error al cerrar el archivo");
        return 0;
    }

    return (leidos == 1) ? 1 : 0;
}

int guardarDatosBinario(const SistemaZonas *sistema, const char *nombreArchivo) {
    if (sistema == NULL || nombreArchivo == NULL) {
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL) {
        return 0;
    }

    size_t escritos = fwrite(sistema, sizeof(SistemaZonas), 1, archivo);
    fclose(archivo);
    return (escritos == 1) ? 1 : 0;
}

int buscarZonaPorId(const SistemaZonas *sistema, int id) {
    if (sistema == NULL) {
        return -1;
    }
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo && sistema->zonas[i].id == id) {
            return i;
        }
    }
    return -1;
}

int buscarZonaPorNombre(const SistemaZonas *sistema, const char *nombre) {
    if (sistema == NULL || nombre == NULL) {
        return -1;
    }

    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo && strcmp(sistema->zonas[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

int agregarOActualizarZona(SistemaZonas *sistema, const Zona *zona) {
    if (sistema == NULL || zona == NULL) {
        return 0;
    }

    int indice = buscarZonaPorId(sistema, zona->id);
    if (indice != -1) {
        sistema->zonas[indice] = *zona;
        return 1;
    }

    if (sistema->cantidad >= MAX_ZONAS) {
        return 0;
    }

    for (int i = 0; i < MAX_ZONAS; i++) {
        if (!sistema->zonas[i].activo) {
            sistema->zonas[i] = *zona;
            sistema->zonas[i].activo = 1;
            sistema->cantidad++;
            return 1;
        }
    }
    return 0;
}

int eliminarZona(SistemaZonas *sistema, int id) {
    if (sistema == NULL) {
        return 0;
    }

    int indice = buscarZonaPorId(sistema, id);
    if (indice == -1) {
        return 0;
    }

    sistema->zonas[indice].activo = 0;
    sistema->zonas[indice].nombre[0] = '\0';
    sistema->zonas[indice].contaminacionActual = 0.0f;
    sistema->zonas[indice].temperaturaActual = 0.0f;
    sistema->zonas[indice].humedadActual = 0.0f;
    sistema->zonas[indice].diasHistorico = 0;
    sistema->cantidad--;
    return 1;
}

void mostrarZona(const Zona *zona) {
    if (zona == NULL || !zona->activo) {
        return;
    }
    printf("| %3d | %-20s | %12.2f | %12.2f | %10.2f | %12d |\n",
           zona->id,
           zona->nombre,
           zona->contaminacionActual,
           zona->temperaturaActual,
           zona->humedadActual,
           zona->diasHistorico);
}

static const char *obtenerAlerta(float contaminacion) {
    if (contaminacion <= 50.0f) {
        return "Buena";
    } else if (contaminacion <= 100.0f) {
        return "Moderada";
    } else if (contaminacion <= 150.0f) {
        return "Insalubre";
    } else if (contaminacion <= 200.0f) {
        return "Peligrosa";
    } else {
        return "Extrema";
    }
}

void mostrarEstadoActual(const SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }

    printf("\nESTADO ACTUAL DE ZONAS\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("| ID  | Nombre              | Contaminacion | Temperatura C | Humedad %% | Alertas      |\n");
    printf("-------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            const char *alerta = obtenerAlerta(sistema->zonas[i].contaminacionActual);
            printf("| %3d | %-20s | %12.2f | %12.2f | %10.2f | %-12s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   sistema->zonas[i].contaminacionActual,
                   sistema->zonas[i].temperaturaActual,
                   sistema->zonas[i].humedadActual,
                   alerta);
        }
    }
    printf("-------------------------------------------------------------------------------\n");
}

float calcularPromedioHistorico(const Zona *zona) {
    if (zona == NULL || zona->diasHistorico <= 0) {
        return 0.0f;
    }
    float suma = 0.0f;
    for (int i = 0; i < zona->diasHistorico; i++) {
        suma += zona->historicoContaminacion[i];
    }
    return suma / zona->diasHistorico;
}

float calcularPromedioPonderado(const Zona *zona) {
    if (zona == NULL || zona->diasHistorico <= 0) {
        return 0.0f;
    }
    float pesoTotal = 0.0f;
    float sumaPonderada = 0.0f;
    for (int i = 0; i < zona->diasHistorico; i++) {
        float peso = (float)(i + 1);
        sumaPonderada += zona->historicoContaminacion[i] * peso;
        pesoTotal += peso;
    }
    return (pesoTotal > 0.0f) ? (sumaPonderada / pesoTotal) : 0.0f;
}

void generarPrediccion24h(const SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }

    printf("\nPREDICCION 24H\n");
    printf("-------------------------------------------------------------\n");
    printf("| ID  | Nombre              | Prediccion Contaminacion | Estado Previsto |\n");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            float prediccion = calcularPromedioPonderado(&sistema->zonas[i]);
            if (prediccion <= 0.0f) {
                prediccion = sistema->zonas[i].contaminacionActual;
            }
            const char *alerta = obtenerAlerta(prediccion);
            printf("| %3d | %-20s | %23.2f | %-15s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   prediccion,
                   alerta);
        }
    }
    printf("-------------------------------------------------------------\n");
}

void mostrarPromediosHistoricos(const SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }

    printf("\nPROMEDIOS HISTORICOS (ULTIMOS %d DIAS)\n", MAX_HISTORICO);
    printf("-------------------------------------------------------------\n");
    printf("| ID  | Nombre              | Promedio Historico | Dias Usados |\n");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            float promedio = calcularPromedioHistorico(&sistema->zonas[i]);
            printf("| %3d | %-20s | %18.2f | %11d |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   promedio,
                   sistema->zonas[i].diasHistorico);
        }
    }
    printf("-------------------------------------------------------------\n");
}

void buscarZonasCoincidentes(const SistemaZonas *sistema, const char *texto) {
    if (sistema == NULL || texto == NULL || texto[0] == '\0') {
        return;
    }

    printf("\nRESULTADOS DE BUSQUEDA\n");
    printf("-------------------------------------------------------------\n");
    printf("| ID  | Nombre              | Contaminacion Actual |\n");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            if (strstr(sistema->zonas[i].nombre, texto) != NULL) {
                printf("| %3d | %-20s | %19.2f |\n",
                       sistema->zonas[i].id,
                       sistema->zonas[i].nombre,
                       sistema->zonas[i].contaminacionActual);
            }
        }
    }
    printf("-------------------------------------------------------------\n");
}

void actualizarRegistroHistorico(Zona *zona, float nuevoValor) {
    if (zona == NULL) {
        return;
    }
    if (zona->diasHistorico < MAX_HISTORICO) {
        zona->historicoContaminacion[zona->diasHistorico++] = nuevoValor;
    } else {
        for (int i = 1; i < MAX_HISTORICO; i++) {
            zona->historicoContaminacion[i - 1] = zona->historicoContaminacion[i];
        }
        zona->historicoContaminacion[MAX_HISTORICO - 1] = nuevoValor;
    }
}

int validarCadena(const char *cadena) {
    if (cadena == NULL) {
        return 0;
    }
    while (*cadena != '\0') {
        if (*cadena != ' ' && *cadena != '\t' && *cadena != '\n' && *cadena != '\r') {
            return 1;
        }
        cadena++;
    }
    return 0;
}

int leerEntero(const char *mensaje, int min, int max) {
    char buffer[128];
    int valor = min - 1;
    while (1) {
        printf("%s", mensaje);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        char *fin;
        valor = (int)strtol(buffer, &fin, 10);
        if (fin == buffer || *fin != '\n') {
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }
        if (valor < min || valor > max) {
            printf("Valor fuera de rango. Debe estar entre %d y %d.\n", min, max);
            continue;
        }
        return valor;
    }
}

float leerFlotante(const char *mensaje, float min, float max) {
    char buffer[128];
    float valor = min - 1.0f;
    while (1) {
        printf("%s", mensaje);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        char *fin;
        valor = strtof(buffer, &fin);
        if (fin == buffer || (*fin != '\n' && *fin != '\0')) {
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }
        if (valor < min || valor > max) {
            printf("Valor fuera de rango. Debe estar entre %.2f y %.2f.\n", min, max);
            continue;
        }
        return valor;
    }
}
