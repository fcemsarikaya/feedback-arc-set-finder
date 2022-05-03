/**
* @file supervisor.c
* @author Fidel Cem Sarikaya <fcemsarikaya@gmail.com>
* @date 13.11.2021
*
* @brief Supervisor program module.
*
* This program reads possible minimal feedback arc set solutions from a shared memory area and remembers the best of them.
**/

#include "generator.h"
#include "circular_buffer.h"
#include "supervisor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_NAME    "/11941488_sharedMemory"

#define SEM_FREE    "/semaphor_free"
#define SEM_USED    "/semaphor_used"
#define MUTEX       "/semaphor_mutex"



volatile sig_atomic_t RUN = 1;          /**<The while loop running condition. */

/**
 * Signal handling function.
 * @brief This function handles the given signal by decrementing a global variable.
 * @details global variables: RUN
 * @param signal The given signal.
 */
void handle_signal(int signal) {
    RUN = 0;}

/**
 * Program entry point.
 * @brief The program starts here. This program repeatedly reads a shared memory buffer and saves, then demonstrates the best feedback arc set solution so far.
 * @details After shared memory and the object to use in it is set, the program keeps reading from the shared memory in a while loop which is sychronized by
 * semaphores. The best solution so far is saved and demonstrated in every iteration of loop. If a better one is reached, gets switched with the current one.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char const *argv[]) {
    edge* bestSolution;
    int bestSolutionEdges = INT_MAX;

    //input check
    if (argc != 1) {
            fprintf(stderr,"Usage: \t%s\t (without any arguments)\n", argv[0]);
            return EXIT_FAILURE;}

   //signal handling
    struct sigaction sa;
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //shared memory setup
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
            fprintf(stderr, "Shared memory couldn't be set. (SUP)\n");
            return EXIT_FAILURE;}
    
    if (ftruncate(shmfd, sizeof(struct sharedMemory)) < 0) {
            fprintf(stderr, "Shared memory couldn't be set. (SUP)\n");
            return EXIT_FAILURE;}
    
    //shared object setup
    struct sharedMemory *access;
    access = mmap(NULL, sizeof(*access), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    
    if (access == MAP_FAILED) {
            fprintf(stderr, "Shared memory couldn't be set. (SUP)\n");
            return EXIT_FAILURE;}
    
    //shared object initialization
    access->accessible = 1;
    initialize(&(access->circularBuffer));

    //semaphore remove in prior for stability
    sem_unlink(SEM_FREE);
    sem_unlink(SEM_USED);
    sem_unlink(MUTEX);

    //opening semaphores
    sem_t *s_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600 , 32);
    sem_t *s_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600 , 0);
    sem_t *s_mut = sem_open(MUTEX, O_CREAT | O_EXCL, 0600 , 1);    


    //while loop for reading from the circular buffer
    //synchronization with semaphores
    int tmpEdges = 0;
    edge* tmpSolution;

    fprintf(stdout, "\nWaiting for a generator...\n\n");
    while(RUN == 1 && bestSolutionEdges != 0) {
        sem_wait(s_used);
        tmpSolution = readBuffer(&(access->circularBuffer), &tmpEdges);
        if (tmpEdges < bestSolutionEdges){
            bestSolution = tmpSolution;
            bestSolutionEdges = tmpEdges;
            if (bestSolutionEdges == 0) {
            fprintf(stdout, "The graph is acyclic!\n");
            }
            else {
            fprintf(stdout, "Solution with %d edges: ", bestSolutionEdges);
            for (int i = 0; i < bestSolutionEdges; i++) {
                fprintf(stdout, "%d-%d ", bestSolution[i].U, bestSolution[i].V);
            }
            fprintf(stdout, "\n");
            }
        }
        sem_post(s_free);
    }
    access->accessible = 0;
    fprintf(stdout, "\nExiting...\n");
    
    //unmapping the shared object
    if (munmap(access, sizeof(*access)) == -1) {
            fprintf(stderr, "Shared memory couldn't be shut. (SUP)\n");
            return EXIT_FAILURE;}

    //closing shared memory
    if (close(shmfd) == -1) {
            fprintf(stderr, "Shared memory couldn't be shut. (SUP)\n");
            return EXIT_FAILURE;}
    if (shm_unlink(SHM_NAME) == -1) {
            fprintf(stderr, "Shared memory couldn't be shut. (SUP)\n");
            return EXIT_FAILURE;}

    //closing semaphores
    sem_close(s_free);
    sem_close(s_used);
    sem_close(s_mut);

    sem_unlink(SEM_FREE);
    sem_unlink(SEM_USED);
    sem_unlink(MUTEX);

    return EXIT_SUCCESS;
}