#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Estructura para pasar datos a cada hilo
typedef struct {
    int** matriz_uno;        // Puntero a la matriz completa
    int** matriz_dos;        // Puntero a la matriz completa
    int** matriz_resultante;        // Puntero a la matriz completa
    int fila_inicio;     // Índice de fila donde empieza a trabajar el hilo
    int fila_fin;        // Índice de fila donde termina
    int cols_uno;            // Número de columnas de la matriz1
    int cols_dos;            // Número de columnas de la matriz2
} DatosHilo;

// Función que ejecuta cada hilo: eleva al cuadrado los elementos de sus filas asignadas
void* multiplicar(void* arg) {
    DatosHilo* datos = (DatosHilo*)arg;
    for (int i = datos->fila_inicio; i < datos->fila_fin; i++) {
        for (int j = 0; j < datos->cols_uno; j++){
            for(int k=0;k< datos -> cols_uno; k++){
                datos->matriz_resultante[i][j] += datos->matriz_uno[i][k] * datos->matriz_dos[k][j];
            }
        }
    }
    return NULL;
}

int main() {
    char nombre_archivo1[100];
    char nombre_archivo2[100];
    int M, N, P, num_hilos;

    // Pedir al usuario el nombre de archivos y el número de hilos
    printf("Nombre del archivo 1: ");
    scanf("%s", nombre_archivo1);

    printf("Nombre del archivo 2: ");
    scanf("%s", nombre_archivo2);

    printf("Número de hilos a usar: ");
    scanf("%d", &num_hilos);

    // Abrir el archivo 1
    FILE* archivo1 = fopen(nombre_archivo1, "r");
    if (!archivo1) {
        perror("No se pudo abrir el archivo 1");
        return 1;
    }
    // Abrir el archivo 2
    FILE* archivo2 = fopen(nombre_archivo2,"r");
    if (!archivo2) {
        perror("No se pudo abrir el archivo 2");
        return 1;
    }

    // Leer dimensiones de la matriz 1
    fscanf(archivo1, "%d %d", &M, &N);

    int temp = N;

    // Leer dimensiones de la matriz 2
    fscanf(archivo2, "%d %d", &N, &P);

    if (temp != N) {
        perror("Error: Las dimensiones de las matrices no son compatibles para la multiplicación.\n");
        return 1;
    }

    // Reservar memoria dinámica para la matriz
    int** matriz_uno = malloc(M * sizeof(int*));
    for (int i = 0; i < M; i++) {
        matriz_uno[i] = malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            fscanf(archivo1, "%d", &matriz_uno[i][j]);  // Leer cada valor de la matriz
        }
    }
    // Reservar memoria dinámica para la matriz
    int** matriz_dos = malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        matriz_dos[i] = malloc(P * sizeof(int));
        for (int j = 0; j < P; j++) {
            fscanf(archivo2, "%d", &matriz_dos[i][j]);  // Leer cada valor de la matriz
        }
    }
    // Reservar memoria dinámica para la matriz
    int** matriz_resultante = malloc(M * sizeof(int*));
    for (int i = 0; i < M; i++) {
        matriz_resultante[i] = malloc(P * sizeof(int));
    }

    fclose(archivo1);  // Cerrar el archivo
    fclose(archivo2);  // Cerrar el archivo

    // Crear hilos
    pthread_t hilos[num_hilos];
    DatosHilo datos[num_hilos];

    int filas_por_hilo = M / num_hilos;    // Cuántas filas debe procesar cada hilo
    int resto = M % num_hilos;             // Si no se divide exacto, distribuir el sobrante

    int fila_actual = 0;
    for (int i = 0; i < num_hilos; i++) {
        datos[i].matriz_uno = matriz_uno;
        datos[i].matriz_dos = matriz_dos;
        datos[i].matriz_resultante = matriz_resultante;
        datos[i].cols_uno = N;
        datos[i].cols_dos = P;
        datos[i].fila_inicio = fila_actual;
        datos[i].fila_fin = fila_actual + filas_por_hilo + (i < resto ? 1 : 0); // Reparto justo
        pthread_create(&hilos[i], NULL, multiplicar, &datos[i]);
        fila_actual = datos[i].fila_fin;  // Avanzar al siguiente bloque de filas
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Mostrar matriz resultante
    printf("\nMatriz Resultante:\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            printf("%5d ", matriz_resultante[i][j]);
        }
        printf("\n");
    }

    // Liberar la memoria reservada
    for (int i = 0; i < M; i++) {
        free(matriz_uno[i]);
    }
    free(matriz_uno);
    // Liberar la memoria reservada
    for (int i = 0; i < N; i++) {
        free(matriz_dos[i]);
    }
    free(matriz_dos);
    // Liberar la memoria reservada
    for (int i = 0; i < M; i++) {
        free(matriz_resultante[i]);
    }
    free(matriz_resultante);

    return 0;
}

// source https://www.mathsisfun.com/algebra/matrix-multiplying.html
