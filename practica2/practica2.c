/*
    Instrucciones: 
    - Definir una cola circular de tamaño 10 con punteros de inserción y extracción.
    - Implementar un hilo productor que genere números aleatorios y los inserte en la cola y los imprima. (Se suspende si la cola está llena)
    - Implementar dos hilos consumidores que extraigan números de la cola concurrentemente y los imprima. (Se suspende si la cola está vacía)
    - Utilizar semáforos (sem_t) para coordinar espacios disponibles y elementos listos, y un mutex (pthread_mutex_t) para proteger el acceso a la cola.
    - El programa debe correr infinitamente hasta que el usuario lo interrumpa (Ctrl+C).    
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Cola circular







