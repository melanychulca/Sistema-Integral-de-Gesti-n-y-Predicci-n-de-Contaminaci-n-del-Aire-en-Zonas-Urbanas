#include "funciones.h"

static int validarFecha(const Fecha *fecha) {
    if (fecha == NULL) {
        return 0;
    }
    if (fecha->anio < 1900 || fecha->anio > 2100) {
        return 0;
    }
    if (fecha->mes < 1 || fecha->mes > 12) {
        return 0;
    }
    if (fecha->dia < 1 || fecha->dia > 31) {
        return 0;
    }
    if ((fecha->mes == 4 || fecha->mes == 6 || fecha->mes == 9 || fecha->mes == 11) && fecha->dia > 30) {
        return 0;
    }
    if (fecha->mes == 2) {
        int bisiesto = (fecha->anio % 400 == 0) || ((fecha->anio % 4 == 0) && (fecha->anio % 100 != 0));
        int maxDia = bisiesto ? 29 : 28;
        if (fecha->dia > maxDia) {
            return 0;
        }
    }
    return 1;
}

static int leerFecha(Fecha *fecha) {
    if (fecha == NULL) {
        return 0;
    }

    while (1) {
        fecha->dia = leerEntero("Ingrese dia (1-31): ", 1, 31);
        fecha->mes = leerEntero("Ingrese mes (1-12): ", 1, 12);
        fecha->anio = leerEntero("Ingrese anio (1900-2100): ", 1900, 2100);
        if (validarFecha(fecha)) {
            return 1;
        }
        printf("Fecha invalida. Intente de nuevo.\n");
    }
}

float obtenerIndiceContaminacion(const Zona *zona) {
    if (zona == NULL) {
        return 0.0f;
    }
    return (zona->co2 + zona->so2 + zona->no2 + zona->pm25) / 4.0f;
}

void inicializarSistema(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    sistema->cantidad = 0;
    for (int i = 0; i < MAX_ZONAS; i++) {
        sistema->zonas[i].activo = 0;
        sistema->zonas[i].id = i + 1;
        sistema->zonas[i].co2 = 0.0f;
        sistema->zonas[i].so2 = 0.0f;
        sistema->zonas[i].no2 = 0.0f;
        sistema->zonas[i].pm25 = 0.0f;
        sistema->zonas[i].velocidadViento = 0.0f;
        sistema->zonas[i].humedad = 0.0f;
        sistema->zonas[i].temperaturaActual = 0.0f;
        sistema->zonas[i].fecha.dia = 0;
        sistema->zonas[i].fecha.mes = 0;
        sistema->zonas[i].fecha.anio = 0;
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
    sistema->zonas[indice].co2 = 0.0f;
    sistema->zonas[indice].so2 = 0.0f;
    sistema->zonas[indice].no2 = 0.0f;
    sistema->zonas[indice].pm25 = 0.0f;
    sistema->zonas[indice].velocidadViento = 0.0f;
    sistema->zonas[indice].humedad = 0.0f;
    sistema->zonas[indice].temperaturaActual = 0.0f;
    sistema->zonas[indice].fecha.dia = 0;
    sistema->zonas[indice].fecha.mes = 0;
    sistema->zonas[indice].fecha.anio = 0;
    sistema->zonas[indice].diasHistorico = 0;
    sistema->cantidad--;
    return 1;
}

void mostrarZona(const Zona *zona) {
    if (zona == NULL || !zona->activo) {
        return;
    }
    printf("| %3d | %-20s | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %02d/%02d/%04d | %3d |\n",
           zona->id,
           zona->nombre,
           zona->co2,
           zona->so2,
           zona->no2,
           zona->pm25,
           zona->velocidadViento,
           zona->humedad,
           zona->temperaturaActual,
           zona->fecha.dia,
           zona->fecha.mes,
           zona->fecha.anio,
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
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("| ID  | Nombre              | CO2      | SO2      | NO2      | PM2.5    | Viento   | Humedad  | Temp     | Fecha       | Alerta      |\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            float indice = obtenerIndiceContaminacion(&sistema->zonas[i]);
            const char *alerta = obtenerAlerta(indice);
            printf("| %3d | %-20s | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %02d/%02d/%04d | %-12s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   sistema->zonas[i].co2,
                   sistema->zonas[i].so2,
                   sistema->zonas[i].no2,
                   sistema->zonas[i].pm25,
                   sistema->zonas[i].velocidadViento,
                   sistema->zonas[i].humedad,
                   sistema->zonas[i].temperaturaActual,
                   sistema->zonas[i].fecha.dia,
                   sistema->zonas[i].fecha.mes,
                   sistema->zonas[i].fecha.anio,
                   alerta);
        }
    }
    printf("--------------------------------------------------------------------------------------------------------------------\n");
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
    printf("-------------------------------------------------------------------\n");
    printf("| ID  | Nombre              | Prediccion Contaminacion | Estado Previsto |\n");
    printf("-------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            float prediccion = calcularPromedioPonderado(&sistema->zonas[i]);
            if (prediccion <= 0.0f) {
                prediccion = obtenerIndiceContaminacion(&sistema->zonas[i]);
            }
            const char *alerta = obtenerAlerta(prediccion);
            printf("| %3d | %-20s | %23.2f | %-15s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   prediccion,
                   alerta);
        }
    }
    printf("-------------------------------------------------------------------\n");
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
    printf("---------------------------------------------------------------------\n");
    printf("| ID  | Nombre              | Indice Contaminacion | Fecha       |\n");
    printf("---------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            if (strstr(sistema->zonas[i].nombre, texto) != NULL) {
                float indice = obtenerIndiceContaminacion(&sistema->zonas[i]);
                printf("| %3d | %-20s | %20.2f | %02d/%02d/%04d |\n",
                       sistema->zonas[i].id,
                       sistema->zonas[i].nombre,
                       indice,
                       sistema->zonas[i].fecha.dia,
                       sistema->zonas[i].fecha.mes,
                       sistema->zonas[i].fecha.anio);
            }
        }
    }
    printf("---------------------------------------------------------------------\n");
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

void ingresarActualizarZona(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }

    char mensaje[60];
    sprintf(mensaje, "Ingrese ID de zona (1-%d): ", MAX_ZONAS);
    int id = leerEntero(mensaje, 1, MAX_ZONAS);
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

    nuevaZona.co2 = leerFlotante("Ingrese CO2 (0-1000): ", 0.0f, 1000.0f);
    nuevaZona.so2 = leerFlotante("Ingrese SO2 (0-1000): ", 0.0f, 1000.0f);
    nuevaZona.no2 = leerFlotante("Ingrese NO2 (0-1000): ", 0.0f, 1000.0f);
    nuevaZona.pm25 = leerFlotante("Ingrese PM2.5 (0-1000): ", 0.0f, 1000.0f);
    nuevaZona.velocidadViento = leerFlotante("Ingrese velocidad del viento (0-200): ", 0.0f, 200.0f);
    nuevaZona.humedad = leerFlotante("Ingrese humedad (0-100): ", 0.0f, 100.0f);
    nuevaZona.temperaturaActual = leerFlotante("Ingrese temperatura actual en C (-40.0-60.0): ", -40.0f, 60.0f);

    printf("Ingrese la fecha actual del registro.\n");
    if (!leerFecha(&nuevaZona.fecha)) {
        printf("No se pudo leer la fecha.\n");
        return;
    }

    int indiceExistente = buscarZonaPorId(sistema, id);
    if (indiceExistente != -1) {
        Zona *zonaExistente = &sistema->zonas[indiceExistente];
        nuevaZona.diasHistorico = zonaExistente->diasHistorico;
        memcpy(nuevaZona.historicoContaminacion, zonaExistente->historicoContaminacion, sizeof(zonaExistente->historicoContaminacion));
        actualizarRegistroHistorico(&nuevaZona, obtenerIndiceContaminacion(&nuevaZona));
        sistema->zonas[indiceExistente] = nuevaZona;
        printf("Zona actualizada correctamente.\n");
    } else {
        nuevaZona.diasHistorico = 0;
        memset(nuevaZona.historicoContaminacion, 0, sizeof(nuevaZona.historicoContaminacion));
        actualizarRegistroHistorico(&nuevaZona, obtenerIndiceContaminacion(&nuevaZona));
        if (agregarOActualizarZona(sistema, &nuevaZona)) {
            printf("Zona agregada correctamente.\n");
        } else {
            printf("No se pudo agregar la zona. El sistema ya contiene el maximo de zonas.\n");
        }
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
