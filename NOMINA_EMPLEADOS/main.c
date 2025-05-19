#include <stdio.h>
#include <stdlib.h>
#include "nomina_empleados.h"



int main()
{
    pid_t hijo1, hijo2, hijo3, hijo4;//variables que se usan para guardar el valor que devuelve el fork( )
    sem_t *semaforo;

    ///1- Abrir archivo nomina.txt y cargar datos a estructura
   Empleado empleados[MAX_EMPLEADOS];
   int cantidad = cargarDatos(ARC_NOMINA, empleados);

    //Para mostrar el vector empleados
   /*if (cantidad > 0) {
        mostrarEmpleados(empleados, cantidad);
    } else {
        printf("No se pudieron cargar empleados.\n");
    }*/


    ///2- Crear semaforo
    sem_unlink(SEM_NAME); //Elimina semaforo anterior si quedo basura
    semaforo = sem_open(SEM_NAME, O_CREAT, 0644, 1); //Crea un semaforo con valor inicial 1, que usaremos para asegurar exclusion mutua(que no se modifiquen los datos al mismo tiempo desde dos procesos).

    if(semaforo == SEM_FAILED){
        perror("Error al crear el semaforo");
        exit(EXIT_FAILURE);
    }

    ///3- Crear proceso hijo 1 (Elimina empelados inactivos)
    hijo1 = fork(); //Hijo 1 usa semaforo porque modifica los datos
    if(hijo1 == 0){
        puts("Hijo 1: eliminando empleados inactivos---\n");
        sem_wait(semaforo); //Bloquea acceso a datos compartidos
    //Codigo para eliminar inactivos
    sem_post(semaforo); //Libera acceso
    exit(0);
    }

    ///4- Crear proceso hijo 2 (buscar mas antiguo)
    hijo2 = fork();
    if(hijo2 == 0){
        puts("Hijo 2: buscando empleado mas antiguo...\n");
    //Codigo para buscar al empleado mas antiguo
    exit(0);
    }

    ///5- Crear hijo 3 (Contar empleados por categoria)
    hijo3 = fork();
    if(hijo3 == 0){
        puts("Hijo 3: contando empleados por categoria...\n");
    //Codigo para contar por categoria
    exit(0);
    }

    ///6- Crear proceso hijo 4 (Actualizar sueldos)
    hijo4 = fork();
    if(hijo4 == 0){
        puts("Hijo 4: actualizando sueldos por categoria...\n");
        sem_wait(semaforo);
    //Codigo para actualizar sueldos
    sem_post(semaforo);
    exit(0);
    }

    ///7- Esperar a los 3 hijos restantes
    waitpid(hijo1, NULL, 0);
    waitpid(hijo2,NULL, 0);
    waitpid(hijo3, NULL, 0);
    waitpid(hijo4, NULL, 0);

    ///8- Guardar resultados finales en archivos
    puts("Padre: generando archivos de salida (nomina, resumen)...\n");

    ///9- Cerrar y destruir semaforo
    sem_close(semaforo);
    sem_unlink(SEM_NAME);

    puts("Padre: todo finalizando.\n");

    //falta agregar la parte de archivos y primitivas

    return 0;
}



