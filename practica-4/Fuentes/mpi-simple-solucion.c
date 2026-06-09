/**
 *  Topologia de anillo usando MPI puro — enfoque manual con SPMD.
 *
 *  REGLA: Solo se pueden usar primitivas MPI vistas en clase (teoria/practica).
 *  Fuente: teorias/clase-7.md  (Fundamentos de MPI puro)
 *
 *  Primitivas permitidas utilizadas aqui:
 *    - MPI_Init / MPI_Finalize          (iniciar/finalizar entorno)
 *    - MPI_Comm_size / MPI_Comm_rank    (consultar comunicador)
 *    - MPI_Send / MPI_Recv              (comunicacion punto a punto bloqueante)
 *
 *  En una topologia de anillo cada proceso se comunica unicamente con
 *  su vecino izquierdo y su vecino derecho:
 *    - Envia hacia la derecha: (myrank + 1) % size
 *    - Recibe desde la izquierda: (myrank - 1 + size) % size
 *
 *  Para evitar deadlock (ver teorias/clase-7.md:
 *  "Comunicaciones punto a punto Caso de deadlock") los procesos pares
 *  envian primero y reciben despues, mientras que los impares hacen
 *  lo contrario: reciben primero y envian despues.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char* argv[])
{
    int myrank;          /* rank de este proceso — MPI_Comm_rank */
    int size;            /* cantidad de procesos — MPI_Comm_size */
    int tag = 0;         /* etiqueta del mensaje, en este ejercicio solo nos intereza el primer mensaje */
    MPI_Status status;   /* estructura de estado MPI_Recv */
    char send_msg[BUFSIZ];
    char recv_msg[BUFSIZ];

    MPI_Init(&argc, &argv);

    /* MPI_Comm_size / MPI_Comm_rank
     */
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if (size < 2) {
        if (myrank == 0) {
            fprintf(stderr,
                "Se requieren al menos 2 procesos para formar una topologia de anillo.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    /* Calculo de los vecinos en el anillo.
     * derecha -> siguiente proceso circularmente.
     * izquierda -> proceso anterior circularmente (sumamos size para no tener negativos).
     */
    int derecha = (myrank + 1) % size;
    int izquierda = (myrank - 1 + size) % size;

    /* Construir el mensaje original a enviar */
    /* sprintf: formatea el mensaje con el rank del proceso. */
    /*parametro 1: buffer donde se almacenara el mensaje formateado
     *parametro 2: formato del mensaje, similar a printf
     *parametro 3: valor del rank del proceso para incluirlo en el mensaje
    */
    sprintf(send_msg, "Hola desde el proceso %d!", myrank);

    /*
     * Comunicacion en anillo con punto a punto bloqueante.
     *
     * Para evitar deadlock se alterna el orden segun la paridad del rank:
     *   - Procesos pares:    envian primero, reciben despues.
     *   - Procesos impares:  reciben primero, envian despues.
     *
     * Esto garantiza que un proceso que envia nunca se enfrente con un
     * vecino que tambien intente enviar al mismo tiempo sin nadie recibiendo.
     */
    if (myrank % 2 == 0) {
        /* Proceso par: envia al vecino derecho */
        /* MPI_Send: envio bloqueante, retorna cuando el buffer puede reusarse.
         * Bajo este enfoque el Master es un colaborador mas, no tiene un rol especial.
         * Todos los procesos contaran con la informacion requerida para enviar y recibir  
        */
        MPI_Send(send_msg, strlen(send_msg) + 1, MPI_CHAR, derecha, tag, MPI_COMM_WORLD);

        /* Luego recibe del vecino izquierdo */
        /* MPI_Recv: recepcion bloqueante, espera hasta que llegue el mensaje.
         */
        MPI_Recv(recv_msg, BUFSIZ, MPI_CHAR, izquierda, tag, MPI_COMM_WORLD, &status);
    } else {
        /* Proceso impar: recibe primero del vecino izquierdo */
        MPI_Recv(recv_msg, BUFSIZ, MPI_CHAR, izquierda, tag, MPI_COMM_WORLD, &status);
        /* Luego envia al vecino derecho */
        MPI_Send(send_msg, strlen(send_msg) + 1, MPI_CHAR, derecha, tag, MPI_COMM_WORLD);
    }

    /* Cada proceso muestra el mensaje que recibio de su vecino izquierdo,
     * completando asi la verificacion de la topologia de anillo.
     */
    printf("Proceso %d recibio de su vecino izquierdo (proceso %d): %s\n",
           myrank, izquierda, recv_msg);

    /* Finaliza el entorno MPI antes de salir del entorno MPI.
     * Obligatorio para todo proceso.
     */
    MPI_Finalize();
    return EXIT_SUCCESS;
}
