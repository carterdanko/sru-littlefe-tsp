////////////////////////////////////////////////////////////////////////////////
//	DESC:	contains types and such used exclusively by the Edge Assembly
//			Crossover part of the algorithm
////////////////////////////////////////////////////////////////////////////////
#ifndef EAX_H // header guard
#define EAX_H

#include "include/tsp.h"


//// CONSTANTS /////////////////////////////////////////////////////////////////
#define MAX_SUB_TOURS MAX_CITIES/4 /* maximum number of sub-tours in an
                                   intermediate tour */
#define MAX_EDGES 4                /* there can only be 4 maximum edges, two
                                   from each parent tour */
#define PANIC_EXIT MAX_CITIES+3    /* some choosing algorithms are implemented
                                   by randomly choosing items that haven't been
                                   chosen yet, choosing again when encountering
                                   one that was already chosen. If this many
                                   iterations of that occur, we exit the loop to
                                   prevent hanging, print an error message, and
                                   halt execution */
#define MAX_ABCYCLES MAX_CITIES/4  /* only for allocation purposes, no boundary
                                   checking assurances */
#define TOUR_A 0
#define TOUR_B 1                   // this is for code clarity
////////////////////////////////////////////////////////////////////////////////


//// INLINE FUNCTIONS //////////////////////////////////////////////////////////
#if PRINT_EDGE_OPERATIONS
	#define REMOVING_PRINT(V, E) \
		printf("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph...\n", \
						(V?V->id:-1), \
						(V && V->edge[E]?V->edge[E]->id:-1), \
						(V?V->tour[E]:-1))
	#define RESTORING_PRINT(V, E) \
		printf("\033[32mrestoring\033[0m edge(v[%i]->v[%i]t%i to graph...\n", \
					(V?V->id:-1), \
					(V && V->edge[E]?V->edge[E]->id:-1), \
					(V?V->tour[E]:-1)) 
#else
	#define REMOVING_PRINT(V, E)
	#define RESTORING_PRINT(V, E)
#endif

// removes edge E from vertex V 
#define REMOVE_EDGE(V,E) do { \
				REMOVING_PRINT(V, E); \
                node_t* _tv; int _tt; _tv=V->edge[--V->size]; _tt=V->tour[V->size]; \
                V->edge[V->size] = V->edge[E]; V->tour[V->size] = V->tour[E]; \
			  	V->edge[E] = _tv; V->tour[E] = _tt; \
			} while(0) 

// restore a previously removed edge back to vertex V
#define RESTORE_EDGE(V,E) do { \
				RESTORING_PRINT(V, E); \
			  	node_t* _tv; int _tt; _tv=V->edge[V->size]; _tt=V->tour[V->size]; \
                V->edge[V->size] = V->edge[E]; V->tour[V->size++] = V->tour[E]; \
			  	V->edge[E] = _tv; V->tour[E] = _tt; \
            } while(0) 
////////////////////////////////////////////////////////////////////////////////


//// STRUCTS ///////////////////////////////////////////////////////////////////
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
 * an edge in a graph used for convenience. When operating on edges, changes to
 * edges must be manually maintained between the graph struct and the edge struct.
 */
typedef struct edge_struct {
#if USE_EDGE_TABLE
     int v1, v2; // city ids
#else
     node_t* v1, *v2; // end points of the edge
#endif
	int cycle; /* which sub-cycle this edge belongs to, 0 for "no" cycle (the
                edge was created by the merging process, but can still be
                considered a candidate) */
     float cost; // the "cost" of the edge, just the distance
} edge_t;
 
/**
 * used when combining two tours to create a graph consisting of the
 * edges in both tourA and tourB
 */
typedef struct {
     node_t* node[MAX_CITIES];/* each node is a city, so the maximum number of
                              nodes and cities is equal */
     node_t alloc_node[MAX_CITIES]; // allocated node objects
     int size; // number of nodes in this graph
} graph_t;

/* performs the entire EAX algorithm, returning a new valid tour created from
   the two parent tours */
void performEAX(char* memory_chunk, tour_t* CitiesA, tour_t* CitiesB,
 tour_t* tourA, tour_t* tourB, tour_t* tourC);
////////////////////////////////////////////////////////////////////////////////


#endif // header guard
