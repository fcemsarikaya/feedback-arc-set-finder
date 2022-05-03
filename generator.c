/**
* @file generator.c
* @author Fidel Cem Sarikaya <fcemsarikaya@gmail.com>
* @date 13.11.2021
*
* @brief Generator program module.
*
* This program takes graph edges in arguments and sends feedback arc set solutions of the represented graph to a shared memory.
**/

#include "generator.h"
#include "supervisor.h"
#include "circular_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>  
#include <semaphore.h>


char *MYPROG;           /**<The program name. */

#define SHM_NAME    "/11941488_sharedMemory"

#define SEM_FREE    "/semaphor_free"
#define SEM_USED    "/semaphor_used"
#define MUTEX       "/semaphor_mutex"


/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: MYPROG
 */
static void usage(void) {
    fprintf(stderr,"Usage: \t%s EDGE1 EDGE2 EDGE3 [EDGE 4...] \n\nEDGE = (INTEGER)'-'(INTEGER)", MYPROG);
    exit(EXIT_FAILURE);}

/**
 * @brief This function takes an integer list and an integer as parameters and returns true if the list containts that element.
 * @param list This array gets interrogated.
 * @param element This element is searched in the array.
 * @param listlen The element number of 'list'.
 * @return Returns 1 or 0.
 */
static int inList(int list[], int element, size_t listLen) {
    for (int i = 0; i < listLen; i++) {
        if (list[i] == element) {return 1;}
    }
    return 0;
}
/**
 * Duplicate element removing function.
 * @brief This function takes a list as parameter and returns a list that every element from previous list is unique.
 * @details Contains dynamic memory allocation, return value to be freed later.
 * @param list Unfiltered list of vertices.
 * @param listLen Length of the list.
 * @param uniqueCount Pointer to an empty integer.
 * @return Returns the input list without duplicates of vertices.
 */ 
static int* duplicateRemover(int *list, int listLen, int *uniqueCount) {
    int* uniqueList =  (int* )malloc(listLen * sizeof(int));
    if (uniqueList == NULL) {exit(EXIT_FAILURE);}

    int uniqueIndex = 0;
    for (int i = 0; i < listLen; i++) {
        if (inList(uniqueList, list[i], uniqueIndex) == 0) {
            uniqueList[uniqueIndex] = list[i];
            uniqueIndex++;
            }
        }
    int* tmp =  realloc (uniqueList, uniqueIndex*sizeof(int));
    if (tmp == NULL) {exit(EXIT_FAILURE);}
    uniqueList = tmp;

    *uniqueCount = uniqueIndex;
    return uniqueList;
}

/**
 * @brief This function takes a list as parameter and returns it randomly shuffled, using Fisher-Yates algorithm.
 * @param list List of vertices to be shuffled.
 * @param n Number of vertices.
 */
static void fisherYates(int *list, int n) { 
     int i, j, tmp;
    srand(time(NULL));
     for (i = n - 1; i > 0; i--) { 

         j = rand() % (i + 1); 
         tmp = list[j];
         list[j] = list[i];
         list[i] = tmp;
     }
}

/**
 * Random feedback arc set generating function.
 * @brief This function takes some unique vertices and edge combinations with these vertices as parameter and returns random feedback arc sets for them.
 * @details The unique vertices get shuffled first, then removable edges get detected and returned as an edge array.
 * @param vertices The unique vertice array.
 * @param verticeCount Number of unique vertices.
 * @param edges Array of all edges of the input graph.
 * @param edgeCount Number of edges.
 * @param solutionCount Pointer to an empty integer.
 * @return Returns an edge array.
 */
static edge* randomizer(int *vertices, int verticeCount, edge *edges, int edgeCount, int *solutionCount) {
    fisherYates(vertices, verticeCount);

    int U[(verticeCount * (verticeCount + 1) / 2)];
    int V[(verticeCount * (verticeCount + 1) / 2)];

    int listIndex = 0;
    for (int i = 0; i < (verticeCount - 1); i++) {
        for (int j = i + 1; j < verticeCount; j++) {
            for (int k = 0; k < edgeCount; k++) {
                if (edges[k].U == vertices[j] && edges[k].V == vertices[i]) {
                    U[listIndex] = vertices[j];
                    V[listIndex] = vertices[i];
                    listIndex++;
                }
            }
        }
    }
    edge *solution = (edge*) malloc(listIndex*2*(sizeof(int)));
    if(solution == NULL) {exit(EXIT_FAILURE);}

    for (int i = 0; i < listIndex; i++) {
        solution[i].U = U[i];
        solution[i].V = V[i];
    }
    *solutionCount = listIndex;
    return solution;
}

/**
 * Program entry point.
 * @brief The program starts here. An Edge array derived from arguments and feedback arc sets of
 * the represented graph keeps getting written to a shared memory buffer for Supervisor to read.
 * @details A unique vertice list and an edge struct array of input edges get created and they get used in a function to create feedback arc sets
 * repeatedly in a while loop that's synchronized by semaphores, where the solutions get written to a shared memory array. 
 * This program writes only the solutions with maximum 8 edges to the shared memory. 
 * global variables: MYPROG
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *const argv[]){
    MYPROG = argv[0];

    //checks arguments
    if (argc < 4) {usage();}
    int opt;
     while((opt = getopt(argc, argv, "")) != -1) 
        { 
            switch(opt) 
            { 
                case '?': 
                    usage();
                    break; 
                default:
                    assert(0);
                    break;
            } 
        }

    //checks input and every vertice in input to array 'vertices', writes every edge to struct array 'edges'
    int vertices[(argc-1)*2];
    edge edges[argc-1];
    for (int i = 1; i < argc; i++){
        if (sizeof(argv[i]) < 4) {usage();}
        else{
            char *token = strtok(argv[i], "-");
            if (token == NULL) {usage();}

            int count = 0;
            while (token != NULL)
                {
                int num;
                char ch;
                if (sscanf(token, "%i%c", &num, &ch) != 1) {{usage();}}
                vertices[(i*2-2) + count] = num;
                if (count == 0) {edges[i-1].U = num;}
                else if (count == 1) {edges[i-1].V = num;}

                token = strtok(NULL, "-");
                
                if (count > 1) {usage();}
                count++;
                }
            count = 0;
            }
    }

    //shared memory setup
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
            fprintf(stderr, "Shared memory couldn't be set. (GEN)\n");
            return EXIT_FAILURE;}
    
    //shared object setup
    struct sharedMemory *access;
    access = mmap(NULL, sizeof(*access), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (access == MAP_FAILED) {
            fprintf(stderr, "Shared memory couldn't be set. (GEN)\n");
            return EXIT_FAILURE;}
   
    //opening semaphores
    sem_t *s_free = sem_open(SEM_FREE, 0);
    sem_t *s_used = sem_open(SEM_USED, 0);
    sem_t *s_mut = sem_open(MUTEX, 0);


    //while loop for writing to the circular buffer
    //synchronization with semaphores
    int uniqueVerticeCount;
    int* uniqueVertices = duplicateRemover(vertices, (argc-1)*2, &uniqueVerticeCount);
    
    edge toWrite[8];
    edge *solution;
    int edgeNumber = 0;
    while(access->accessible == 1) {
        sem_wait(s_free);
        sem_wait(s_mut);
        solution = randomizer(uniqueVertices, uniqueVerticeCount, edges, argc-1, &edgeNumber);
        if (edgeNumber < 9) {       //only the solutions with maximum 8 edges are written to the buffer
            for (int i = 0; i < 8; i++) {
                if (i < edgeNumber) {
                    toWrite[i].U = solution[i].U;
                    toWrite[i].V = solution[i].V;
                continue;
                }
                toWrite[i].U = -1;
                toWrite[i].V = -1;
            }
            writeBuffer(&(access->circularBuffer), toWrite, edgeNumber);
            }
        sem_post(s_mut);
        sem_post(s_used);
    }
    //unmapping the shared object
    if (munmap(access, sizeof(*access)) == -1) {
            fprintf(stderr, "Shared memory couldn't be shut: (GEN)\n");
            return EXIT_FAILURE;}
    
    if (close(shmfd) == -1) {
            fprintf(stderr, "Shared memory couldn't be shut. (GEN)\n");
            return EXIT_FAILURE;}

    //freeing resources
    free(uniqueVertices);
    free(solution);

    //closing semaphores
    sem_close(s_free);
    sem_close(s_used);
    sem_close(s_mut);

    return EXIT_SUCCESS;
}