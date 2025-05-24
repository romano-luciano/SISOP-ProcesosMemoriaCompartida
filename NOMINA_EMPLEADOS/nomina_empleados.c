
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
    char* dato;//Va a contener el dato que vamos a guardar en las varibles

    // Legajo
    dato = strtok(linea, "|");
    if (!dato) return -1;
    empleado->legajo = atoi(dato);

    // Nombre y Apellido
    dato = strtok(NULL, "|");
    if (!dato) return -1;
    strcpy(empleado->nombre_y_ap, dato);

    // Fecha
    dato = strtok(NULL, "|");
    if (!dato) return -1;
    sscanf(dato, "%2d-%2d-%4d",
       &(empleado->fecha_ingreso.d),
       &(empleado->fecha_ingreso.m),
       &(empleado->fecha_ingreso.a));


    // Estado
    dato = strtok(NULL, "|");
    if (!dato) return -1;
    strcpy(empleado->estado, dato);

    // Categoría
    dato = strtok(NULL, "|");
    if (!dato) return -1;
    strcpy(empleado->categoria, dato);

    // Sueldo
    dato = strtok(NULL, "|");
    if (!dato) return -1;
    empleado->sueldo = atof(dato);

    return 0;
}

void mostrarEmpleados(Empleado *empleados, int cantidad) {
    printf("%s | %s | %s | %s | %s | %s","Legajo", "Nombre y Apellido", "Fecha", "Estado", "Categoría", "Sueldo");
    printf("\n--------------------------------------------------------------------------------\n");

    for (int i = 0; i < cantidad; i++) {
        printf("%d | %s | %d-%d-%d | %s | %s | %f\n",
               empleados[i].legajo,
               empleados[i].nombre_y_ap,
               empleados[i].fecha_ingreso.d,
               empleados[i].fecha_ingreso.m,
               empleados[i].fecha_ingreso.a,
               empleados[i].estado,
               empleados[i].categoria,
               empleados[i].sueldo);

    printf("\n");

    }
}


