/**
* @file supervisor.h
* @author Fidel Cem Sarikaya <fcemsarikaya@gmail.com>
* @date 12.11.2021
*
* @brief Provides the usability of the data structure sharedMemory to be used as shared memory object.
*
* Contains the definition of 'struct sharedMemory'.
**/

#ifndef SUPERVISOR_H  
#define SUPERVISOR_H  

#include "circular_buffer.h"

/**
* @brief The struct object to be used in shared memory. Contains an integer that controls the generator operation and 
* a buffer object that will be written/read by Generator and Supervisor
*/
struct sharedMemory {
    int accessible; 
    buffer circularBuffer;
};


#endif         