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
	DPRINTF("\ncreating empty graph (of size %i)...", tourSize);
	R = (graph_t*)malloc(sizeof(graph_t)); // TODO: FREE_MEM: don't forget to free(R) when you're done using it
	R->size = tourSize;
	for (i=0; i < tourSize; i++)
	{
		R->node[i] = (node_t*)malloc(sizeof(node_t)); // TODO: FREE_MEM: don't forget to free each node when freeing R
		R->node[i]->size = 0;
		
		R->node[i]->id = i; // Each node in the graph is numbered the same as the master city list
	}
	DPRINTF("done!\n");
	
	// visit each city in both tours, adding the edges to R
	// node[i] is the same city as tA->city[i], but feel free to look at the id's to be sure
	DPRINTF("populating the graph (merging the edges)...");
	for (i=1; i < tourSize-1; i++)
	{
		// tourA's cities
		curNode = R->node[tA->city[i]->id]; // grab the node representing tour A's city[i]
		curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
		curNode->edge[curNode->size++] = R->node[tA->city[i-1]->id]; // previous node in tourA
		curNode->edge[curNode->size++] = R->node[tA->city[i+1]->id]; // next node in tourA
		
		// tourB's cities
		curNode = R->node[tB->city[i]->id]; // grab the node representing tour B's city[i]
		curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 1; // these edges belong to B
		curNode->edge[curNode->size++] = R->node[tB->city[(i-1+tourSize)%tourSize]->id]; // previous node in tourB
		curNode->edge[curNode->size++] = R->node[tB->city[(i+1)%tourSize]->id]; // next node in tourB
	}
	DPRINTF("Special cases..."); //TODO debug remove
	// now handle the special case of the first and last nodes in the tour, which link back around (each tour is a cycle)
	// first city in each tour
	curNode = R->node[tA->city[0]->id]; // grab the node representing tour A's city[0]
	curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
	curNode->edge[curNode->size++] = R->node[tA->city[tourSize-1]->id]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[tA->city[1]->id]; // next node in tourA
	// tourB's cities
	curNode = R->node[tB->city[0]->id]; // grab the node representing tour B's city[0]
	curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 1; // these edges belong to B
	curNode->edge[curNode->size++] = R->node[tB->city[tourSize-1]->id]; // previous node in tourB
	curNode->edge[curNode->size++] = R->node[tB->city[1]->id]; // next node in tourB
	
	// last city in each tour
	curNode = R->node[tA->city[tourSize-1]->id]; // grab the node representing tour A's last city
	curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
	curNode->edge[curNode->size++] = R->node[tA->city[tourSize-2]->id]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[tA->city[0]->id]; // next node in tourA
	// tourB's cities
	curNode = R->node[tB->city[tourSize-1]->id]; // grab the node representing tour B's last city
	curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 1; // these edges belong to B
	curNode->edge[curNode->size++] = R->node[tB->city[tourSize-2]->id]; // previous node in tourB
	curNode->edge[curNode->size++] = R->node[tB->city[0]->id]; // next node in tourB
	DPRINTF("done!\n"); //TODO debug remove
	
	// at this point R should be fully populated with all of the edges in tA and tB, so we can return what we calculated
	return R;
} // mergeTours()

/**
 * creates a graph that consists of all of the edges in tour A.
 * tA : pointer to tour A
 * returns : a pointer to the generated graph structure (remember to free this memory when you're finished)
 */
graph_t* createGraph(const tour_t* const tA)
{
	// declarations
	graph_t* R; // the graph we're going to create from the edges in tA, this is also the return value
	node_t* curNode; // current node we're modifying or looking at in the graph
	city_t* curCity; // current city we're looking at
	int i; // loop counter
	int tourSize;
	
	// initializations
	tourSize = tA->size; // size of the tour
	
	// create an empty graph
	DPRINTF("\ncreating empty graph (of size %i)...", tourSize);
	R = (graph_t*)malloc(sizeof(graph_t)); // TODO: FREE_MEM: don't forget to free(R) when you're done using it
	R->size = tourSize;
	for (i=0; i < tourSize; i++)
	{
		R->node[i] = (node_t*)malloc(sizeof(node_t)); // TODO: FREE_MEM: don't forget to free each node when freeing R
		R->node[i]->size = 0;
		
		R->node[i]->id = i; // Each node in the graph is numbered the same as the master city list
	}
	DPRINTF("done!\n");
	
	// visit each city in both tours, adding the edges to R
	// node[i] is the same city as tA->city[i], but feel free to look at the id's to be sure
	DPRINTF("populating the graph (merging the edges)...");
	for (i=1; i < tourSize-1; i++)
	{
		// tourA's cities
		curNode = R->node[tA->city[i]->id]; // grab the node representing tour A's city[i]
		curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
		curNode->edge[curNode->size++] = R->node[tA->city[i-1]->id]; // previous node in tourA
		curNode->edge[curNode->size++] = R->node[tA->city[i+1]->id]; // next node in tourA
	}
	DPRINTF("Special cases..."); //TODO debug remove
	// now handle the special case of the first and last nodes in the tour, which link back around (each tour is a cycle)
	// first city in each tour
	curNode = R->node[tA->city[0]->id]; // grab the node representing tour A's city[0]
	curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
	curNode->edge[curNode->size++] = R->node[tA->city[tourSize-1]->id]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[tA->city[1]->id]; // next node in tourA
	
	// last city in each tour
	curNode = R->node[tA->city[tourSize-1]->id]; // grab the node representing tour A's last city
	curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
	curNode->edge[curNode->size++] = R->node[tA->city[tourSize-2]->id]; // previous node in tourA
	curNode->edge[curNode->size++] = R->node[tA->city[0]->id]; // next node in tourA
	DPRINTF("done!\n"); //TODO debug remove
	
	// at this point R should be fully populated with all of the edges in tA, so we can return what we calculated
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
 * Cities : master array of all of the cities in the TSP set. This will not be modified.
 * R : the graph object that all these edges belong to. R will be modified in the 
 *     process. (should be empty basically if all went well)
 * cycles : (pass by reference) an array of already allocated cycle structures for
 *          us to use. Makes no assumptions about the data already in the array,
 *          other than that every index points to an allocated object so that it
 *          can overwrite that object.
 * side-effects : modifies given graph object
 * side-effects : modifies cycle array
 * returns : (int) number of A-B cycles taht were generated
 */
int generateABCycles(const tour_t* const Cities, graph_t* R /*byref*/, tour_t** cycles /*byref*/)
{
	// local declarations
	int visited[MAX_CITIES]; // tracks which cities were visited inside inner AB cycle loop
	int iteration[MAX_CITIES]; // keeps track of which iteration of the inner AB cycle loop that each index was encountered FOR THE FIRST TIME see NOTE
	int currentIteration; // How many iterations have occurred in this AB cycle generation
	// NOTE: it is possible for a vertex to be visited more than once per AB cycle, this would occur if the first time that vertex was encountered
	//       there was an odd number of cities in the current AB cycle, therefore the algorithm had to continue until the cycle was extended to the 
	//       where there is an even number of cities in the cycle. In this case, the second time that city was encountered, we DO NOT update the value
	//       stored in iteration, because we're more interested in the first time that city was encountered. This allows us to calculate the length of
	//       the fixed AB cycle in an O(1) operation. (simple subtraction)
	int size = 0; // size of the cycles array, number of AB cycles
	int panic; // used to exit out of randomly picking a node if there's an infinite loop
	int c[2]; // choice array, used when choosing one of two possible edges on a vertex
	int i,e; // loop counter
	int n; // used for tracking the size of things
	int a,b; // used as temporaries during swapping and other operations
	
	node_t *v0, *v1, *v2; // vertex one and two of the current edge, v0 is the first v1
	//note: if v1==v2 at start of iteration, choose an edge incident to v1
	int edges = R->size*MAX_EDGES/2; // R should have MAX_EDGES/2 edges for every vertex, we half it because it's an undirected graph and don't want duplicates. (NOTE: could still have duplicates if both tours share an edge, this is ok).
	tour_t* curCycle = 0; // current AB cycle we're working on
	int v2i = -1; // stores the index of the edge of v1 that connects to v2. So, v1->edge[v2i] == v2

	// initializations
	v0 = v1 = v2 = 0;

	// generate AB cycles
	while (edges > 0)
	{
		DPRINTF("Generating AB Cycle, edges left: \033[032m%i[\033[0m...\n", edges);
		// pick starting edge
		DPRINTF("Choosing a random vertex...\n");//TODO: debug remove
		// choose a vertex with at least 2 edges (to create a loop)
		v2 = R->node[rand() % R->size];
		panic = 0;
		DPRINTF("first pick: [%i]->s:%i\n", v2->id, v2->size);
		for (;v2->size == 0 && panic<PANIC_EXIT; panic++)
			v2 = R->node[rand() % R->size];
		if (panic >= PANIC_EXIT)
		{
			ERROR_TEXT;
			printf("generateABCycles() :: ERROR : panic_exit : too many iterations (%i >= %i) when trying to pick a vertex that has remaining edges. halting...\n", panic, PANIC_EXIT);
			exit(32);
		}
		DPRINTF("v2:%i...\n", v2->id);//TODO: debug remove

		// set up current cycle
		DPRINTF("next cycle #%i...\n", size);
		curCycle = cycles[size++];
		curCycle->size = 0;
		DPRINTF("Initializing cycle...\n");
		memset((void*)visited, 0, sizeof(visited)); // reset visited nodes for this cycle
		memset((void*)iteration, 0, sizeof(iteration)); // reset the iteration tracking array for this cycle
		currentIteration = 0;
		v0 = v2; // save the starting vertex
		DPRINTF("entering cycle loop...\n"); //TODO: debug remove
		do  // keep alternating until we've made a cycle
		{
			// first, add this edge to the current cycle
			//DPRINTF("initial increment...\n");
			curCycle->city[curCycle->size++] = Cities->city[v2->id];
			if (!visited[v2->id]++) iteration[v2->id] = currentIteration;
			++currentIteration;
			DPRINTF("Adding edge to cycle...\n");//TODO: debug remove
			//DPRINTF("Alternating tour picking new edge...\n");//TODO: debug remove
			v1 = v2; // we need to pick a new v2
			c[2]; // choice1 and an optional choice 2
			c[0] = c[1] = 0; // NOTE: these are off by one indices to speed up boolean checks!
			switch(v1->size) // TODO: this could be moved into the other switch
			{
			case 1: // this should be the case only if we just created a closed loop
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0; // only one edge to pick
				break;
			case 2: // this would be the case when there is only one path left on these verts
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == TOUR_A) ? 2 : 0;
				break;
			case 3: // this would be the case when creating a closed loop, but also another loop
			        // could be on that vertex in another AB cycle
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == TOUR_A) ? 2 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[2] == TOUR_A) ? 3 : c[1];
				break;
			case 4: // this would be the case when two AB cycles can be generated from that vertex
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == TOUR_A) ? 2 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[2] == TOUR_A) ? 3 : c[1];
				c[(c[0] == 1)?1:0] = (v1->tour[3] == TOUR_A) ? 4 : c[1];
				break;
			default:
				ERROR_TEXT;
				printf("ERROR44: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				exit(44);
				break;
			}// calculating choices for the next edge
			if (!c[0])
			{
				ERROR_TEXT;
				printf("ERROR45: no choices for next vertex found v[%i] (%i) HALT\n", v1->id, v1->size);
				exit(45);
			}
			// pick one of the choices if there are more than one, otherwise the only choice
			v2i = c[1] ? c[rand() % 2]-1 : c[0]-1; // subtract one, since they're OBO, see above
			DPRINTF("v2i: %i, c[0]: %i, c[1]: %i\n",v2i, c[0]-1, c[1]-1);
			DPRINTF("v1e0: %i, v1e1: %i, v1e2: %i, v1e3: %i\n", 
				(v1->edge[0]?v1->edge[0]->id:-1), 
				(v1->edge[1]?v1->edge[1]->id:-1), 
				(v1->edge[2]?v1->edge[2]->id:-1), 
				(v1->edge[3]?v1->edge[3]->id:-1));
			v2 = v1->edge[v2i];
			// END PICKING V2
			//DPRINTF("next!\n");
			STRONG_TEXT;
			DPRINTF("next iteration...v1:%i, v2:%i\n", v1->id, v2->id);//TODO: debug remove
			NORMAL_TEXT;

			// remove the edge from the graph
			// undirected graph, so first remove the edge from v1
			--edges;

			// turns out, v2i is always preserved as the edge we last choice from v1
			// so we can just re-use it here!
			REMOVE_EDGE(v1, v2i); // boo-yah for O(1) operations
			// undirected graph, so now we must remove the edge from v2
			//TODO: it may be possible to speed this up by caching the edge index
			switch(v2->size) // using a switch here to avoid iteration
			{
			case 1: // NOTE: this should only be the case if this is the last vertex on the cycle
				REMOVE_EDGE(v2, 0);
				break;
			case 2:
				if (v2->edge[0] == v1 && v2->tour[0] == TOUR_A) // one of two possible edges
					REMOVE_EDGE(v2, 0);
				else
					REMOVE_EDGE(v2, 1);
				break;
			case 3:
				if (v2->edge[0] == v1 && v2->tour[0] == TOUR_A) // three possible edges to check
					REMOVE_EDGE(v2, 0);
				else if (v2->edge[1] == v1 && v2->tour[1] == TOUR_A)
					REMOVE_EDGE(v2, 1);
				else
					REMOVE_EDGE(v2, 2);
				break;
			case 4:
				if (v2->edge[0] == v1 && v2->tour[0] == TOUR_A) // four possible edges to check
					REMOVE_EDGE(v2, 0);
				else if (v2->edge[1] == v1 && v2->tour[1] == TOUR_A)
					REMOVE_EDGE(v2, 1);
				else if (v2->edge[2] == v1 && v2->tour[2] == TOUR_A)
					REMOVE_EDGE(v2, 2);
				else
					REMOVE_EDGE(v2, 3);
				break;
			default:
				ERROR_TEXT;
				printf("ERROR43: invalid # of edges on vertex[%i] (%i) halting...\n", v2->id, v2->size);
				exit(43);
				break;
			}// removing the edge from v2
			
			/////////////////////////////////////////////////////////////////////////////////////////////
			// LOOP UNROLLING : iterate again, but use an edge from TOUR_B
			/////////////////////////////////////////////////////////////////////////////////////////////
			// first, add this edge to the current cycle
			//DPRINTF("initial increment...\n");
			curCycle->city[curCycle->size++] = Cities->city[v2->id];
			if (!visited[v2->id]++) iteration[v2->id] = currentIteration;
			++currentIteration;
			DPRINTF("Adding edge to cycle...\n");//TODO: debug remove
			//DPRINTF("Alternating tour picking new edge...\n");//TODO: debug remove
			v1 = v2; // we need to pick a new v2
			c[0] = c[1] = 0; // NOTE: these are off by one indices to speed up boolean checks!
			switch(v1->size) // TODO: this could be moved into the other switch
			{
			case 1: // this should be the case only if we just created a closed loop
				c[0] = (v1->tour[0] == TOUR_B) ? 1 : 0; // only one edge to pick
				break;
			case 2: // this would be the case when there is only one path left on these verts
				c[0] = (v1->tour[0] == TOUR_B) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == TOUR_B) ? 2 : 0;
				break;
			case 3: // this would be the case when creating a closed loop, but also another loop
			        // could be on that vertex in another AB cycle
				c[0] = (v1->tour[0] == TOUR_B) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == TOUR_B) ? 2 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[2] == TOUR_B) ? 3 : c[1];
				break;
			case 4: // this would be the case when two AB cycles can be generated from that vertex
				c[0] = (v1->tour[0] == TOUR_B) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == TOUR_B) ? 2 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[2] == TOUR_B) ? 3 : c[1];
				c[(c[0] == 1)?1:0] = (v1->tour[3] == TOUR_B) ? 4 : c[1];
				break;
			default:
				ERROR_TEXT;
				printf("ERROR244: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				exit(244);
				break;
			}// calculating choices for the next edge
			if (!c[0])
			{
				ERROR_TEXT;
				printf("ERROR245: no choices for next vertex found v[%i] (%i) HALT\n", v1->id, v1->size);
				exit(245);
			}
			// pick one of the choices if there are more than one, otherwise the only choice
			v2i = c[1] ? c[rand() % 2]-1 : c[0]-1; // subtract one, since they're OBO, see above
			DPRINTF("v2i: %i, c[0]: %i, c[1]: %i\n",v2i, c[0]-1, c[1]-1);
			DPRINTF("v1e0: %i, v1e1: %i, v1e2: %i, v1e3: %i\n", 
				(v1->edge[0]?v1->edge[0]->id:-1), 
				(v1->edge[1]?v1->edge[1]->id:-1), 
				(v1->edge[2]?v1->edge[2]->id:-1), 
				(v1->edge[3]?v1->edge[3]->id:-1));
			v2 = v1->edge[v2i];
			// END PICKING V2
			//DPRINTF("next!\n");
			STRONG_TEXT;
			DPRINTF("next iteration...v1:%i, v2:%i\n", v1->id, v2->id);//TODO: debug remove
			NORMAL_TEXT;

			// remove the edge from the graph
			// undirected graph, so first remove the edge from v1
			--edges;

			// turns out, v2i is always preserved as the edge we last choice from v1
			// so we can just re-use it here!
			REMOVE_EDGE(v1, v2i); // boo-yah for O(1) operations
			// undirected graph, so now we must remove the edge from v2
			//TODO: it may be possible to speed this up by caching the edge index
			switch(v2->size) // using a switch here to avoid iteration
			{
			case 1: // NOTE: this should only be the case if this is the last vertex on the cycle
				REMOVE_EDGE(v2, 0);
				break;
			case 2:
				if (v2->edge[0] == v1 && v2->tour[0] == TOUR_B) // one of two possible edges
					REMOVE_EDGE(v2, 0);
				else
					REMOVE_EDGE(v2, 1);
				break;
			case 3:
				if (v2->edge[0] == v1 && v2->tour[0] == TOUR_B) // three possible edges to check
					REMOVE_EDGE(v2, 0);
				else if (v2->edge[1] == v1 && v2->tour[1] == TOUR_B)
					REMOVE_EDGE(v2, 1);
				else
					REMOVE_EDGE(v2, 2);
				break;
			case 4:
				if (v2->edge[0] == v1 && v2->tour[0] == TOUR_B) // four possible edges to check
					REMOVE_EDGE(v2, 0);
				else if (v2->edge[1] == v1 && v2->tour[1] == TOUR_B)
					REMOVE_EDGE(v2, 1);
				else if (v2->edge[2] == v1 && v2->tour[2] == TOUR_B)
					REMOVE_EDGE(v2, 2);
				else
					REMOVE_EDGE(v2, 3);
				break;
			default:
				ERROR_TEXT;
				printf("ERROR243: invalid # of edges on vertex[%i] (%i) halting...\n", v2->id, v2->size);
				exit(243);
				break;
			}// removing the edge from v2
		} while ((visited[v2->id]==0 || ((currentIteration-iteration[v2->id])%2==1)) && edges > 0); // while creating a cycle
		//TODO: REMOVE, print iteration array
		DPRINTF("ITERATIONS : ");
		for (i=0; i < Cities->size; i++)
			DPRINTF("%i, ", iteration[i]);
		DPRINTF("\n");
		DPRINTF("Ab cycle generated: ");
		STRONG_TEXT;
		for (n=0; n < curCycle->size; n++)
			DPRINTF("->[%i]", curCycle->city[n]->id);
		NORMAL_TEXT;
		DPRINTF("\n");

		// check to see if we've made a cycle with a tail
		if (v2 != v0)
		{
			OOPS_TEXT;
			DPRINTF("Cycle with tail generated, removing tail...\n");
			NORMAL_TEXT;

			// flip the entire current cycle, this makes it easier to remove cities from it
			a = 0;
			b = curCycle->size;
			city_t* t;
			for (; a < --b; a++)// decrement b, swap if a<b, exit if a>=b, increment a
			{
				t = curCycle->city[a];
				curCycle->city[a] = curCycle->city[b];
				curCycle->city[b] = t;
			}
			// Print out the reversed cycle for verification
			DPRINTF("Ab cycle reversed: ");
			STRONG_TEXT;
			for (n=0; n < curCycle->size; n++)
				DPRINTF("->[%i]", curCycle->city[n]->id);
			DPRINTF("\n");
			NORMAL_TEXT;

			// now keep removing the last city in the cycle and adding that edge back to R
			// until v2 represents the last city in the cycle
			//v1 = v0; // move back to the original vertex
			v0 = v2; // the "new" original vertex is the actual end of the cycle
			v2 = R->node[curCycle->city[curCycle->size-1]->id];
			DPRINTF("v0,v1,v2: %i,%i,%i\n", v0->id, v1->id, v2->id);
			int curTour; // the current tour of the edge we're trying to restore. alternates
			curTour = TOUR_A; // every path starts with tourA, then we flipped it,
			                  // meaning that the tail edge of curCycle must be from tourA
			while (v0->id != curCycle->city[--curCycle->size]->id)
			{
				DPRINTF("curCycle->city[%i]->id = %i\n", curCycle->size, curCycle->city[curCycle->size]->id);
				// grab the new tip of the tail, and restore the edge
				v1 = v2;
				v2 = R->node[curCycle->city[curCycle->size-1]->id];
				DPRINTF("Restoring edge: %i->%i\n", v1->id, v2->id);
				// we need to add that edge back to the graph, but the edge
				// actualy still exists in the graph, we just 'removed' it by modifying its
				// position in the arrays and changing the size of the arrays.
				// what we need to do now is find it again, move it back, and modify the sizes again
				
				// NOTE: about the switching:
				//   - V1 should always have 1 or 3 edges
				//   - V2 should always have 0, 1, or 2 edges
				//   logic:
				//   - V1 can't have 0 edges, or adding an edge would create a dead end
				//   - V1 can't have 2 edges, or adding an edge, then removing 2 later makes ^
				//   - neither V can have 4 edges, or adding an edge creates too many edges
				//   - V2 CAN have 1 edge, but only when it's the terminal V in the last edge to be restored
				//   - if v2 is not the terminal edge, v2 must have 0 or 2 edges, as both edges
				//     will be resotred in this process.
				//  summary: all vertices after this process is over must have an even # of edges

				//TODO: it may be possible to speed this up by caching the edge index
				switch(v1->size) // I'm using a switch here to avoid iteration
				{
				case 1:
					if (v1->edge[1] == v2 && v1->tour[1] == curTour)
						RESTORE_EDGE(v1, 1);
					else if (v1->edge[2] == v2 && v1->tour[2] == curTour)
						RESTORE_EDGE(v1, 2);
					else
						RESTORE_EDGE(v1, 3);
					break;
				case 3:
					RESTORE_EDGE(v1, 3);
					break;
				default:
					ERROR_TEXT;
					printf("ERROR53: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
					exit(53);
					break;
				}// removing edge from v1 */
				// undirected graph, so now we must remove the edge from v2
				//TODO: it may be possible to speed this up by caching the edge index
				switch(v2->size) // using a switch here to avoid iteration
				{
				case 0:
					if (v2->edge[0] == v1 && v2->tour[0] == curTour)
						RESTORE_EDGE(v2, 0);
					else if (v2->edge[1] == v1 && v2->tour[1] == curTour)
						RESTORE_EDGE(v2, 1);
					else if (v2->edge[2] == v1 && v2->tour[2] == curTour)
						RESTORE_EDGE(v2, 2);
					else
						RESTORE_EDGE(v2, 3);
					break;
				case 1: // terminal V edge case
					if (v2->edge[1] == v1 && v2->tour[1] == curTour)
						RESTORE_EDGE(v2, 1);
					else if (v2->edge[2] == v1 && v2->tour[2] == curTour)
						RESTORE_EDGE(v2, 2);
					else
						RESTORE_EDGE(v2, 3);
					break;
				case 2:
					if (v2->edge[2] == v1 && v2->tour[2] == curTour)
						RESTORE_EDGE(v2, 2);
					else
						RESTORE_EDGE(v2, 3);
					break;
				default:
					ERROR_TEXT;
					printf("ERROR54: invalid # of edges on vertex[%i] (%i) halting...\n", v2->id, v2->size);
					exit(54);
					break;
				}// restoring edge to v2

				// we're done restoring the edge
				++edges;

				// alternate tours
				curTour = (curTour+1)%2;
			}// while fixing the cycle
			++curCycle->size; // have to restore the last node onto the list

			DPRINTF("Ab cycle as fixed: ");
			//int n;
			STRONG_TEXT;
			for (n=0; n < curCycle->size; n++)
				DPRINTF("->[%i]", curCycle->city[n]->id);
			DPRINTF("\n");
			NORMAL_TEXT;
			// tell the next iteration to start on the problematic vertex
			v1 = v2;
		}// (end) if we needed to fix the cycle
		else // closed loop created normally
		{
			//v1 = v2 = 0; //TODO: i don't think this is actually necessary.
			DPRINTF("No fixing necessary (no 'tail' on cycle)\n.");
		}

		DPRINTF("next AB Cycle (back to top of outer while)\n"); //TODO: debug remove
		DPRINTF("\nGraph R contains %i nodes: \n", R->size);
		for (i=0; i < R->size; i++)
		{
			DPRINTF("%04i [id:\033[32m%04i\033[0m] -> %i edges: ", i, R->node[i]->id, R->node[i]->size);
			for (e=0; e < R->node[i]->size; e++)
				DPRINTF((e>0) ? ", [\033[32m%04i\033[0m:t%01i]" : "[%04i:t%01i]", R->node[i]->edge[e]->id, R->node[i]->tour[e]);
			DPRINTF("\n");
		}
	}// while R has edges left

	return size;
}

/**
 * generates an e-set from the given AB-cycles by choosing an AB cycle for inclusion
 * in the E-set if the AB-cycle is at least 4 cities in length with a .5 probability.
 * Cities : master array of all cities
 * cycles : (pass by reference) an array of ABcycles, this array is modified into an E-set
 * nCycles : (int) number of cycles in the ABcycles array
 * returns : the number of ABcycles in the E-set
 */
int generateESetRAND(const tour_t* const Cities, tour_t** cycles /*byref*/, int nCycles)
{
	int size = nCycles; // contains the number of AB cycles in the E-set
	int curCycle = 0; // current ABcycle
	float r; // random number. if >=0.5 then the cycle is included in the E-set
	
	// iterate until every ABcycle has been examined
	while (curCycle < size)
	{
		// only roll the dice if we care about this ABcycle
		r = 0;
		if (cycles[curCycle]->size > 2)
			r = frand();
			
		// either remove the ABcycle from the E-set, or move on to examine the next ABcycle
		if (r < 0.5) // remove cycle from E-set
		{
			//TODO: DEBUG REMOVE outputting tour that was removed
			DPRINTF("(r:%f)removing Cycle[%i]: [%i]", r, curCycle, cycles[curCycle]->city[0]->id);
			int a;
			for (a=1; a < cycles[curCycle]->size; a++)
				DPRINTF(", [%i]", cycles[curCycle]->city[a]->id);
			DPRINTF("\n");
			// remove the cycle
			cycles[curCycle] = cycles[--size];
		}// if remove the cycle
		else
		{
			//TODO: DEBUG REMOVE outputting tour that was removed
			DPRINTF("(r:%f)allowing Cycle[%i]: [%i]", r, curCycle, cycles[curCycle]->city[0]->id);
			int a;
			for (a=1; a < cycles[curCycle]->size; a++)
				DPRINTF(", [%i]", cycles[curCycle]->city[a]->id);
			DPRINTF("\n");
			// allow this cycle, examin the next one
			++curCycle;
		}// else allow the cycle
	}// while examining every cycle
	
	// return the number of AB cycles in the E-set
	return size;
}// generateESetRAND()

/**
 * Applies an eset to a tour, generating an intermediate tour with disjoint
 * sub-cycles. If the edge in the cycle belongs to tourA, the edge is removed. if the edge
 * in the cycle belongs to tourB, the edge is added.
 * T : (byref) the intermediate to create (this is "tourA" represented as a graph)
 * E : (byref) the E-set
 * nCycles : number of cycles in the E-set
 * returns : the number of disjoint cycles in the graph
 */
int applyESet(graph_t* T /*byref*/, tour_t** E /*byref*/, int nCycles)
{
	int e = 0; // current ab cycle
	int vi = 0; // current vertex in the current cycle
	
	// iterate over every cycle in the E-set, removing or adding edges as appropriate
	for (e = 0; e < nCycles; e++)
	{
		tour_t* curCycle = E[e];
		// iterate over every city in the current cycle, removing or adding edges as appropriate
		for (vi = 0; vi < curCycle->size; vi++)
		{
			// find the edge
			node_t* v1 = T->node[curCycle->city[vi]->id]; // current node in the cycle
			node_t* v2 = T->node[curCycle->city[(vi+1)%curCycle->size]->id]; // next node in the cycle
			
			// remove or add the edge to v1
			int removing = 1; // keep track for v2 whether we removed or added
			switch(v1->size)
			{
			case 0: // no edges on v1, have to be adding the edge
				removing = 0;
				v1->edge[v1->size++] = v2;
				break;
			case 1: // only one edge to check
				if (v1->edge[0] == v2)
				{
					--v1->size; // last node, safe to decrement
				}
				else // must be adding
				{
					removing = 0;
					v1->edge[v1->size++] = v2;
				}
				break;
			case 2: // two edges to check
				if (v1->edge[0] == v2)
				{
					REMOVE_EDGE(v1, 0);
				}
				else if (v1->edge[1] == v2)
				{
					--v1->size; // last node, safe to decrement
				}
				else // must be adding
				{
					removing = 0;
					v1->edge[v1->size++] = v2;
				}
				break;
			case 3: // three edges to check
				if (v1->edge[0] == v2)
				{
					REMOVE_EDGE(v1, 0);
				}
				else if (v1->edge[1] == v2)
				{
					REMOVE_EDGE(v1, 1);
				}
				else if (v1->edge[2] == v2)
				{
					--v1->size; // last node, safe to decrement
				}
				else // must be adding
				{
					removing = 0;
					v1->edge[v1->size++] = v2;
				}
				break;
			case 4: // four edges to check
				if (v1->edge[0] == v2)
				{
					REMOVE_EDGE(v1, 0);
				}
				else if (v1->edge[1] == v2)
				{
					REMOVE_EDGE(v1, 1);
				}
				else if (v1->edge[2] == v2)
				{
					REMOVE_EDGE(v1, 2);
				}
				else if (v1->edge[3] == v2)
				{
					--v1->size; // last node, safe to decrement
				}
				else // must be adding
				{
					removing = 0;
					v1->edge[v1->size++] = v2;
				}
				break;
			default:
				ERROR_TEXT;
				printf("ERROR339: invalid number of edges on vertex 1 [id:%i,size:%i]\n",
					(v1? v1->id:-1),
					(v1? v1->size:-1));
				break;
			} // switch on the number of edges on v1, to remove or add
			
			// remove or add edge to v2
			if (removing)
			{
				switch(v2->size)
				{
				case 0: // no edges on v2, must've already removed the edge in v1's processing
					ERROR_TEXT;
					printf("ERROR333: No edges on v2, this shouldn't happen. halting\n");
					exit(333);
					break;
				case 1: // only one edge to check, if it's not v1 we have a problem
					if (v2->edge[0] == v1)
					{
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						ERROR_TEXT;
						printf("ERROR334: only edge on v2 wasn't v1. We messed up.\n");
						exit(334);
					}
					break;
				case 2: // two edges to check, if they're not v1 we have a problem
					if (v2->edge[0] == v1)
					{
						REMOVE_EDGE(v2, 0);
					}
					else if (v2->edge[1] == v1)
					{
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						ERROR_TEXT;
						printf("ERROR336: only edge on v2 wasn't v1. We messed up.\n");
						exit(336);
					}
					break;
				case 3: // three edges to check, if they're not v1 we have a problem
					if (v2->edge[0] == v1)
					{
						REMOVE_EDGE(v2, 0);
					}
					else if (v2->edge[1] == v1)
					{
						REMOVE_EDGE(v2, 1);
					}
					else if (v2->edge[2] == v1)
					{
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						ERROR_TEXT;
						printf("ERROR335: only edge on v2 wasn't v1. We messed up.\n");
						exit(335);
					}
					break;
				case 4: // four edges to check, if they're not v1 we have a problem
					if (v2->edge[0] == v1)
					{
						REMOVE_EDGE(v2, 0);
					}
					else if (v2->edge[1] == v1)
					{
						REMOVE_EDGE(v2, 1);
					}
					else if (v2->edge[2] == v1)
					{
						REMOVE_EDGE(v2, 2);
					}
					else if (v2->edge[3] == v1)
					{
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						ERROR_TEXT;
						printf("ERROR337: only edge on v2 wasn't v1. We messed up.\n");
						exit(335);
					}
					break;
				default:
					ERROR_TEXT;
					printf("ERROR338: invalid number of edges on vertex 2 [id:%i,size:%i]\n",
						(v2? v2->id:-1),
						(v2? v2->size:-1));
					break;
				}// switch on the number of edges on v2, to remove or add
			}
			else
			{
				DPRINTF("adding edge to v2...\n");
				v2->edge[v2->size++] = v1;
			}// else we're adding an edge
		}// for each city in the current cycle
	}// for each cycle
	
	// iterate over the graph, finding the disjoint cycles
	int visited[MAX_CITIES];
	memset((void*)visited, 0, sizeof(visited)); // reset visited nodes for this cycle
	// start at the first node, traverse the graph, keeping track of which nodes belong to which cycles
	int iteration = 1; // this is the current disjoint cycle number we're looking at
	int i; // don't confuse with iteration, this is just a generic loop counter
	DPRINTF("tracking disjoint cycles...\n");
	do
	{
		// grab the first unvisited node
		DPRINTF("grabbing first unvisited node: ");
		for (i=0; i < T->size; i++)
			if (!visited[i])
				break;
		DPRINTF("%i\n", i);
		node_t* curNode, *startingNode, *lastNode, *tempNode;
		curNode = startingNode = T->node[i];
		lastNode = tempNode = 0; // tempNode for swapping, lastNode was previous node in the cycle
		do
		{
			DPRINTF("next node...\n");
			visited[curNode->id] = iteration;
			tempNode = curNode;
			curNode = curNode->edge[curNode->edge[0]==lastNode?1:0]; // make sure we don't back-track
			lastNode = tempNode;
		} while (curNode != startingNode);
		++iteration;
		DPRINTF("Checking visited array.\n");
		///////////////////////////////////////////////////////////////////////
		// check visited array
		// yes, I know this looks like assembly programming but I think it's fast, and it makes sense to me :P
		for (i=-1; i+1 < T->size; i++)
			if (!visited[i+1])
				{DPRINTF("unvisited[%i]!\n", i);break;} // found an unvisited node, keep iterating
		if (!visited[i]) // keep iterating
			continue;
		break;// if we made it this far, then all nodes were visited
	} while (1);
	
	return iteration-1; //TODO: return number of disjoint cycles
}// applyESet()
