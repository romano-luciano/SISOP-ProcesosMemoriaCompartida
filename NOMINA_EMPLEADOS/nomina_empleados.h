#ifndef NOMINA_EMPLEADOS_H_INCLUDED
#define NOMINA_EMPLEADOS_H_INCLUDED
#include <unistd.h>       //para trabajar con fork()
#include <sys/wait.h>     //para waitpid
#include <semaphore.h>
#include <fcntl.h>        //para constantes de semáforos
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///////////////ARCHIVOS DE ENTRADA//////////////
#define ARC_NOMINA "../Archivos de Entrada/nomina.txt"
#define ARC_CATEGORIA "../Archivos de Entrada/categoria.txt"

///////////////ARCHIVOS DE SALIDA///////////////
#define ARC_NOMINA_ACT "../Archivos de Salida/nomina.txt"
#define ARC_INFORMACION "../Archivos de Salida/nomina_informacion.txt"
#define ARC_NOMINA_ANT "../Archivos de Salida/nomina_antigua.txt"

/////////////////TAMAÑOS////////////////////////
#define TAM_NOMYAP 30
#define TAM_ESTADO 20
#define TAM_CATEGORIA 20
#define MAXLINEA_EMPLEADOS 256
#define MAXLINEA_CATEGORIA 40
#define MAX_EMPLEADOS 100
#define CANT_CATEGORIA 4
/////////////////VALORES////////////////////////
#define TODO_OK 0
#define ERROR -1
#define ESPERANDO_ESTADO 1
#define TERMINEN_ESTADO 0

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
    int cantEmpleadosActivos;
    int cantEmpleadosElim;
    Empleado empleadoMasAntiguo;
    int cantEmpleadosXCat[CANT_CATEGORIA];//Cada posicion representa una categoria [Inicial,Intermedio,Superior,Lider]
}Resultados;

int trozarLinea(char* linea, Empleado* empleado);
int cargarDatos(const char *nombreArchivo, Empleado *empleado) ;
void mostrarEmpleados(Empleado *empleados, int cantidad);
int compararFechas(Fecha f1, Fecha f2);
int cargarCategoria(Categoria *categoria);
int trozarLineaCategoria(char *linea, Categoria *categoria);

int eliminarEmpleadosInactivos(Empleado *empleados,int *cantidad);
Empleado buscarEmpleadoMasAntiguo(Empleado *empleados,int *cantidad);
void actualizarSueldos(Empleado *empleados, int *cantEmp,const Categoria *categorias);
void generarArchivosSalida(Empleado *empleados, Resultados *resultados);
int generarNominaActualizada(Empleado *empleados, int *cantidadEmpleados);
int generarArchivoResultados(Resultados *resultados);
void contarPorCategoria(Empleado *empleados, Categoria *categorias, Resultados *resultados);


#endif
