/**
* @file generator.h
* @author Fidel Cem Sarikaya <fcemsarikaya@gmail.com>
* @date 13.11.2021
*
* @brief Provides the usability of the main data structure Generator uses.
*
* Contains the definition of 'struct edge'.
**/

#ifndef GENERATOR_H  
#define GENERATOR_H  

/**
* @brief The representation of an edge using struct, where 'int U' corresponds to U end of the edge. Same with V.
*/
typedef struct edge{
    int U;
    int V;
}edge;


#endif                