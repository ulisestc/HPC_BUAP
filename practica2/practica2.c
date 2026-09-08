/*
    Instrucciones: 
    - Definir una cola circular de tamaño 10 con punteros de inserción y extracción.
    - Implementar un hilo productor que genere números aleatorios y los inserte en la cola y los imprima. (Se suspende si la cola está llena)
    - Implementar dos hilos consumidores que extraigan números de la cola concurrentemente y los imprima. (Se suspende si la cola está vacía)
    - Utilizar semáforos (sem_t) para coordinar espacios disponibles y elementos listos, y un mutex (pthread_mutex_t) para proteger el acceso a la cola.
    - El programa debe correr infinitamente hasta que el usuario lo interrumpa (Ctrl+C).    
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define SIZE 10

// Cola circular
// hacemos la lista circular al usar in = (in + 1) % SIZE y out = (out + 1) % SIZE, siempre iterará del 0 a SIZE-1
int queue[SIZE];
int in = 0;
int out = 0;

pthread_mutex_t mutex; // para bloquear arreglo de la cola
sem_t sem_empty; // indicará espacios vacíos en la cola
sem_t sem_full; // indicará elementos no vacíos en la cola

// PRODUCTOR
void* producer(void* arg)
{
    while(1) // inifinito
    {
        // espera a que haya > 0 espacios vacíos (Para poder producir), cuando entra, 
        // resta 1 (un espacio vacío menos)
        sem_wait(&sem_empty);

        //activar mutex y bloquear acceso
        pthread_mutex_lock(&mutex);

        //generar dato aleatorio e introducirlo
        int dato = rand() % 100;
        queue[in] = dato;
        printf("Productor produjo: %d en posicion %d\n", dato, in);
        
        // sumar a indice de inserción
        in = (in + 1) % SIZE;

        pthread_mutex_unlock(&mutex); // desbloquea cola

        sem_post(&sem_full); // hay +1 lugares llenos
        // sleep(.1);
    }
    return NULL;
}

void* consumer(void* arg)
{
    //saber que consumidor es
    int id = *((int*)arg);

    while (1)
    {
        sem_wait(&sem_full); // espera a que haya  > 0 productos para consumir

        //bloquear mutex
        pthread_mutex_lock(&mutex);

        // consumir
        int dato = queue[out];
        printf("Consumidor %d extrajo: %d de posicion %d\n", id, dato, out);    

        // sumar indice de extracción
        out = (out + 1) % SIZE;

        pthread_mutex_unlock(&mutex); // desbloquear mutex

        sem_post(&sem_empty); // +1 lugares libres!
        // sleep(.1);
    }
    return NULL;
}

int main()
{
    pthread_t hilos[3];

    int id1 = 1;
    int id2 = 2;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem_empty, 0, SIZE); // inicializar semáforo de espacios vacíos con valor SIZE
    sem_init(&sem_full, 0, 0);

    pthread_create(&hilos[0], NULL, producer, NULL);
    pthread_create(&hilos[1], NULL, consumer, &id1);
    pthread_create(&hilos[2], NULL, consumer, &id2);

    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < 3; i++) {
        pthread_join(hilos[i], NULL);
    }

    
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);

    return 0;
}

