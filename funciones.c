#include "funciones.h"
#include <ctype.h>
#include <string.h>
#include <time.h>

static void limpiarEntrada(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        ;
    }
}

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
        if (fecha->dia > (bisiesto ? 29 : 28)) {
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

static int leerCadena(const char *mensaje, char *buffer, int maxLen) {
    if (mensaje == NULL || buffer == NULL || maxLen <= 0) {
        return 0;
    }
    printf("%s", mensaje);
    if (fgets(buffer, maxLen, stdin) == NULL) {
        return 0;
    }
    size_t longitud = strlen(buffer);
    if (longitud > 0 && buffer[longitud - 1] == '\n') {
        buffer[longitud - 1] = '\0';
    } else if (longitud == (size_t)maxLen - 1 && buffer[longitud - 1] != '\n') {
        limpiarEntrada();
    }
    return 1;
}

static int compararCadenasNoCase(const char *a, const char *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    while (*a != '\0' && *b != '\0') {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

static int contieneSubcadenaNoCase(const char *texto, const char *busqueda) {
    if (texto == NULL || busqueda == NULL) {
        return 0;
    }
    size_t lenBusqueda = strlen(busqueda);
    if (lenBusqueda == 0) {
        return 1;
    }
    for (size_t i = 0; texto[i] != '\0'; i++) {
        size_t j;
        for (j = 0; j < lenBusqueda; j++) {
            char ca = (char)tolower((unsigned char)texto[i + j]);
            char cb = (char)tolower((unsigned char)busqueda[j]);
            if (ca != cb) {
                break;
            }
        }
        if (j == lenBusqueda) {
            return 1;
        }
    }
    return 0;
}

static int esDiaHabil(const Fecha *fecha) {
    if (fecha == NULL) {
        return 0;
    }
    struct tm tmFecha = {0};
    tmFecha.tm_mday = fecha->dia;
    tmFecha.tm_mon = fecha->mes - 1;
    tmFecha.tm_year = fecha->anio - 1900;
    mktime(&tmFecha);
    return tmFecha.tm_wday >= 1 && tmFecha.tm_wday <= 5;
}

static float calcularICA(const Registro *registro) {
    if (registro == NULL) {
        return 0.0f;
    }
    return registro->co2 * 0.20f + registro->so2 * 0.25f + registro->no2 * 0.30f + registro->pm25 * 0.25f;
}

static const char *obtenerRecomendacion(float ica) {
    if (ica > 150.0f) {
        return "Evite actividades al aire libre";
    }
    if (ica > 100.0f) {
        return "Reduzca esfuerzo fisico";
    }
    if (ica > 50.0f) {
        return "Precaucion si es sensible";
    }
    return "Condiciones favorables";
}

static const char *obtenerCategoriaICA(float ica) {
    if (ica > 150.0f) {
        return "Muy alta";
    }
    if (ica > 100.0f) {
        return "Alta";
    }
    if (ica > 50.0f) {
        return "Moderada";
    }
    return "Baja";
}

static const char *obtenerContaminantePrincipal(const Registro *registro) {
    if (registro == NULL) {
        return "Ninguno";
    }
    float mayor = registro->co2;
    const char *nombre = "CO2";
    if (registro->so2 > mayor) {
        mayor = registro->so2;
        nombre = "SO2";
    }
    if (registro->no2 > mayor) {
        mayor = registro->no2;
        nombre = "NO2";
    }
    if (registro->pm25 > mayor) {
        mayor = registro->pm25;
        nombre = "PM2.5";
    }
    return nombre;
}

static float aplicarCorreccionesQuito(const Zona *zona, float indice, const Fecha *fechaActual) {
    float factor = 1.0f;
    if (esDiaHabil(fechaActual)) {
        factor *= FACTOR_PICO_PLACA;
    }
    if (contieneSubcadenaNoCase(zona->nombre, "Quito")) {
        factor *= 0.92f;
    }
    return indice * factor;
}

static int encontrarIndiceUltimoRegistro(const SistemaZonas *sistema, int idZona) {
    if (sistema == NULL) {
        return -1;
    }
    for (int i = sistema->cantidadRegistros - 1; i >= 0; i--) {
        if (sistema->registros[i].id_zona == idZona) {
            return i;
        }
    }
    return -1;
}

static int cargarRegistrosDesdeArchivo(SistemaZonas *sistema, const char *nombreArchivo) {
    if (sistema == NULL || nombreArchivo == NULL) {
        return 0;
    }
    FILE *archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL) {
        return 1;
    }
    sistema->cantidadRegistros = 0;
    while (sistema->cantidadRegistros < MAX_REGISTROS) {
        Registro registro;
        if (fread(&registro, sizeof(Registro), 1, archivo) != 1) {
            break;
        }
        sistema->registros[sistema->cantidadRegistros++] = registro;
    }
    fclose(archivo);
    return 1;
}

static int crearArchivoVacioSiNoExiste(const char *nombreArchivo) {
    if (nombreArchivo == NULL) {
        return 0;
    }
    FILE *archivo = fopen(nombreArchivo, "ab");
    if (archivo == NULL) {
        return 0;
    }
    fclose(archivo);
    return 1;
}

static int guardarRegistrosCompleto(const SistemaZonas *sistema, const char *nombreArchivo) {
    if (sistema == NULL || nombreArchivo == NULL) {
        return 0;
    }
    FILE *archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL) {
        return 0;
    }
    size_t escritos = fwrite(sistema->registros, sizeof(Registro), sistema->cantidadRegistros, archivo);
    fclose(archivo);
    return escritos == (size_t)sistema->cantidadRegistros;
}

void inicializarSistema(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    sistema->cantidadZonas = 0;
    sistema->cantidadRegistros = 0;
    for (int i = 0; i < MAX_ZONAS; i++) {
        sistema->zonas[i].id = i + 1;
        sistema->zonas[i].activo = 0;
        sistema->zonas[i].nombre[0] = '\0';
    }
}

int cargarZonasBinario(SistemaZonas *sistema, const char *nombreArchivo) {
    if (sistema == NULL || nombreArchivo == NULL) {
        return 0;
    }
    crearArchivoVacioSiNoExiste(nombreArchivo);
    FILE *archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL) {
        return 1;
    }
    if (fread(&sistema->cantidadZonas, sizeof(sistema->cantidadZonas), 1, archivo) != 1) {
        fclose(archivo);
        return 1;
    }
    if (fread(sistema->zonas, sizeof(Zona), MAX_ZONAS, archivo) != MAX_ZONAS) {
        fclose(archivo);
        return 0;
    }
    fclose(archivo);
    return 1;
}

int guardarZonasBinario(const SistemaZonas *sistema, const char *nombreArchivo) {
    if (sistema == NULL || nombreArchivo == NULL) {
        return 0;
    }
    FILE *archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL) {
        return 0;
    }
    if (fwrite(&sistema->cantidadZonas, sizeof(sistema->cantidadZonas), 1, archivo) != 1) {
        fclose(archivo);
        return 0;
    }
    size_t escritos = fwrite(sistema->zonas, sizeof(Zona), MAX_ZONAS, archivo);
    fclose(archivo);
    return escritos == MAX_ZONAS;
}

int cargarRegistrosBinario(SistemaZonas *sistema, const char *nombreArchivo) {
    crearArchivoVacioSiNoExiste(nombreArchivo);
    return cargarRegistrosDesdeArchivo(sistema, nombreArchivo);
}

int guardarRegistroBinario(const Registro *registro, const char *nombreArchivo) {
    if (registro == NULL || nombreArchivo == NULL) {
        return 0;
    }
    FILE *archivo = fopen(nombreArchivo, "ab");
    if (archivo == NULL) {
        return 0;
    }
    size_t escritos = fwrite(registro, sizeof(Registro), 1, archivo);
    fclose(archivo);
    return escritos == 1;
}

int cargarDatosBinario(SistemaZonas *sistema, const char *nombreArchivoZonas) {
    if (sistema == NULL || nombreArchivoZonas == NULL) {
        return 0;
    }
    inicializarSistema(sistema);
    if (!cargarZonasBinario(sistema, nombreArchivoZonas)) {
        return 0;
    }
    if (!cargarRegistrosBinario(sistema, NOMBRE_ARCHIVO_REGISTROS)) {
        return 0;
    }
    return 1;
}

int guardarDatosBinario(const SistemaZonas *sistema, const char *nombreArchivoZonas) {
    if (sistema == NULL || nombreArchivoZonas == NULL) {
        return 0;
    }
    if (!guardarZonasBinario(sistema, nombreArchivoZonas)) {
        return 0;
    }
    if (!guardarRegistrosCompleto(sistema, NOMBRE_ARCHIVO_REGISTROS)) {
        return 0;
    }
    return 1;
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
        if (sistema->zonas[i].activo && compararCadenasNoCase(sistema->zonas[i].nombre, nombre)) {
            return i;
        }
    }
    return -1;
}

int agregarZona(SistemaZonas *sistema, const Zona *zona) {
    if (sistema == NULL || zona == NULL) {
        return 0;
    }
    int indice = buscarZonaPorId(sistema, zona->id);
    if (indice != -1) {
        sistema->zonas[indice] = *zona;
        return 1;
    }
    if (sistema->cantidadZonas >= MAX_ZONAS) {
        return 0;
    }
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (!sistema->zonas[i].activo) {
            sistema->zonas[i] = *zona;
            sistema->zonas[i].activo = 1;
            sistema->cantidadZonas++;
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
    sistema->cantidadZonas--;
    return 1;
}

void mostrarZona(const SistemaZonas *sistema, const Zona *zona) {
    if (sistema == NULL || zona == NULL || !zona->activo) {
        return;
    }
    int indiceRegistro = encontrarIndiceUltimoRegistro(sistema, zona->id);
    const Registro *registro = NULL;
    float ica = 0.0f;
    const char *recomendacion = "Sin datos historicos";
    if (indiceRegistro != -1) {
        registro = &sistema->registros[indiceRegistro];
        ica = calcularICA(registro);
        recomendacion = obtenerRecomendacion(ica);
    }
    const char *contaminantePrincipal = (registro != NULL) ? obtenerContaminantePrincipal(registro) : "Ninguno";
    printf("| %3d | %-20s | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %02d/%02d/%04d | %7.2f | %-10s | %-12s | %-16s |\n",
           zona->id,
           zona->nombre,
           (registro != NULL) ? registro->co2 : 0.0f,
           (registro != NULL) ? registro->so2 : 0.0f,
           (registro != NULL) ? registro->no2 : 0.0f,
           (registro != NULL) ? registro->pm25 : 0.0f,
           (registro != NULL) ? registro->velocidadViento : 0.0f,
           (registro != NULL) ? registro->humedad : 0.0f,
           (registro != NULL) ? registro->temperaturaActual : 0.0f,
           (registro != NULL) ? registro->fecha.dia : 0,
           (registro != NULL) ? registro->fecha.mes : 0,
           (registro != NULL) ? registro->fecha.anio : 0,
           ica,
           obtenerCategoriaICA(ica),
           contaminantePrincipal,
           recomendacion);
}

void mostrarEstadoActual(const SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    printf("\nESTADO ACTUAL DE ZONAS\n");
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-3s | %-20s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-10s | %-7s | %-10s | %-12s | %-16s |\n",
           "ID", "Nombre", "CO2", "SO2", "NO2", "PM2.5", "Viento", "Humedad", "Temp", "Fecha", "ICA", "Cat.", "Contam.", "Recomendacion");
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (sistema->zonas[i].activo) {
            mostrarZona(sistema, &sistema->zonas[i]);
        }
    }
    printf("------------------------------------------------------------------------------------------------------------------------------\n");
}

void generarPrediccion24h(const SistemaZonas *sistema, const Fecha *fechaActual) {
    if (sistema == NULL || fechaActual == NULL) {
        return;
    }
    printf("\nPREDICCION 24 HORAS\n");
    printf("--------------------------------------------------------------------------------------------------------------\n");
    printf("| %-3s | %-20s | %-8s | %-8s | %-8s | %-8s | %-8s | %-9s | %-10s | %-7s | %-10s | %-12s | %-26s |\n",
           "ID", "Nombre", "CO2", "SO2", "NO2", "PM2.5", "Viento", "Humedad", "Fecha", "ICA", "Cat.", "Contam.", "Recomendacion");
    printf("--------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (!sistema->zonas[i].activo) {
            continue;
        }
        int ultimoIndice = encontrarIndiceUltimoRegistro(sistema, sistema->zonas[i].id);
        if (ultimoIndice == -1) {
            printf("| %3d | %-20s | %8s | %8s | %8s | %8s | %8s | %9s | %10s | %7s | %10s | %-12s | %-26s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   "--", "--", "--", "--", "--", "--", "--", "--", "--", "Ninguno", "Sin datos historicos");
            continue;
        }
        const Registro *registroReciente = &sistema->registros[ultimoIndice];
        int registrosDisponibles = 0;
        Registro acumulado = {0};
        for (int j = ultimoIndice; j >= 0 && registrosDisponibles < 5; j--) {
            if (sistema->registros[j].id_zona == sistema->zonas[i].id) {
                acumulado.co2 += sistema->registros[j].co2;
                acumulado.so2 += sistema->registros[j].so2;
                acumulado.no2 += sistema->registros[j].no2;
                acumulado.pm25 += sistema->registros[j].pm25;
                acumulado.velocidadViento += sistema->registros[j].velocidadViento;
                acumulado.humedad += sistema->registros[j].humedad;
                acumulado.temperaturaActual += sistema->registros[j].temperaturaActual;
                registrosDisponibles++;
            }
        }
        if (registrosDisponibles == 0) {
            printf("| %3d | %-20s | %8s | %8s | %8s | %8s | %8s | %9s | %7s | %-26s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   "--", "--", "--", "--", "--", "--", "--", "Sin datos historicos");
            continue;
        }
        acumulado.co2 /= registrosDisponibles;
        acumulado.so2 /= registrosDisponibles;
        acumulado.no2 /= registrosDisponibles;
        acumulado.pm25 /= registrosDisponibles;
        acumulado.velocidadViento /= registrosDisponibles;
        acumulado.humedad /= registrosDisponibles;
        acumulado.temperaturaActual /= registrosDisponibles;
        float indice = calcularICA(&acumulado);
        float indiceCorregido = aplicarCorreccionesQuito(&sistema->zonas[i], indice, fechaActual);
        const char *contaminantePrincipal = obtenerContaminantePrincipal(&acumulado);
        printf("| %3d | %-20s | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %9.2f | %02d/%02d/%04d | %7.2f | %-10s | %-12s | %-26s |\n",
               sistema->zonas[i].id,
               sistema->zonas[i].nombre,
               acumulado.co2,
               acumulado.so2,
               acumulado.no2,
               acumulado.pm25,
               acumulado.velocidadViento,
               acumulado.humedad,
               registroReciente->fecha.dia,
               registroReciente->fecha.mes,
               registroReciente->fecha.anio,
               indiceCorregido,
               obtenerCategoriaICA(indiceCorregido),
               contaminantePrincipal,
               obtenerRecomendacion(indiceCorregido));
    }
    printf("--------------------------------------------------------------------------------------------------------------\n");
}

void mostrarPromediosHistoricos(const SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    printf("\nPROMEDIOS HISTORICOS (ULTIMOS REGISTROS)\n");
    printf("------------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-3s | %-20s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-7s |\n",
           "ID", "Nombre", "CO2", "SO2", "NO2", "PM2.5", "Viento", "Humedad", "Temp", "ICA");
    printf("------------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (!sistema->zonas[i].activo) {
            continue;
        }
        int registrosZona = 0;
        Registro acumulado = {0};
        for (int j = 0; j < sistema->cantidadRegistros; j++) {
            if (sistema->registros[j].id_zona == sistema->zonas[i].id) {
                acumulado.co2 += sistema->registros[j].co2;
                acumulado.so2 += sistema->registros[j].so2;
                acumulado.no2 += sistema->registros[j].no2;
                acumulado.pm25 += sistema->registros[j].pm25;
                acumulado.velocidadViento += sistema->registros[j].velocidadViento;
                acumulado.humedad += sistema->registros[j].humedad;
                acumulado.temperaturaActual += sistema->registros[j].temperaturaActual;
                registrosZona++;
            }
        }
        if (registrosZona == 0) {
            continue;
        }
        acumulado.co2 /= registrosZona;
        acumulado.so2 /= registrosZona;
        acumulado.no2 /= registrosZona;
        acumulado.pm25 /= registrosZona;
        acumulado.velocidadViento /= registrosZona;
        acumulado.humedad /= registrosZona;
        acumulado.temperaturaActual /= registrosZona;
        float ica = calcularICA(&acumulado);
        printf("| %3d | %-20s | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %8.2f | %7.2f |\n",
               sistema->zonas[i].id,
               sistema->zonas[i].nombre,
               acumulado.co2,
               acumulado.so2,
               acumulado.no2,
               acumulado.pm25,
               acumulado.velocidadViento,
               acumulado.humedad,
               acumulado.temperaturaActual,
               ica);
    }
    printf("------------------------------------------------------------------------------------------------------------------------\n");
}

void buscarZonasCoincidentes(const SistemaZonas *sistema, const char *texto) {
    if (sistema == NULL || texto == NULL || texto[0] == '\0') {
        return;
    }
    printf("\nRESULTADOS DE BUSQUEDA\n");
    printf("---------------------------------------------------------------\n");
    printf("| %-3s | %-20s | %-6s |\n", "ID", "Nombre", "Activo");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < MAX_ZONAS; i++) {
        if (!sistema->zonas[i].activo) {
            continue;
        }
        char idTexto[16];
        snprintf(idTexto, sizeof(idTexto), "%d", sistema->zonas[i].id);
        if (contieneSubcadenaNoCase(sistema->zonas[i].nombre, texto) || contieneSubcadenaNoCase(idTexto, texto)) {
            printf("| %3d | %-20s | %-6s |\n",
                   sistema->zonas[i].id,
                   sistema->zonas[i].nombre,
                   sistema->zonas[i].activo ? "Si" : "No");
        }
    }
    printf("---------------------------------------------------------------\n");
}

void ingresarActualizarZona(SistemaZonas *sistema) {
    if (sistema == NULL) {
        return;
    }
    Zona zona = {0};
    int id = leerEntero("Ingrese ID de la zona (1-20): ", 1, MAX_ZONAS);
    zona.id = id;
    zona.activo = 1;
    if (!leerCadena("Ingrese nombre de la zona: ", zona.nombre, MAX_NOMBRE) || !validarCadena(zona.nombre)) {
        printf("Nombre de zona invalido. Operacion cancelada.\n");
        return;
    }
    Registro registro = {0};
    registro.id_zona = id;
    if (!leerFecha(&registro.fecha)) {
        printf("No se pudo leer la fecha.\n");
        return;
    }
    registro.co2 = leerFlotante("Ingrese valor CO2 (0-500): ", 0.0f, 500.0f);
    registro.so2 = leerFlotante("Ingrese valor SO2 (0-500): ", 0.0f, 500.0f);
    registro.no2 = leerFlotante("Ingrese valor NO2 (0-500): ", 0.0f, 500.0f);
    registro.pm25 = leerFlotante("Ingrese valor PM2.5 (0-500): ", 0.0f, 500.0f);
    registro.velocidadViento = leerFlotante("Ingrese velocidad del viento (0-200 km/h): ", 0.0f, 200.0f);
    registro.humedad = leerFlotante("Ingrese humedad (%) (0-100): ", 0.0f, 100.0f);
    registro.temperaturaActual = leerFlotante("Ingrese temperatura actual (Celsius, -50 a 60): ", -50.0f, 60.0f);
    if (sistema->cantidadRegistros >= MAX_REGISTROS) {
        memmove(&sistema->registros[0], &sistema->registros[1], sizeof(Registro) * (MAX_REGISTROS - 1));
        sistema->cantidadRegistros = MAX_REGISTROS - 1;
    }
    sistema->registros[sistema->cantidadRegistros++] = registro;
    if (!agregarZona(sistema, &zona)) {
        printf("No se pudo agregar la zona. El sistema ya tiene el maximo de zonas.\n");
        return;
    }
    if (!guardarRegistroBinario(&registro, NOMBRE_ARCHIVO_REGISTROS)) {
        printf("Advertencia: no se pudo guardar el registro en disco.\n");
    }
    printf("Zona '%s' actualizada correctamente y registro agregado.\n", zona.nombre);
}

int validarCadena(const char *cadena) {
    if (cadena == NULL) {
        return 0;
    }
    while (*cadena != '\0') {
        if (!isspace((unsigned char)*cadena)) {
            return 1;
        }
        cadena++;
    }
    return 0;
}

int validarValorFlotante(float valor, float min, float max) {
    return valor >= min && valor <= max;
}

int leerEntero(const char *mensaje, int min, int max) {
    char buffer[128];
    long valor;
    char *endptr;
    while (1) {
        printf("%s", mensaje);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        valor = strtol(buffer, &endptr, 10);
        if (endptr == buffer || (*endptr != '\n' && *endptr != '\0')) {
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }
        if (valor < min || valor > max) {
            printf("Valor fuera de rango (%d-%d). Intente de nuevo.\n", min, max);
            continue;
        }
        return (int)valor;
    }
}

float leerFlotante(const char *mensaje, float min, float max) {
    char buffer[128];
    float valor;
    char *endptr;
    while (1) {
        printf("%s", mensaje);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        valor = strtof(buffer, &endptr);
        if (endptr == buffer || (*endptr != '\n' && *endptr != '\0')) {
            printf("Entrada invalida. Intente de nuevo.\n");
            continue;
        }
        if (!validarValorFlotante(valor, min, max)) {
            printf("Valor fuera de rango (%.2f-%.2f). Intente de nuevo.\n", min, max);
            continue;
        }
        return valor;
    }
}
