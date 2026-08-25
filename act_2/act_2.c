#define _POSIX_C_SOURCE 199309L // -> para time.h en linuxyy

#include <stdio.h> //in and out
#include <stdlib.h> // malloc, free
#include <pthread.h> // hilos
// #include <linux/time.h> // tiempo
#include <time.h> // tiempo

// Estructura para pasar datos a cada hilo
typedef struct {
    int** matriz_uno;        // Punteros a las matrices
    int** matriz_dos;
    int** matriz_resultante; 
    // cada hilo iterara solo sobre una casilla de la matriz resultante
    // Como el usuario puede pedir cualquier número de hilos, cada hilo debe saber sobre qué casilla de la matriz resultante iterar
    int cols_resultante;
    // Casillas EN 1D, se puede convertir a 2D usando / y % cols
    int casilla_inicio; 
    int casilla_fin;
    // dimensión compartida (M)
    int dim_comp;
} DatosHilo;

// Hilo iterando sobre una misma casilla
void *multiplicar_elemento(void *arg) {
    DatosHilo* datos = (DatosHilo*)arg;
    for(int idx = datos->casilla_inicio; idx < datos->casilla_fin; idx++) {
        int fila = idx / datos->cols_resultante;
        int col = idx % datos->cols_resultante;
        int suma = 0;
        for (int k = 0; k < datos->dim_comp; k++) {
            suma += datos->matriz_uno[fila][k] * datos->matriz_dos[k][col];
        }
        datos->matriz_resultante[fila][col] = suma;
    }
    return NULL;
}

// multiplicación secuencial de matrices (algorimo clásico)
void multiplicar_secuencial(int** matriz_uno, int** matriz_dos, int** matriz_resultante, int N, int M, int Z) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < Z; j++) {
            matriz_resultante[i][j] = 0;
            for (int k = 0; k < M; k++) {
                matriz_resultante[i][j] += matriz_uno[i][k] * matriz_dos[k][j];
            }
        }
    }
}

int main() {
    char nombre_archivo[100];
    int num_hilos;

    // Pedir al usuario el nombre del archivo y el número de hilos
    printf("Nombre del archivo: ");
    scanf("%s", nombre_archivo);

    
    // Abrir el archivo 1
    FILE* archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        perror("No se pudo abrir el archivo");
        return 1;
    }

    // Declarar Dimensiones de las matrices
    int N_uno, M_uno, M_dos, Z_dos;

    // Leer dimensiones de la matriz 1
    fscanf(archivo, "%d %d", &N_uno, &M_uno);


    // Reservar memoria dinámica para la matriz 1
    int** matriz_uno = malloc(N_uno * sizeof(int*));
    for (int i = 0; i < N_uno; i++) {
        matriz_uno[i] = malloc(M_uno * sizeof(int));
        for (int j = 0; j < M_uno; j++) {
            fscanf(archivo, "%d", &matriz_uno[i][j]);  // Leer cada valor de la matriz
        }
    }

    // ler dimensiones de la matriz 2
    fscanf(archivo, "%d %d", &M_dos, &Z_dos);
    // comprobar dimensiones compatibles
    if (M_uno != M_dos) {
        perror("Error: Las dimensiones de las matrices no son compatibles para la multiplicación.\n");
        return 1;
    }
    
    // Reservar memoria dinámica para la matriz 2
    int** matriz_dos = malloc(M_dos * sizeof(int*));
    for (int i = 0; i < M_dos; i++) {
        matriz_dos[i] = malloc(Z_dos * sizeof(int));
        for (int j = 0; j < Z_dos; j++) {
            fscanf(archivo, "%d", &matriz_dos[i][j]);  // Leer cada valor de la matriz
        }
    }
    // Reservar memoria dinámica para la matriz resultante
    int** matriz_resultante = malloc(N_uno * sizeof(int*));
    for (int i = 0; i < N_uno; i++) {
        matriz_resultante[i] = calloc(Z_dos, sizeof(int)); //calloc inicializa con 0, de tal manera podemos usar += sin errores
    }
    
    fclose(archivo);  // Cerrar el archivo
    
    // calcular numero casillas de matriz resultante
    int num_casillas = N_uno * Z_dos;

    // pedir al usuario el número de hilos a usar
    printf("Número de hilos a usar: (se reocmienda usar hilos == casillas de la matriz resultante (%d)): ", num_casillas);
    scanf("%d", &num_hilos);
    
    // para medir tiempos
    struct timespec start, end;
    double tiempo_secuencial, tiempo_paralelo;

    // --------------------------------------------------------------------
    // ejecución secuencial
    clock_gettime(CLOCK_MONOTONIC, &start);
    multiplicar_secuencial(matriz_uno, matriz_dos, matriz_resultante, N_uno, M_uno, Z_dos);
    clock_gettime(CLOCK_MONOTONIC, &end);
    tiempo_secuencial = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Limpiar matriz_resultante (poner en 0) antes de la paralela...
    for(int i=0; i<N_uno; i++){
        for(int j=0; j<Z_dos; j++){
            matriz_resultante[i][j] = 0;  
        }
    }
    // --------------------------------------------------------------------
    // Ejecución paralela
    // Crear hilos
    pthread_t hilos[num_hilos];
    DatosHilo datos[num_hilos];

    int casillas_por_hilo = num_casillas / num_hilos;    // Cuántas casillas debe procesar cada hilo
    int resto = num_casillas % num_hilos;             // Si no se divide exacto, distribuir el sobrante

    // iniciatemporizador
    clock_gettime(CLOCK_MONOTONIC, &start);

    int casilla_actual = 0;
    for (int i = 0; i < num_hilos; i++) {
        datos[i].matriz_uno = matriz_uno;
        datos[i].matriz_dos = matriz_dos;
        datos[i].matriz_resultante = matriz_resultante;
        datos[i].cols_resultante = Z_dos;
        datos[i].dim_comp = M_uno;
        datos[i].casilla_inicio = casilla_actual;
        datos[i].casilla_fin = casilla_actual + casillas_por_hilo + (i < resto ? 1 : 0); // Distribuir el resto entre los primeros hilos
        pthread_create(&hilos[i], NULL, multiplicar_elemento, &datos[i]);
        casilla_actual = datos[i].casilla_fin;
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    // detener temporizador
    clock_gettime(CLOCK_MONOTONIC, &end);
    tiempo_paralelo = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // -----------------------------------------------------
    // Mostrar tiempos
    printf("Tiempo secuencial: %.6f segundos\n", tiempo_secuencial);
    printf("Tiempo paralelo: %.6f segundos\n", tiempo_paralelo);

    // mostrar speedup
    if (tiempo_paralelo > 0) {
        double speedup = tiempo_secuencial / tiempo_paralelo;
        printf("Speedup: %.2f\n", speedup);
    } else {
        printf("Tiempo paralelo es 0, no se puede calcular speedup.\n");
    }

    // mostrar resultado de la multiplicación
    // no mostrar si la matriz es muy grande
    if (N_uno * Z_dos <= 100) {
        printf("\nMatriz Resultante:\n");
        for (int i = 0; i < N_uno; i++) {
            for (int j = 0; j < Z_dos; j++) {
                printf("%5d ", matriz_resultante[i][j]);
            }
            printf("\n");
        }
    }
    else {
        printf("\nMatriz resultante demasiado grande para mostrar (dimensiones: %d x %d)\n", N_uno, Z_dos);
    }

    // liberar memoria reservada
    for (int i = 0; i < N_uno; i++) {
        free(matriz_uno[i]);
    }
    free(matriz_uno);
    for (int i = 0; i < M_dos; i++) {
        free(matriz_dos[i]);
    }
    free(matriz_dos);
    for (int i = 0; i < N_uno; i++) {
        free(matriz_resultante[i]);
    }
    free(matriz_resultante);

    return 0;
}

// source https://www.mathsisfun.com/algebra/matrix-multiplying.html
// fix time.h: https://stackoverflow.com/a/79220510