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
    pid_t hijo1, hijo2, hijo3, hijo4;
    sem_t *semMC_NuevaNomina, *semMC_Resultados;

    /// 1- Crear semaforo
    // Elimino semaforos anteriorres si quedaron basura
    sem_unlink(SEM_MC_NUEVA_NOMINA);
    sem_unlink(SEM_MC_NUEVA_NOMINA);

    // Crea semaforos con valor inicial 1, que usaremos para asegurar exclusion mutua(que no se modifiquen los datos al mismo tiempo desde dos procesos).
    semMC_NuevaNomina = sem_open(SEM_MC_NUEVA_NOMINA, O_CREAT, 0644, 1);
    semMC_Resultados = sem_open(SEM_MC_RESULTADOS, O_CREAT, 0644, 1);

    if (semMC_NuevaNomina == SEM_FAILED || semMC_Resultados == SEM_FAILED)
    {
        perror("--- Error al crear el semaforo ---");
        exit(EXIT_FAILURE);
    }

    // 2.0 - Preparo la variable 'empleados' para trabajar como Mem.Copartida
    // Se crea un objeto de memoria compartida POSIX, identificado por un nombre (debe empezar con '/' porque va a estar dentro de la misma carpeta del proyecto)
    int IdMCNuevaNomina = shm_open(MC_NUEVA_NOMINA, O_CREAT | O_RDWR, 0666);
    if (IdMCNuevaNomina == -1)
    {
        perror("--- FFallo al crear la Memoria Compartida --- "); // Tira el msj como del tipo error
        exit(1);
    }

    // Esta funcion asigna un tamaño a la memoria compartida para la cantidad de empleados
    ftruncate(IdMCNuevaNomina, sizeof(Empleado) * MAX_EMPLEADOS);

    // Hace que el proceso puede usar el arreglo 'empleados' como si fuera una variable normal, pero en realidad está accediendo a memoria compartida.
    Empleado *empleados = mmap(NULL, sizeof(Empleado) * MAX_EMPLEADOS, PROT_READ | PROT_WRITE, MAP_SHARED, IdMCNuevaNomina, 0);
    if (empleados == MAP_FAILED)
    {
        perror("--- Error al ejecutar en la funcion 'mmap' ---");
        exit(1);
    }

    // 2.1 - Preparo la MC para los resultados de los demas procesos hijos
    // Como los hijos van calcular y obtener datos, para que eln pradre u otro proceso pueda ver y trabajar esos datos, tienen que
    // estar en MC, por eso creamos una especifica para los resultados
    int IdMCResultados = shm_open(MC_RESULTADOS, O_CREAT | O_RDWR, 0666);
    if (IdMCResultados == -1)
    {
        perror("--- Fallo al crear la Memoria Compartida ---");
        exit(1);
    }

    ftruncate(IdMCResultados, sizeof(Resultados));

    Resultados *resultadosDeHijos = mmap(NULL, sizeof(Resultados), PROT_READ | PROT_WRITE, MAP_SHARED, IdMCResultados, 0);
    if (resultadosDeHijos == MAP_FAILED)
    {
        perror("--- Error al ejecutar en la funcion 'mmap' ---");
        exit(1);
    }

    // 2.2 -  Cargamos los datos de categoria.txt a la variable 'categoriasYAumentos' para que pueda ser usado en el hijo 3 y 4
    Categoria categoriasYAumentos[CANT_CATEGORIA];
    cargarCategoria(categoriasYAumentos);
    
    // 2.3 - Abrir archivo nomina.txt y cargar datos a estructura
    //Usamos como contador temporal la variable cantEmpleadosActivos de la estructura resultados
    sem_wait(semMC_Resultados);
    resultadosDeHijos->cantEmpleadosActivos = cargarDatos(ARC_NOMINA, empleados);  
    sem_post(semMC_Resultados);

    /// 3- Crear proceso hijo 1 (Elimina empelados inactivos)
    hijo1 = fork(); // Hijo 1 usa semaforo porque modifica los datos
    if (hijo1 == 0)
    {
        puts("--- Eliminando empleados inactivos ---");
        // Bloquea acceso a datos compartidos
        sem_wait(semMC_NuevaNomina);
        sem_wait(semMC_Resultados);

        // Codigo para eliminar inactivos
        resultadosDeHijos->cantEmpleadosElim = eliminarEmpleadosInactivos(empleados, &resultadosDeHijos->cantEmpleadosActivos);

        // Libera accesos
        sem_post(semMC_NuevaNomina);
        sem_post(semMC_Resultados);
        exit(0);
    }    

    /// 4- Crear proceso hijo 2 (buscar mas antiguo)
    hijo2 = fork();
    if (hijo2 == 0)
    {
        puts("-- Buscando al empleado mas antiguo ---");
        sem_wait(semMC_Resultados);

        // Codigo para buscar al empleado mas antiguo
        resultadosDeHijos->empleadoMasAntiguo = buscarEmpleadoMasAntiguo(empleados, &resultadosDeHijos->cantEmpleadosActivos);

        sem_post(semMC_Resultados);
        exit(0);
    }

    /// 5- Crear hijo 3 (Contar empleados por categoria)
    hijo3 = fork();
    if (hijo3 == 0)
    {
        puts("-- Contando empleados en cada categoria ---");
        sem_wait(semMC_Resultados);

        // Codigo para contar por categoria
        contarPorCategoria(empleados,categoriasYAumentos,resultadosDeHijos);

        sem_post(semMC_Resultados);
        exit(0);
    }

    /// 6- Crear proceso hijo 4 (Actualizar sueldos)
    hijo4 = fork();
    if (hijo4 == 0)
    {
        puts("-- Actualizando sueldos ---");
        sem_wait(semMC_NuevaNomina);

        // Codigo para actualizar sueldos por categoria
        actualizarSueldos(empleados, &resultadosDeHijos->cantEmpleadosActivos, categoriasYAumentos);

        sem_post(semMC_NuevaNomina);
        exit(0);
    }

    /// 7- Esperar a los 3 hijos restantes
    waitpid(hijo1, NULL, 0);
    waitpid(hijo2, NULL, 0);
    waitpid(hijo3, NULL, 0);
    waitpid(hijo4, NULL, 0);

    /// 8- Guardar resultados finales en archivos
    puts("--- Generando Archivos de Salida ---");
    generarArchivosSalida(empleados, resultadosDeHijos);

    /// 9- Cerrar y destruir semaforos
    sem_close(semMC_NuevaNomina);
    sem_close(semMC_Resultados);
    sem_unlink(SEM_MC_NUEVA_NOMINA);
    sem_unlink(SEM_MC_RESULTADOS);

    // Liberar Mem.Comp
    // Desmapear
    munmap(empleados, sizeof(Empleado) * MAX_EMPLEADOS);
    munmap(resultadosDeHijos, sizeof(Resultados));

    int seEliminaMCNuevaNomina = shm_unlink(MC_NUEVA_NOMINA);
    int seEliminaMCResutlados = shm_unlink(MC_RESULTADOS);

    if (seEliminaMCNuevaNomina == -1 || seEliminaMCResutlados == -1)
    {
        if (seEliminaMCNuevaNomina == -1)
        {
            perror("--- Error al eliminar la memoria compartida de la nueva nomina ---");
        }
        else
        {
            perror("--- Error al eliminar la memoria compartida de resultados ---");
        }
    }
    else
    {
        puts("--- Memoria compartida eliminada correctamente ---");
    }

    puts("--- Proceso finalizo ---");

    return TODO_OK;
}