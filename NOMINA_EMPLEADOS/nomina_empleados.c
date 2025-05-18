#include "nomina_Empleados.h"

int cargarDatos(const char *nombreArchivo, Empleado *empleado, int MAX_EMPLEADOS) {
    
    FILE *archivo = fopen(nombreArchivo, "r");
    
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 0;
    }

    char linea[MAXLINEA];
    int i = 0;

    while (fgets(linea, sizeof(linea), archivo)) {
        // Eliminar salto de línea
        linea[strcspn(linea, "\n")] = 0;

       trozarLinea(linea, )
        i++;
    }

    fclose(archivo);
    return i; // cantidad de personas cargadas
}


int trozarLinea(char* linea, Empleado* Empleado){
    char* aux;
    aux = strrchr(linea,'|');
    sscanf(aux+1,"%f",&(Empleado->sueldo));
    *aux= '\0';
    aux= strrchr(linea,'|');
    strcpy(Empleado->categoria,aux+1);
    *aux='\0';
    aux= strrchr(linea,'|');
    strcpy(Empleado->estado,aux+1);
    aux= strrchr(linea,'-');
    sscanf(aux+1,"%d",&(Empleado->fecha_ingreso.a));
    *aux='\0';
    aux= strrchr(linea,'-');
    sscanf(aux+1,"%d",&(Empleado->fecha_ingreso.m));
    *aux='\0';
    aux= strrchr(linea,'|');
    sscanf(aux+1,"%d",&(Empleado->fecha_ingreso.d));
    *aux='\0';
    aux= strrchr(linea,'|');
    strcpy(Empleado->nombre_y_ap,aux+1);
    *aux='\0';
    sscanf(linea,"%d",&(Empleado->legajo));
    return 0;
}