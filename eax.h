/**
 * contains types and such used exclusively by the Edge Assembly Crossover part of the algorithm
 */
#ifndef EAX_H // header guard
#define EAX_H

#include "tsp.h"

#define MAX_SUB_TOURS 10  // maximum number of sub-tours in an intermediate tour
#define MAX_EDGES 4       // there can only be 4 maximum edges, two from each parent tour
#define PANIC_EXIT 100  // some choosing algorithms are implemented by randomly choosing items that haven't been chosen yet, choosing again when encountering one that was already chosen. If this many iterations of that occur, we exit the loop to prevent hanging, print an error message, and halt execution

/**
 * a node in a graph
 */
typedef struct node_struct {
	struct node_struct* edge[MAX_EDGES]; // neighbor nodes of this node
	int tour[MAX_EDGES]; // which tour each edge belongs to. 0 for A, 1 for B, etc.
	int size; // number of edges in this node
	int id; // the id of this node in the graph (same as the city it represents)
} node_t;

/**
 * used when combining two tours to create a graph consisting of the
 * edges in both tourA and tourB
 */
typedef struct {
	node_t* node[MAX_CITIES]; // each node is a city, so the maximum number of nodes and cities is equal
	int size; // number of nodes in this graph
} graph_t;

/**
 * represents an intermediate tour in the eax algorithm.
 * this tour is generated by applying an E-set to Tour_A
 */
typedef struct {
	tour_t* subTour[MAX_SUB_TOURS]; // the subtours contained in this intermediate (iteratively combined in the algorithm)
	int size; // number of sub tours currently in the intermediate
} intermediate_t;

// tour merging and graph generation/deletion
graph_t* mergeTours(const tour_t* const tA, const tour_t* const tB); // creates a graph that consists of all of the edges in tour A and all of the edges in tour B
void freeGraph(graph_t* R); // frees all of the memory used by graph R

// A-B cycles
int generateABCycles(graph_t* R /*byref*/, tour_t** cycles /*byref*/); // generates A-B cycles and stores them in cycles, returns size of the array of sub-tours

// E-sets

// intermediates generation and modification into valid tours

#endif // header guard
