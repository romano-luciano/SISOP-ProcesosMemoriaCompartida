# SISOP-ProcesosMemoriaCompartida
Repositorio para desarrollar el ejercicio 1 del trabajo practico. Coordinación de Tareas con Procesos y Memoria Compartida  
Proyecto desarrollado en lenguaje C 

EJECUCION
-------------------------------------------------------------------------
Para la ejecucion del Proceso, lo primero que se debe hacer es ubicarse en la carpea 'SISOP-ProcesosMemoriaCompartida/NOMINA_EMPLEADOS'.
Despues para la compilacion "gcc main.c nomina_empleados.c -o nomina". Y se ejecuta con <./nomina>

Se trabajara con los archivos "nomina.txt" y "categorias.txt" ubicados en la carpeta 'Archivos de Entrada' del proyecto.

Al ejecutarse, el programa, se nos presentara un cartel preguntandonos, "Desea finalizar el proceso?(si o no)"; ante el ingreso de cualquier caraterer
distinto de 'Si' el programa lo vera como un 'No' y no finalizara. Al ingresar 'Si', el proceso padre le indicara a los hijos que deben finalizar, a 
medida que van finalizando se nos presentara un cartel indicando la finalizacion de cada tarea. 

Una vez finalizado, obtendremos como resultados dos archivos, "nomina_actualizada" y "nomina_informacion" ubicados en la carpeta 'Archivos de Salida' del proyecto.


HERRAMIENTAS
-------------------------------------------------------------------------
Una ves que el proceso esta en funcionamiento, y mientras no se indique que el mismo finalice podemos monitorear.


MONITOREO
-------
Al usar la interfaz POSIX, para monitorear la memoria compartida utilzada en el proceso usamos:
ls /dev/shm

Para ver los procesos y sus hijos en forma de arbol usamos:
pstree -ap

Para monitorear y ver los semaforos usamos:
ls -l /dev/shm