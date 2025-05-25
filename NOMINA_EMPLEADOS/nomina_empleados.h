#ifndef NOMINA_EMPLEADOS_H_INCLUDED
#define NOMINA_EMPLEADOS_H_INCLUDED
#include <unistd.h>       //para trabajar con fork()
#include <sys/wait.h>     //para waitpid
#include <semaphore.h>
#include <fcntl.h>        //para constantes de semáforos
#define SEM_NAME "/semaforo_nomina" //el nombre del semaforo debe empezar con "/"

///////////////ARCHIVOS DE ENTRADA//////////////
#define ARC_NOMINA "../nomina.txt"
#define ARC_CATEGORIA "../categoria.txt"

///////////////ARCHIVOS DE SALIDA///////////////
#define ARC_NOMINA_ACT "nomina_actualizada.txt"
#define ARC_INFORMACION "nomina_informacion.txt"
#define ARC_NOMINA_ANT "nominaAntigua.txt"

/////////////////TAMAÑOS////////////////////////
#define TAM_NOMYAP 30
#define TAM_ESTADO 20
#define TAM_CATEGORIA 20
#define MAXLINEA 256
#define TODO_OK 0
#define FALLO_ELIMINAR_DUPLICADOS 1
#define MAX_EMPLEADOS 100
#define MAX_CATEGORIAS 50 
#define CANT_CATEGORIA 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct{
    int d;
    int m;
    int a;
}Fecha;

typedef struct{
    int legajo;
    char nombre_y_ap[TAM_NOMYAP];
    Fecha fecha_ingreso;
    char estado[TAM_ESTADO];
    char categoria[TAM_CATEGORIA];
    float sueldo;

}Empleado;

typedef struct{
    char categoria[TAM_CATEGORIA];
    float aumento;

}Categoria;

typedef struct{
    int cantEmpleadosElim;
    Empleado empleadoMasAntiguo;
    int cantEmpleadosXCat[CANT_CATEGORIA];
}Resultados;

int trozarLinea(char* linea, Empleado* empleado);
int cargarDatos(const char *nombreArchivo, Empleado *empleado) ;
void mostrarEmpleados(Empleado *empleados, int cantidad);
int compararFechas(Fecha f1, Fecha f2);

// nomina_empleados.h

int eliminarEmpleadosInactivos(Empleado *empleados,int *cantidad);
Empleado buscarEmpleadoMasAntiguo(Empleado *empleados,int *cantidad);

void contarPorCategoria(Empleado *empleados, int cantidad, Categoria *categorias, int *cantEmpleadosXCat) ;


/*
void actualizarSueldos();
*/

#endif // NOMINA_EMPLEADOS_H_INCLUDED
