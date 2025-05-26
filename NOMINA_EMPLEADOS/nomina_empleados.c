
#include "nomina_empleados.h"

int cargarDatos(const char *nombreArchivo, Empleado *empleado)
{
    FILE *archivo = fopen(nombreArchivo, "r");

    if (!archivo)
    {
        perror("Error al abrir el archivo");
        return 0;
    }

    char linea[MAXLINEA_EMPLEADOS];
    int i = 0;

    while (fgets(linea, sizeof(linea), archivo))
    {
        linea[strcspn(linea, "\n")] = 0;
        if (trozarLinea(linea, &empleado[i]) == 0)
        {
            i++;
        }
        else
        {
            printf("Error en línea %d\n", i + 1);
        }
    }

    fclose(archivo);
    return i;
}

int trozarLinea(char *linea, Empleado *empleado)
{
    char *dato; // Va a contener el dato que vamos a guardar en las varibles

    // Legajo
    dato = strtok(linea, "|");
    if (!dato)
        return ERROR;

    empleado->legajo = atoi(dato);

    // Nombre y Apellido
    dato = strtok(NULL, "|");
    if (!dato)
        return ERROR;

    strcpy(empleado->nombre_y_ap, dato);

    // Fecha
    dato = strtok(NULL, "|");
    if (!dato)
        return ERROR;

    sscanf(dato, "%2d-%2d-%4d",
           &(empleado->fecha_ingreso.d),
           &(empleado->fecha_ingreso.m),
           &(empleado->fecha_ingreso.a));

    // Estado
    dato = strtok(NULL, "|");
    if (!dato)
        return ERROR;

    strcpy(empleado->estado, dato);

    // Categoría
    dato = strtok(NULL, "|");
    if (!dato)
        return ERROR;

    strcpy(empleado->categoria, dato);

    // Sueldo
    dato = strtok(NULL, "|");
    if (!dato)
        return ERROR;

    empleado->sueldo = atof(dato);

    return TODO_OK;
}

void mostrarEmpleados(Empleado *empleados, int cantidad)
{
    printf("%s | %s | %s | %s | %s | %s", "Legajo", "Nombre y Apellido", "Fecha", "Estado", "Categoría", "Sueldo");
    printf("\n--------------------------------------------------------------------------------\n");

    for (int i = 0; i < cantidad; i++)
    {
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

int eliminarEmpleadosInactivos(Empleado *empleados, int *cantidad)
{
    int cantEmpleadosElim = 0;

    for (int i = 0; i < *cantidad; i++)
    {
        // Preguntamos si el empleado esta 'inactivo' (camparamos todo en minusculas)
        if (strcmp(empleados[i].estado, "inactivo") == 0)
        {
            // Aumentamos el contador de empleados eliminados
            cantEmpleadosElim++;
            // No lo eliminamos como tal, sino que movemos los regiustros de adelante, pisando el registro
            for (int j = i; j < *cantidad - 1; j++)
            {
                empleados[j] = empleados[j + 1];
            }
            (*cantidad)--; // Decrementamos la cantidad de empleados del vector
            i--;           // Como movemos los registros de adelante, tenemos que evaluar la misma posicion en donde queda el elemento que estaba adelante antes
        }
    }

    return cantEmpleadosElim;
}

Empleado buscarEmpleadoMasAntiguo(Empleado *empleados, int *cantidad)
{
    Empleado empleadoMasAntiguo;

    for (int i = 0; i < *cantidad - 1; i++)
    {

        if (i == 0)
        {
            empleadoMasAntiguo = empleados[i];
        }
        else
        {
            if (compararFechas(empleadoMasAntiguo.fecha_ingreso, empleados[i].fecha_ingreso) > 0)
                empleadoMasAntiguo = empleados[i];
        }
    }

    return empleadoMasAntiguo;
}

int compararFechas(Fecha f1, Fecha f2)
{
    if (f1.a > f2.a)
        return 1;
    if (f1.a < f2.a)
        return ERROR;

    if (f1.m > f2.m)
        return 1;
    if (f1.m < f2.m)
        return ERROR;

    if (f1.d > f2.d)
        return 1;
    if (f1.d < f2.d)
        return ERROR;

    return 0;
}

int cargarCategoria(Categoria *categoria)
{
    FILE *archivoCategoria = fopen(ARC_CATEGORIA, "r");

    if (!archivoCategoria)
    {
        perror("Error al abrir el archivo");
        return 0;
    }

    char lineaCategoria[MAXLINEA_CATEGORIA];
    int i = 0;

    while (fgets(lineaCategoria, sizeof(lineaCategoria), archivoCategoria))
    {
        lineaCategoria[strcspn(lineaCategoria, "\n")] = 0;

        if (trozarLineaCategoria(lineaCategoria, &categoria[i]) == TODO_OK)
        {
            i++;
        }
        else
        {
            printf("Error en línea %d\n", i + 1);
        }
    }
    fclose(archivoCategoria);

    return TODO_OK;
}

int trozarLineaCategoria(char *linea, Categoria *categoria)
{
    char *dato;

    // CATEGORIA
    dato = strtok(linea, "|");
    if (!dato)
        return ERROR;

    strcpy(categoria->categoria, dato);

    // AUMENTO
    dato = strtok(NULL, "|"); // Para que me de lo que sigue despues del | que es el aumento
    if (!dato)
        return ERROR;

    categoria->aumento = atof(dato); // Convierte string a float

    return TODO_OK;
}

void contarPorCategoria(Empleado *empleados, Categoria *categorias, Resultados *resultados) {
    // Inicializa el contador
    for (int i = 0; i < CANT_CATEGORIA; i++) {
        resultados->cantEmpleadosXCat[i] = 0;
    }

    // Cuenta empleados por categoría
    for (int i = 0; i < resultados->cantEmpleadosActivos; i++) {
        for (int j = 0; j < CANT_CATEGORIA; j++) {
            if (strcmp(empleados[i].categoria, categorias[j].categoria) == 0) {
                resultados->cantEmpleadosXCat[j]++;
            }
        }
    }
}

void actualizarSueldos(Empleado *empleados, int *cantEmp, const Categoria *categorias)
{
    for (int i = 0; i < *cantEmp; i++)
    {
        for (int j = 0; j < CANT_CATEGORIA; j++)
        {
            if (strcmp(empleados[i].categoria, categorias[j].categoria) == 0)
            {
                empleados[i].sueldo *= (1 + categorias[j].aumento / 100.0);
            }
        }
    }
}

void generarArchivosSalida(Empleado *empleados, Resultados *resultados)
{
    generarNominaActualizada(empleados,&resultados->cantEmpleadosActivos);
    generarArchivoResultados(resultados);
}

int generarNominaActualizada(Empleado *empleados, int *cantidadEmpleados){
    FILE *archivo = fopen(ARC_NOMINA_ACT,"wt+");

    if(!archivo){
        perror("Error al crear nomina_actualizada.txt");
        return ERROR;
    }

    for (int i = 0; i < *cantidadEmpleados; i++)
    {
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
    return TODO_OK;
}

int generarArchivoResultados(Resultados *resultados){
    FILE *archivo = fopen(ARC_INFORMACION,"wt+");

    if(!archivo){
        perror("Error al crear nomina_informacion.txt");
        return ERROR;
    }

    fprintf(archivo, "La cantidad de empleados activos es %d\n",resultados->cantEmpleadosActivos);
    fprintf(archivo, "La cantidad de empleados inactivos es %d\n",resultados->cantEmpleadosElim);
    fprintf(archivo, "El empleado mas antiguo es %s que ingreso el %d/%d/%d\n", resultados->empleadoMasAntiguo.nombre_y_ap, resultados->empleadoMasAntiguo.fecha_ingreso.d, resultados->empleadoMasAntiguo.fecha_ingreso.m, resultados->empleadoMasAntiguo.fecha_ingreso.a);
    fprintf(archivo, "La cantidad de empleados en cada categoria es:\nInicial:%d\nIntermedio:%d\nSuperior:%d\nLider:%d\n",resultados->cantEmpleadosXCat[0],resultados->cantEmpleadosXCat[1],resultados->cantEmpleadosXCat[2],resultados->cantEmpleadosXCat[3]);

    fclose(archivo);
    return TODO_OK;
}
