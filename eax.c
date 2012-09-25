#include "eax.h"

/**
 * creates a graph that consists of all of the edges in tour A and all of the edges in tour B
 * tA : pointer to tour A
 * tB : pointer to tour B
 * returns : a pointer to the generated graph structure (remember to free this memory when you're finished)
 */
graph_t* mergeTours(const tour_t* const tA, const tour_t* const tB) 
{
	// declarations
	graph_t* R; // the graph we're going to create by combining tA and tB, this is also the return value
	node_t* curNode; // current node we're modifying or looking at in the graph
	city_t* curCity; // current city we're looking at
	int i; // loop counter
	int tourSize;
	
	// initializations
	tourSize = tA->size; // size of the tour
	
	// create an empty graph
	R = (graph_t*)malloc(sizeof(R)); // TODO: FREE_MEM: don't forget to free(R) when you're done using it
	R->size = tourSize;
	for (i=0; i < tourSize; i++)
	{
		R->node[i] = (node_t*)malloc(sizeof(R->node[i])); // TODO: FREE_MEM: don't forget to free each node when freeing R
		R->node[i]->size = 0;
		
		R->node[i]->id = tA->city[i]->id; // node[i] represents tA's city[i]
	}
	
	// visit each city in both tours, adding the edges to R
	// node[i] is the same city as tA->city[i], but feel free to look at the id's to be sure
	for (i=1; i < tourSize-1; i++)
	{
		// tourA's cities
		curCity = tA->city[i];
		curNode = R->node[i]; // tourA's city[i] is represented as node[i]
		curNode->edge[curNode->size++] = R->node[(curCity-1)->id]; // previous node in tourA
		curNode->edge[curNode->size++] = R->node[(curCity+1)->id]; // next node in tourA
		
		// tourB's cities
		curCity = tB->city[i];
		curNode = R->node[curCity->id]; // tourB's city[i] is not the same as R's node[i], but we can still find tB's city[i] in graph R by using IDs
		curNode->edge[curNode->size++] = R->node[(curCity-1)->id]; // previous node in tourB
		curNode->edge[curNode->size++] = R->node[(curCity+1)->id]; // next node in tourB
	}
	// now handle the special case of the first and last nodes in the tour, which link back around (each tour is a cycle)
	// first city in each tour
	curNode = R->node[0]; // tourA's city[i] is represented as node[i]
	curNode->edge[curNode->size++] = R->node[tourSize-1]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[1]; // next node in tourA
	// tourB's cities
	curCity = tB->city[0];
	curNode = R->node[curCity->id]; // tourB's city[i] is not the same as R's node[i], but we can still find tB's city[i] in graph R by using IDs
	curNode->edge[curNode->size++] = R->node[tB->city[tourSize-1]->id]; // previous node in tourB
	curNode->edge[curNode->size++] = R->node[tB->city[1]->id]; // next node in tourB
	
	// last city in each tour
	curNode = R->node[tourSize-1]; // last city in tourA is the same as the last node in R
	curNode->edge[curNode->size++] = R->node[tourSize-2]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[0]; // next node in tourA
	// tourB's cities
	curCity = tB->city[tourSize-1]; // last city in tourB
	curNode = R->node[curCity->id]; // tourB's city[i] is not the same as R's node[i], but we can still find tB's city[i] in graph R by using IDs
	curNode->edge[curNode->size++] = R->node[tB->city[tourSize-2]->id]; // previous node in tourB
	curNode->edge[curNode->size++] = R->node[tB->city[0]->id]; // next node in tourB
	
	// at this point R should be fully populated with all of the edges in tA and tB, so we can return what we calculated
	return R;
} // mergeTours()

/**
 * frees all of the memory used by graph R
 * R : the graph structure whose memory to free
 * side-effects : R will point to invalid memory, it is suggested to set it to null when finished
 */
void freeGraph(graph_t* R)
{
	int i;
	int tourSize;
	tourSize = R->size;
	
	// free each node in the graph
	for (i=0; i < tourSize; i++)
	{
		free(R->node[i]);
	}
	
	// free the graph
	free(R);
}
