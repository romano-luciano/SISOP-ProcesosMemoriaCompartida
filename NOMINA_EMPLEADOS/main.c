#include "nomina_empleados.h"
// Estos cuatro include son necesarios para la memoria compartida
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

//Manejo de señales
#include <signal.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#define MC_NUEVA_NOMINA "/mi_mc_nueva_nomina"
#define MC_RESULTADOS "/mi_mc_resultados"
#define MC_TERMINAL "/mi_mc_terminal"
#define SEM_MC_NUEVA_NOMINA "/semaforo_nuevaNomina"
#define SEM_MC_RESULTADOS "/semaforo_resultados"
#define SEM_MC_TERMINAL "/semaforo_terminal"

//Lo definis como variables globales 
Empleado *empleados = NULL;
Resultados *resultadosDeHijos = NULL;
int *estadoTerminal = NULL;
sem_t *semMC_NuevaNomina = NULL, *semMC_Resultados = NULL, *semMC_Terminal = NULL;
pid_t hijo1 = -1, hijo2 = -1, hijo3 = -1, hijo4 = -1;

// Estas funciones se ejecuta si el padre recibe una senal como Ctrl+C o kill
void manejador_senial(int sig);
void manejador_senial_hijo(int sig);
void liberar_recursos();

int main()
{
    signal(SIGINT, manejador_senial);
    signal(SIGTERM, manejador_senial);

    /// 1- Crear semaforo
    // Elimino semaforos anteriorres si quedaron basura
    sem_unlink(SEM_MC_NUEVA_NOMINA);
    sem_unlink(SEM_MC_RESULTADOS);
    sem_unlink(SEM_MC_TERMINAL);

    // Crea semaforos con valor inicial 1, que usaremos para asegurar exclusion mutua(que no se modifiquen los datos al mismo tiempo desde dos procesos).
    semMC_NuevaNomina = sem_open(SEM_MC_NUEVA_NOMINA, O_CREAT, 0644, 1);
    semMC_Resultados = sem_open(SEM_MC_RESULTADOS, O_CREAT, 0644, 1);
    semMC_Terminal = sem_open(SEM_MC_TERMINAL, O_CREAT, 0644, 1);

    if (semMC_NuevaNomina == SEM_FAILED || semMC_Resultados == SEM_FAILED || semMC_Terminal == SEM_FAILED) 
    {
        perror("--  Error al crear los semaforo");
        exit(EXIT_FAILURE);
    }

    // 2.0 - Preparo la variable 'empleados' para trabajar como Mem.Copartida
    // Se crea un objeto de memoria compartida POSIX, identificado por un nombre (debe empezar con '/' porque va a estar dentro de la misma carpeta del proyecto)
    int IdMCNuevaNomina = shm_open(MC_NUEVA_NOMINA, O_CREAT | O_RDWR, 0666);
    if (IdMCNuevaNomina == -1)
    {
        perror("--  Fallo al crear la Memoria Compartida para 'empleados'"); // Tira el msj como del tipo error
        exit(1);
    }

    // Esta funcion asigna un tamaño a la memoria compartida para la cantidad de empleados
    ftruncate(IdMCNuevaNomina, sizeof(Empleado) * MAX_EMPLEADOS);

    // Hace que el proceso puede usar el arreglo 'empleados' como si fuera una variable normal, pero en realidad está accediendo a memoria compartida.
    empleados = mmap(NULL, sizeof(Empleado) * MAX_EMPLEADOS, PROT_READ | PROT_WRITE, MAP_SHARED, IdMCNuevaNomina, 0);
    if (empleados == MAP_FAILED)
    {
        perror("--  Error al ejecutar en la funcion 'mmap'  para 'empleados'");
        exit(1);
    }

    // 2.1 - Preparo la MC para los resultados de los demas procesos hijos
    // Como los hijos van calcular y obtener datos, para que eln pradre u otro proceso pueda ver y trabajar esos datos, tienen que
    // estar en MC, por eso creamos una especifica para los resultados
    int IdMCResultados = shm_open(MC_RESULTADOS, O_CREAT | O_RDWR, 0666);
    if (IdMCResultados == -1)
    {
        perror("--  Fallo al crear la Memoria Compartida para 'resultadosHijos'");
        exit(1);
    }

    ftruncate(IdMCResultados, sizeof(Resultados));

    resultadosDeHijos = mmap(NULL, sizeof(Resultados), PROT_READ | PROT_WRITE, MAP_SHARED, IdMCResultados, 0);
    if (resultadosDeHijos == MAP_FAILED)
    {
        perror("--  Error al ejecutar en la funcion 'mmap' para 'resultadosHijos'");
        exit(1);
    }

    // 2.2 - Preparo la MC para controlar la terminal
    int IdMCTerminal = shm_open(MC_TERMINAL, O_CREAT | O_RDWR, 0666);
    if (IdMCTerminal == -1)
    {
        perror("--  Fallo al crear la Memoria Compartida para 'estadoTerminal'");
        exit(1);
    }

    ftruncate(IdMCTerminal, sizeof(int));

    estadoTerminal = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, IdMCTerminal, 0);
    if (estadoTerminal == MAP_FAILED)
    {
        perror("--  Error al ejecutar en la funcion 'mmap' para 'estadoTerminal'");
        exit(1);
    }

    sem_wait(semMC_Terminal);
    *estadoTerminal=ESPERANDO_ESTADO;
    sem_post(semMC_Terminal);

    // 2.3 -  Cargamos los datos de categoria.txt a la variable 'categoriasYAumentos' para que pueda ser usado en el hijo 3 y 4
    Categoria categoriasYAumentos[CANT_CATEGORIA];
    cargarCategoria(categoriasYAumentos);
    
    // 2.4 - Abrir archivo nomina.txt y cargar datos a estructura
    //Usamos como contador temporal la variable cantEmpleadosActivos de la estructura resultados
    sem_wait(semMC_Resultados);
    resultadosDeHijos->cantEmpleadosActivos = cargarDatos(ARC_NOMINA, empleados);  
    sem_post(semMC_Resultados);

    puts("--    Gestion de nomina en proceso");

    /// 3- Crear proceso hijo 1 (Elimina empelados inactivos)
    hijo1 = fork(); // Hijo 1 usa semaforo porque modifica los datos
    if (hijo1 == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        signal(SIGINT, manejador_senial_hijo);
        signal(SIGTERM, manejador_senial_hijo);
        
        // Bloquea acceso a datos compartidos
        sem_wait(semMC_NuevaNomina);
        sem_wait(semMC_Resultados);

        // Codigo para eliminar inactivos
        resultadosDeHijos->cantEmpleadosElim = eliminarEmpleadosInactivos(empleados, &resultadosDeHijos->cantEmpleadosActivos);

        // Libera accesos
        sem_post(semMC_NuevaNomina);
        sem_post(semMC_Resultados);
        
        //Una vez realizado el trabajo, el hijo espera al padre para terminar
        while (*estadoTerminal) {
            sleep(1);
        }
        puts("--    Empleados inactivos eliminados");
        exit(0);
    }    

    /// 4- Crear proceso hijo 2 (buscar mas antiguo)
    hijo2 = fork();
    if (hijo2 == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        signal(SIGINT, manejador_senial_hijo);
        signal(SIGTERM, manejador_senial_hijo);

        sem_wait(semMC_Resultados);

        // Codigo para buscar al empleado mas antiguo
        resultadosDeHijos->empleadoMasAntiguo = buscarEmpleadoMasAntiguo(empleados, &resultadosDeHijos->cantEmpleadosActivos);

        sem_post(semMC_Resultados);

        while (*estadoTerminal) {
            sleep(1);
        }
        puts("--    Empleado mas antiguo encontrado");
        exit(0);
    }

    /// 5- Crear hijo 3 (Contar empleados por categoria)
    hijo3 = fork();
    if (hijo3 == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        signal(SIGINT, manejador_senial_hijo);
        signal(SIGTERM, manejador_senial_hijo);

        sem_wait(semMC_Resultados);

        // Codigo para contar por categoria
        contarPorCategoria(empleados,categoriasYAumentos,resultadosDeHijos);

        sem_post(semMC_Resultados);
        
        while (*estadoTerminal) {
            sleep(1);
        }
        puts("--    Empleados en cada categoria contados");
        exit(0);
    }

    /// 6- Crear proceso hijo 4 (Actualizar sueldos)
    hijo4 = fork();
    if (hijo4 == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        signal(SIGINT, manejador_senial_hijo);
        signal(SIGTERM, manejador_senial_hijo);

        sem_wait(semMC_NuevaNomina);

        // Codigo para actualizar sueldos por categoria
        actualizarSueldos(empleados, &resultadosDeHijos->cantEmpleadosActivos, categoriasYAumentos);

        sem_post(semMC_NuevaNomina);
        
        while (*estadoTerminal) {
            sleep(1);
        }
        puts("--    Sueldos actualizados");
        exit(0);
    }

    /// 7.1 - Pregunto al user si quiere terminar
    char respuesta[10];

    while (1) {
        printf("\n¿Deseás finalizar el programa? (si / no): ");
        fgets(respuesta, sizeof(respuesta), stdin);
        respuesta[strcspn(respuesta, "\n")] = 0; // Quitar salto de línea

        if (strcasecmp(respuesta, "si") == 0) {
            sem_wait(semMC_Terminal);
            *estadoTerminal=TERMINEN_ESTADO;
            sem_post(semMC_Terminal);
            break;
        } 
    }
    printf("\n");
    

    /// 7.2 - Esperar a los 3 hijos restantes
    waitpid(hijo1, NULL, 0);
    waitpid(hijo2, NULL, 0);
    waitpid(hijo3, NULL, 0);
    waitpid(hijo4, NULL, 0);

    /// 8- Guardar resultados finales en archivos
    puts("\n--    Archivos de Salida Generados");
    generarArchivosSalida(empleados, resultadosDeHijos);

    /// 9- Cerrar y destruir semaforos y liberar recursos compartidos
    liberar_recursos();

    puts("--    Proceso finalizado");

    return TODO_OK;
}

void manejador_senial(int sig) {
    printf("\n--    Señal recibida. Terminando hijos y liberando recursos...\n");
    if (hijo1 > 0) 
        kill(hijo1, SIGTERM);

    if (hijo2 > 0) 
        kill(hijo2, SIGTERM);

    if (hijo3 > 0) 
        kill(hijo3, SIGTERM);

    if (hijo4 > 0) 
        kill(hijo4, SIGTERM);

    liberar_recursos();

    exit(EXIT_FAILURE);
}

void manejador_senial_hijo(int sig) {
    printf("--  Hijo %d recibio señal %d. Liberando recursos...\n", getpid(), sig);

    if (empleados) 
        munmap(empleados, sizeof(Empleado) * MAX_EMPLEADOS);

    if (resultadosDeHijos) 
        munmap(resultadosDeHijos, sizeof(Resultados));

    if (estadoTerminal) 
        munmap(estadoTerminal, sizeof(int));

    if (semMC_NuevaNomina) 
        	sem_close(semMC_NuevaNomina);

    if (semMC_Resultados) 
        sem_close(semMC_Resultados);

    if (semMC_Terminal) 
        sem_close(semMC_Terminal);

    exit(EXIT_FAILURE);
}

void liberar_recursos() {
    if (semMC_NuevaNomina) 
        sem_close(semMC_NuevaNomina);

    if (semMC_Resultados) 
        sem_close(semMC_Resultados);

    if (semMC_Terminal) 
        sem_close(semMC_Terminal);
    
    sem_unlink(SEM_MC_NUEVA_NOMINA);
    sem_unlink(SEM_MC_RESULTADOS);
    sem_unlink(SEM_MC_TERMINAL);

    if (empleados) 
        munmap(empleados, sizeof(Empleado) * MAX_EMPLEADOS);

    if (resultadosDeHijos) 
        munmap(resultadosDeHijos, sizeof(Resultados));

    if (estadoTerminal) 
        munmap(estadoTerminal, sizeof(int));

    int seEliminaMCNuevaNomina = shm_unlink(MC_NUEVA_NOMINA);
    int seEliminaMCResutlados = shm_unlink(MC_RESULTADOS);
    int seEliminaMCTerminal = shm_unlink(MC_TERMINAL);

    if (seEliminaMCNuevaNomina == -1 || seEliminaMCResutlados == -1 || seEliminaMCTerminal == -1)
    {
        if (seEliminaMCNuevaNomina == -1)
        {
            perror("--  Error al eliminar la memoria compartida de la nueva nomina");
        }
        else
        {
            if(seEliminaMCResutlados == -1){
                perror("--  Error al eliminar la memoria compartida de resultados");
            }
            else
            {
                perror("--  Error al eliminar la memoria compartida de la Terminal");
            }
        }
    }
    else
    {
        puts("--    Memoria compartida eliminada correctamente");
    }
}