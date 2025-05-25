#include "nomina_empleados.h"

// Estos cuatro include son necesarios para la memoria compartida
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define MC_NUEVA_NOMINA "/mi_mc_nueva_nomina"
#define MC_RESULTADOS "/mi_mc_resultados"
#define SEM_MC_NUEVA_NOMINA "/semaforo_nuevaNomina"
#define SEM_MC_RESULTADOS "/semaforo_resultados"

int main()
{
    pid_t hijo1, hijo2, hijo3, hijo4; // variables que se usan para guardar el valor que devuelve el fork( )
    sem_t *semMC_NuevaNomina,*semMC_Resultados;

    /// 1- Crear semaforo
    // Elimino semaforos anteriorres si quedaron basura
    sem_unlink(SEM_MC_NUEVA_NOMINA);
    sem_unlink(SEM_MC_NUEVA_NOMINA);

    // Crea semaforos con valor inicial 1, que usaremos para asegurar exclusion mutua(que no se modifiquen los datos al mismo tiempo desde dos procesos).
    semMC_NuevaNomina = sem_open(SEM_MC_NUEVA_NOMINA, O_CREAT, 0644, 1);
    semMC_Resultados = sem_open(SEM_MC_RESULTADOS, O_CREAT, 0644, 1);

    if (semMC_NuevaNomina == SEM_FAILED || semMC_Resultados == SEM_FAILED)
    {
        perror("Error al crear el semaforo");
        exit(EXIT_FAILURE);
    }

    // 2.0 - Preparo la variable 'empleados' para trabajar como Mem.Copartida
    // Se crea un objeto de memoria compartida POSIX, identificado por un nombre (debe empezar con '/' porque va a estar dentro de la misma carpeta del proyecto)
    int IdMCNuevaNomina = shm_open(MC_NUEVA_NOMINA, O_CREAT | O_RDWR, 0666);
    if (IdMCNuevaNomina == -1)
    {
        perror("Fallo al crear la Memoria Compartida"); // Tira el msj como del tipo error
        exit(1);
    }

    // Esta funcion asigna un tamaño a la memoria compartida para la cantidad de empleados
    ftruncate(IdMCNuevaNomina, sizeof(Empleado) * MAX_EMPLEADOS);

    // Hace que el proceso puede usar el arreglo 'empleados' como si fuera una variable normal, pero en realidad está accediendo a memoria compartida.
    Empleado *empleados = mmap(NULL, sizeof(Empleado) * MAX_EMPLEADOS, PROT_READ | PROT_WRITE, MAP_SHARED, IdMCNuevaNomina, 0);
    if (empleados == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // 2.1 - Abrir archivo nomina.txt y cargar datos a estructura
    // La variable 'cantidad' no es Mem.Comp
    int cantidad = cargarDatos(ARC_NOMINA, empleados);

    // Para mostrar el vector empleados
    /*if (cantidad > 0)
    {
        mostrarEmpleados(empleados, cantidad);
    }
    else
    {
        printf("No se pudieron cargar empleados.\n");
    }*/

    // 2.3 - Preparo la MC para los resultados de los demas procesos hijos
    //Como los hijos van calcular y obtener datos, para que eln pradre u otro proceso pueda ver y trabajar esos datos, tienen que
    //estar en MC, por eso creamos una especifica para los resultados
    int IdMCResultados = shm_open(MC_RESULTADOS, O_CREAT | O_RDWR, 0666);
    if (IdMCResultados == -1)
    {
        perror("Fallo al crear la Memoria Compartida");
        exit(1);
    }

    ftruncate(IdMCResultados, sizeof(Resultados));

    Resultados *resultadosDeHijos = mmap(NULL, sizeof(Resultados), PROT_READ | PROT_WRITE, MAP_SHARED, IdMCResultados, 0);
    if (resultadosDeHijos == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    /// 3- Crear proceso hijo 1 (Elimina empelados inactivos)
    hijo1 = fork(); // Hijo 1 usa semaforo porque modifica los datos
    if (hijo1 == 0)
    {
        puts("--- Eliminando empleados inactivos ---\n");
        // Bloquea acceso a datos compartidos
        sem_wait(semMC_NuevaNomina);
        sem_wait(semMC_Resultados);

        // Codigo para eliminar inactivos
        resultadosDeHijos->cantEmpleadosElim = eliminarEmpleadosInactivos(empleados,&cantidad);

        // Libera accesos
        sem_post(semMC_NuevaNomina);
        sem_post(semMC_Resultados);
        exit(0);
    }

    //Espero a que termine el hijo 1
    waitpid(hijo1, NULL, 0);
    printf("Se eliminaron %d empelados\n\n",resultadosDeHijos->cantEmpleadosElim);

    /// 4- Crear proceso hijo 2 (buscar mas antiguo)
    hijo2 = fork();
    if (hijo2 == 0)
    {
        puts("-- Buscando al empleado mas antiguo ---\n");
        sem_wait(semMC_Resultados);

        // Codigo para buscar al empleado mas antiguo
        resultadosDeHijos->empleadoMasAntiguo = buscarEmpleadoMasAntiguo(empleados,&cantidad);

        sem_post(semMC_Resultados);
        exit(0);
    }

    waitpid(hijo2, NULL, 0);
    printf("El empleado mas antiguo es %s que ingreso el %d/%d/%d\n\n",resultadosDeHijos->empleadoMasAntiguo.nombre_y_ap,resultadosDeHijos->empleadoMasAntiguo.fecha_ingreso.d,resultadosDeHijos->empleadoMasAntiguo.fecha_ingreso.m,resultadosDeHijos->empleadoMasAntiguo.fecha_ingreso.a);

    /// 5- Crear hijo 3 (Contar empleados por categoria)
    hijo3 = fork();
    if (hijo3 == 0)
    {
        puts("Hijo 3: contando empleados por categoria...\n");
        // Codigo para contar por categoria

        // Contar empleados por categoría y guardar en memoria compartida
        contarPorCategoria(empleados, cantidad, "Aca va la categoria puntero", resultadosDeHijos->cantEmpleadosXCat);
        
        exit(0);
    }

    /// 6- Crear proceso hijo 4 (Actualizar sueldos)
    hijo4 = fork();
    if (hijo4 == 0)
    {
        puts("Hijo 4: actualizando sueldos por categoria...\n");
        sem_wait(semMC_NuevaNomina);
        // Codigo para actualizar sueldos
        sem_post(semMC_NuevaNomina);
        exit(0);
    }

    /// 7- Esperar a los 3 hijos restantes
    waitpid(hijo2, NULL, 0);
    waitpid(hijo3, NULL, 0);
    waitpid(hijo4, NULL, 0);

    /// 8- Guardar resultados finales en archivos
    puts("Padre: generando archivos de salida (nomina, resumen)...\n");

    /// 9- Cerrar y destruir semaforos
    sem_close(semMC_NuevaNomina);
    sem_close(semMC_Resultados);
    sem_unlink(SEM_MC_NUEVA_NOMINA);
    sem_unlink(SEM_MC_RESULTADOS);

    puts("Padre: todo finalizando.\n");

    // falta agregar la parte de archivos y primitivas

    // Liberar Mem.Comp
    // Desmapear
    munmap(empleados, sizeof(Empleado) * MAX_EMPLEADOS);
    munmap(resultadosDeHijos, sizeof(Resultados));

    int seEliminaMCNuevaNomina = shm_unlink(MC_NUEVA_NOMINA);
    int seEliminaMCResutlados = shm_unlink(MC_RESULTADOS);

    if (seEliminaMCNuevaNomina == -1 || seEliminaMCResutlados == -1)
    {
        if(seEliminaMCNuevaNomina == -1){
            perror("Error al eliminar la memoria compartida de la nueva nomina");
        }else{
            perror("Error al eliminar la memoria compartida de resultados");
        }
    }
    else
    {
        printf("Memoria compartida eliminada correctamente.\n");
    }

    return 0;
}
