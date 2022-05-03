/**
* @file circular_buffer.c
* @author Fidel Cem Sarikaya <fcemsarikaya@gmail.com>
* @date 12.11.2021
*
* @brief Circular buffer program module.
*
* This program represents a data structure for a circular buffer that holds arrays of edges in structs.
**/

#include "circular_buffer.h"
#include "generator.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Initializer of the circular buffer.
 * @brief This function initializes the integers of the struct.
 * @details 32 slots for solutions, each with 8 edges. (32 + 32*16 + 4) *4 =  2600 Bytes of total size
 * @param b A pointer to a buffer struct.
 */
void initialize(buffer *b) {
    b->size = 32;
    b->numberofEntries = 0;
    b->head = 0;
    b->tail = 0;
}

/**
 * Bool function.
 * @brief Tells if the buffer is empty or not.
 * @param b A pointer to a buffer struct.
 * @return Returns True or False
 */
static bool bufferEmpty(buffer *b) {
    return (b->numberofEntries == 0);
}

/**
 * Bool function.
 * @brief Tells if the buffer is full or not.
 * @param b A pointer to a buffer struct.
 * @return Returns True or False
 */
static bool bufferFull(buffer *b) {
    return (b->numberofEntries == b->size);
}


/**
 * Write function.
 * @brief This function writes an entry to the buffer.
 * @details Writes an edge cluster with maximum edges of 8 to where the tail index is pointing at in the two-dimensional edge array,
 * and updates the tail index and 'numberofEntries' accordingly.
 * @param b A pointer to a buffer struct.
 * @param entry edge array to be written.
 * @param edgeNumber Number of edges in 'entry'.
 * @return Returns True or False
 */
bool writeBuffer(buffer *b, edge* entry, int edgeNumber) {
    if (bufferFull(b)) {return false;}
    for(int i; i < 8; i++){
        if (i < edgeNumber) {
            b->entries[b->tail][i].U = entry[i].U;
            b->entries[b->tail][i].V = entry[i].V;
            continue;
        }
       b->entries[b->tail][i].U = -1;
       b->entries[b->tail][i].V = -1;
    }
    b->entryEdgeNumbers[b->tail] = edgeNumber;
    b->numberofEntries++;
    b->tail = (b->tail + 1) % b->size;

    return true;
}

/**
 * Read function.
 * @brief This function reads an entry from the buffer.
 * @details Reads an edge cluster with maximum edges of 8 from where the head index is pointing at in the two-dimensional edge array,
 * and updates the head index and numberofentries accordingly.
 * @param b A pointer to a buffer struct.
 * @param edgeNumber An empty integer to take over number of edges of in read edge array.
 * @return Returns the read edge array
 */
edge* readBuffer(buffer *b, int *edgeNumber) {
    if (bufferEmpty(b)) {return false;}

    edge* read = b->entries[b->head];
    *edgeNumber = b->entryEdgeNumbers[b->head];
    b->numberofEntries--;
    b->head = (b->head + 1) % b->size;
    
    return read;
}