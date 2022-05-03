/**
* @file circular_buffer.h
* @author Fidel Cem Sarikaya <fcemsarikaya@gmail.com>
* @date 12.11.2021
*
* @brief Provides the usability of the data structure circular buffer to be used in shared memory.
*
* Contains the definition of 'struct buffer' and 3 functions to control it.
**/

#ifndef CIRCULARBUFFER_H  
#define CIRCULARBUFFER_H  

#include "circular_buffer.h"
#include "generator.h"
#include <stdbool.h>

/**
* @brief Structure of a circular_buffer object: a double array of 'edge' to hold 32 edge clusters with a maximum edge of 8,
* an int array for the edge count of each cluster, integer for head index, tail index, number of entries and total slots for entries. 
*/
typedef struct buffer{
    edge entries[32][8];
    int entryEdgeNumbers[32];
    unsigned int head, tail, numberofEntries, size;
}buffer;

/**
* @brief This function initializes the buffer to be written and read afterwars.
*/
void initialize(buffer *b);

/**
* @brief This function writes a new cluster of edges to the buffer with its number.
*/
bool writeBuffer(buffer *b, edge* entry, int edgeNumber);

/**
* @brief This function reads and returns the next cluster of edges in buffer and their number.
*/
edge* readBuffer(buffer *b, int *edgeNumber);


#endif