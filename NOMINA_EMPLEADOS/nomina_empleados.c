
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

//////////PROCESO HIJO 1 - ELIMINAR EMPLEADOS INACTIVOS/////////////////////
int eliminarEmpleadosInactivos(Empleado *empleados, int *cantidad){
    int cantEmpleadosElim=0;

    for (int i = 0; i < *cantidad; i++) {
        //Preguntamos si el empleado esta 'inactivo' (camparamos todo en minusculas)
        if (strcmp(empleados[i].estado, "inactivo") == 0) {
            //Aumentamos el contador de empleados eliminados
            cantEmpleadosElim++;
            //puts("Se elimino");
            // No lo eliminamos como tal, sino que movemos los regiustros de adelante, pisando el registro
            for (int j = i; j < *cantidad - 1; j++) {
                empleados[j] = empleados[j + 1];
            }
            (*cantidad)--; //Decrementamos la cantidad de empleados del vector
            i--; //Como movemos los registros de adelante, tenemos que evaluar la misma posicion en donde queda el elemento que estaba adelante antes
        }
    }

    printf("Se eliminaron %d empleados\n",cantEmpleadosElim);
    return cantEmpleadosElim;

}

//////////PROCESO HIJO 4 - ACTUALIZAR SUELDOS SEGUN CATEGORIA/////////////////////
int cargarCategoria(const char *nombreArchivo, Categoria *categoria)
{
   FILE* archivoCategoria = fopen(nombreArchivo,"r");

    if (!archivoCategoria) {
        perror("Error al abrir el archivo");
        return 0;
    }

    char lineaCategoria[MAX_LINEACAT];
    int i = 0;

    while(fgets(lineaCategoria, sizeof(lineaCategoria), archivoCategoria))
    {
        lineaCategoria[strcspn(lineaCategoria, "\n")] = 0;

        if (trozarLineaCategoria(lineaCategoria, &categoria[i]) == 0) {
            i++;
        } else {
            printf("Error en línea %d\n", i + 1);
        }
    }
    fclose(archivoCategoria);
    return i;
}

int trozarLineaCategoria(char *linea, Categoria *categoria)
{
    char *dato;

    //CATEGORIA
    dato = strtok(linea, "|");
    if (!dato) return -1;
    strcpy(categoria->categoria, dato);

    //AUMENTO
    dato = strtok(NULL, "|"); //Para que me de lo que sigue despues del | que es el aumento
    if (!dato) return -1;
    categoria->aumento = atof(dato); //Convierte string a float

    return TODO_OK;
}

void actualizarSueldos(const char *nombreArchivo, Empleado *empleados, int cantEmp, Categoria *categorias, int cantCat)
{
    FILE *archivo = fopen(nombreArchivo,"w");

    if(!archivo){
        perror("Error al crear nomina_actualizada.txt");
        return;
    }

    for(int i = 0; i < cantEmp; i++)
    {
        for(int j = 0; j< cantCat; j++)
        {
            if(strcmp(empleados[i].categoria, categorias[j].categoria) == 0)
            {
                empleados[i].sueldo *= (1 + categorias[j].aumento /100.0);
                break;
            }
        }
            fprintf(archivo, "%d|%s|%02d/%02d/%04d|%s|%s|%.2f\n",
                empleados[i].legajo,
                empleados[i].nombre_y_ap,
                empleados[i].fecha_ingreso.d,
                empleados[i].fecha_ingreso.m,
                empleados[i].fecha_ingreso.a,
                empleados[i].estado,
                empleados[i].categoria,
                empleados[i].sueldo);
    }

    fclose(archivo);
}
