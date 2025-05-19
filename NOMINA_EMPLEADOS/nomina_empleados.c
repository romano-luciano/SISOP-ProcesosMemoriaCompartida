
#include "nomina_empleados.h"

int cargarDatos(const char *nombreArchivo, Empleado *empleado) {
    FILE *archivo = fopen(nombreArchivo, "r");

    if (!archivo) {
        perror("Error al abrir el archivo");
        return 0;
    }

    char linea[MAXLINEA];
    int i = 0;

    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\n")] = 0;
        if (trozarLinea(linea, &empleado[i]) == 0) {
            i++;
        } else {
            printf("Error en línea %d\n", i + 1);
        }
    }

    fclose(archivo);
    return i;
}



int trozarLinea(char* linea, Empleado* empleado) {
    char* token;

    // Legajo
    token = strtok(linea, "|");
    if (!token) return -1;
    empleado->legajo = atoi(token);

    // Nombre y Apellido
    token = strtok(NULL, "|");
    if (!token) return -1;
    strcpy(empleado->nombre_y_ap, token);

    // Fecha
    token = strtok(NULL, "|");
    if (!token) return -1;
    printf("Fecha cruda: '%s'\n", token);
    sscanf(token, "%2d-%2d-%4d",
       &(empleado->fecha_ingreso.d),
       &(empleado->fecha_ingreso.m),
       &(empleado->fecha_ingreso.a));


    // Estado
    token = strtok(NULL, "|");
    if (!token) return -1;
    strcpy(empleado->estado, token);

    // Categoría
    token = strtok(NULL, "|");
    if (!token) return -1;
    strcpy(empleado->categoria, token);

    // Sueldo
    token = strtok(NULL, "|");
    if (!token) return -1;
    empleado->sueldo = atof(token);

    return 0;
}



