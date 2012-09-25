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
	printf("\ncreating empty graph (of size %i)...", tourSize); //TODO debug remove
	R = (graph_t*)malloc(sizeof(graph_t)); // TODO: FREE_MEM: don't forget to free(R) when you're done using it
	R->size = tourSize;
	for (i=0; i < tourSize; i++)
	{
		R->node[i] = (node_t*)malloc(sizeof(node_t)); // TODO: FREE_MEM: don't forget to free each node when freeing R
		R->node[i]->size = 0;
		
		R->node[i]->id = i; // Each node in the graph is numbered the same as the master city list
	}
	printf("done!\n"); //TODO debug remove
	
	// visit each city in both tours, adding the edges to R
	// node[i] is the same city as tA->city[i], but feel free to look at the id's to be sure
	printf("populating the graph (merging the edges)..."); //TODO debug remove
	for (i=1; i < tourSize-1; i++)
	{
		// tourA's cities
		curNode = R->node[tA->city[i]->id]; // grab the node representing tour A's city[i]
		curNode->edge[curNode->size++] = R->node[tA->city[i-1]->id]; // previous node in tourA
		curNode->edge[curNode->size++] = R->node[tA->city[i+1]->id]; // next node in tourA
		
		// tourB's cities
		curNode = R->node[tB->city[i]->id]; // grab the node representing tour B's city[i]
		curNode->edge[curNode->size++] = R->node[tB->city[(i-1+tourSize)%tourSize]->id]; // previous node in tourB
		curNode->edge[curNode->size++] = R->node[tB->city[(i+1)%tourSize]->id]; // next node in tourB
	}
	printf("Special cases..."); //TODO debug remove
	// now handle the special case of the first and last nodes in the tour, which link back around (each tour is a cycle)
	// first city in each tour
	curNode = R->node[tA->city[0]->id]; // grab the node representing tour A's city[0]
	curNode->edge[curNode->size++] = R->node[tA->city[tourSize-1]->id]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[tA->city[1]->id]; // next node in tourA
	// tourB's cities
	curNode = R->node[tB->city[0]->id]; // grab the node representing tour B's city[0]
	curNode->edge[curNode->size++] = R->node[tB->city[tourSize-1]->id]; // previous node in tourB
	curNode->edge[curNode->size++] = R->node[tB->city[1]->id]; // next node in tourB
	
	// last city in each tour
	curNode = R->node[tA->city[tourSize-1]->id]; // grab the node representing tour A's last city
	curNode->edge[curNode->size++] = R->node[tA->city[tourSize-2]->id]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[tA->city[0]->id]; // next node in tourA
	// tourB's cities
	curNode = R->node[tB->city[tourSize-1]->id]; // grab the node representing tour B's last city
	curNode->edge[curNode->size++] = R->node[tB->city[tourSize-2]->id]; // previous node in tourB
	curNode->edge[curNode->size++] = R->node[tB->city[0]->id]; // next node in tourB
	printf("done!\n"); //TODO debug remove
	
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

/**
 * Generates A-B cycles from R and stores them as an array of pointers to
 * tour objects in cycles. Cycles is expected to already be populated by
 * allocated memory (that is, this function will not allocated or deallocate
 * any memory by itself. Consider this a type of memory pooling).
 * The process of generating A-B cycles is described in depth by the original
 * EAX paper, but I will summarize briefly: start on a node, then generate a
 * subcycle by alternating from edges on TourA and TourB.
 * R : the graph object that all these edges belong to
 * cycles : (pass by reference) an array of already allocated cycle structures for
 *          us to use.
 * side-effects : modifies cycle array
 * returns : (int) number of A-B cycles taht were generated
 */
int generateABCycles(const graph_t* const R, tour_t** cycles /*byref*/)
{
	// TODO: stub
	return 0;
}
