#ifndef NOMINA_EMPLEADOS_H_INCLUDED
#define NOMINA_EMPLEADOS_H_INCLUDED

///////////////ARCHIVOS DE ENTRADA//////////////
#define ARC_NOMINA "nomina.txt"
#define ARC_CATEGORIA "categoria.txt"

///////////////ARCHIVOS DE SALIDA///////////////
#define ARC_NOMINA_ACT "nomina_actualizada.txt"
#define ARC_INFORMACION "nomina_informacion.txt"
#define ARC_NOMINA_ANT "nominaAntigua.txt"

/////////////////TAMAÑOS////////////////////////
#define TAM_NOMYAP 30
#define TAM_ESTADO 20
#define TAM_CATEGORIA 20


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


#endif // NOMINA_EMPLEADOS_H_INCLUDED
