#include "include/eax.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

void INIT_EDGE(edge_t* E, node_t* V1, node_t* V2, int C)
{
	//edge_t* _E = E;
	E->v1 = V1;
	E->v2 = V2;
	E->cycle = C;
	E->cost = lookup_distance(V1->id, V2->id);
	//DPRINTF("(no inline):looked up distance: %f\n", lookup_distance(V1->id, V2->id));
#if PRINT_EDGE_INIT
	DPRINTF("(no inline):initialized edge = {%i -> %i : i%i : c%f}\n", V1?E->v1->id:-1, V2?E->v2->id:-1, E->cycle, E->cost);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * merges two tours together,
 * replacing e1 and e2 with new edges e3 and e4
 * and then updates the graph
 * T : IN: graph structure with all edges OUT: updated graph structure
 * A : IN: tourA OUT: the merged tour
 * B : IN: tourB OUT: the merged tour
 * e1 : edge 1 belongs to tourA, will be removed
 * e2 : edge 2 belongs to tourB, will be removed, e2.v1 is e3.v2 or e4.v2, and e2.v2 is the other
 * e3 : edge 3, e3.v1 belongs to A (e3.v1 == e1.v1), e3.v2 belongs to B, will be added
 * e4 : edge 4, e4.v1 belongs to A (e4.v1 == e1.v2), e4.v2 belongs to B, will be added
 */
void mergeSubTours(graph_t* T, tour_t* A, tour_t* B, edge_t* e1, edge_t* e2, edge_t* e3, edge_t* e4)
{
	int a, b; // positions in cycles A and B of where e3 or e4 connect
	int e3first; // whether e3 was the connecting edge (first new edge found) or closing edge (last)
	tour_t tempT; // a temporary tour structure to hold A
	tour_t* tempA = &tempT; // made into a pointer for easier code
	memcpy(tempA, A, sizeOfTour(A)); // copy over A
	
	 // find e1 in A
	for(a=0; a < tempA->size; a++)
	{
		city_t* curCity = tempA->city[a];
		node_t* v1 = T->node[curCity->id];
		if (v1 == e1->v1)
		{ // found the edge
			if (T->node[tempA->city[a+1]->id] != e1->v2)
			{
				ERROR_TEXT;
				DPRINTF("Found v1 but not v2 in A. a:%i, v1:%i\n", a, v1->id);
				terminate_program(190);
			}// error
			break;
		}// found e1
	}// while looking for e1
	if (a>=tempA->size)
	{
		ERROR_TEXT;
		DPRINTF("Failed to find e1 in A.\n");
		terminate_program(191);
	}// error
	
	// now we need to find e2 in B
	for(b=0; b < B->size; b++)
	{
		city_t* curCity = B->city[b];
		node_t* v1 = T->node[curCity->id];
		if (v1 == e2->v1)
		{ // found the edge
			if (T->node[B->city[b+1]->id] != e2->v2)
			{
				ERROR_TEXT;
				DPRINTF("Found v1 but not v2 in B. b:%i, v1:%i\n", b, v1->id);
				terminate_program(190);
			}// error
			break;
		}// found e2
	}// while looking for e2
	if (b>=B->size)
	{
		ERROR_TEXT;
		DPRINTF("Failed to find e2 in B.\n");
		terminate_program(191);
	}// error
#if PRINT_MERGE_SUB_TOURS
	DPRINTF("a%i b%i : av%i bv%i\n", a, b, A->city[a]->id, B->city[b]->id);
#endif
	
	// move B onto A
	// depending on which of the two choices for merging was chosen, we need to start
	// at b+1 and go all the way to b (looping around the end) addign all those nodes, or
	// start at b and go backwards all the way to b+1 (looping around) adding the nodes
	A->size = a+1; // make A stop at a (remember oldA is preserved in tempA) 
	b%=B->size-1; // we ignore the last node in B for our purposes, so if b was this node, make it the first one instead
	// graph fixing
	// connect v1 to v3 and v2 to v4
	node_t* v1 = e1->v1; // A v1
	node_t* v2 = e1->v2; // A v2
	node_t* v3 = e2->v1; // B v1
	node_t* v4 = e2->v2; // B v2
	// calculate the edge we'll be replacing
	int v1e = (v1->edge[0] == v2) ? 0 : 1;
	int v2e = (v2->edge[0] == v1) ? 0 : 1;
	int v3e = (v3->edge[0] == v4) ? 0 : 1;
	int v4e = (v4->edge[0] == v3) ? 0 : 1;
#if PRINT_MERGE_SUB_TOURS
	DPRINTF("BEFORE: v1[%i]e%i->[%i] -> v2[%i]e%i->[%i] AND v3[%i]e%i->[%i] -> v4[%i]e%i->[%i]\n", 
			v1->id, v1e, v1->edge[v1e]->id, 
			v2->id, v2e, v2->edge[v2e]->id,
			v3->id, v3e, v3->edge[v3e]->id,
			v4->id, v4e, v4->edge[v4e]->id);
#endif
	if (e3->v2 == v4) // counting from b+1 back around to b
	{
		// replace the edges
		v1->edge[v1e] = v4;
		v2->edge[v2e] = v3;
		v3->edge[v3e] = v2;
		v4->edge[v4e] = v1;
#if PRINT_MERGE_SUB_TOURS
		DPRINTF("AFTER(cx): v1[%i]e%i->[%i] -> v4[%i]e%i->[%i] AND v2[%i]e%i->[%i] -> v3[%i]e%i->[%i]\n", 
			v1->id, v1e, v1->edge[v1e]->id, 
			v4->id, v4e, v4->edge[v4e]->id,
			v2->id, v2e, v2->edge[v2e]->id,
			v3->id, v3e, v3->edge[v3e]->id);
#endif
		
		// move B onto A
		int t = b+1;
		A->city[A->size++] = B->city[t];
		while (t != b)
		{
			++t;
			t %= B->size-1; // loop back around, ignoring the last node in the cycle because it is the first
			A->city[A->size++] = B->city[t];
		}// while we haven't looped back to b
	}// if counting from b+1 back around to b
	else // counting from b down around to b+1
	{
		// replace the edges
		v1->edge[v1e] = v3;
		v2->edge[v2e] = v4;
		v3->edge[v3e] = v1;
		v4->edge[v4e] = v2;
#if PRINT_MERGE_SUB_TOURS
		DPRINTF("AFTER(nm): v1[%i]e%i->[%i] -> v3[%i]e%i->[%i] AND v2[%i]e%i->[%i] -> v4[%i]e%i->[%i]\n", 
			v1->id, v1e, v1->edge[v1e]->id, 
			v3->id, v3e, v3->edge[v3e]->id,
			v2->id, v2e, v2->edge[v2e]->id,
			v4->id, v4e, v4->edge[v4e]->id);
#endif
		
		// move B onto A
		int t = b;
		if (t < 1) t = B->size-1;
		A->city[A->size++] = B->city[t];
		while (t != b+1)
		{
			--t;
			if (t < 1) t = B->size-1; // remember to ignore the last node
			A->city[A->size++] = B->city[t];
		}// while we haven't looped back to b+1
	}// else counting from b down around to b+1
	
	// move the rest of A back onto A (from tempA)
	for (++a; a < tempA->size; a++)
	{
		A->city[A->size++] = tempA->city[a];
	}
	
#if PRINT_MERGE_SUB_TOURS
#if PRINT_INTERMEDIATE_INFO
	DPRINTF("A: ");
	int n;
	STRONG_TEXT;
	for (n=0; n < tempA->size; n++)
	{
		DPRINTF("--> [%i]", tempA->city[n]->id);
	}
	DPRINTF("\n");
	NORMAL_TEXT;
	
	DPRINTF("B: ");
	STRONG_TEXT;
	for (n=0; n < B->size; n++)
	{
		DPRINTF("--> [%i]", B->city[n]->id);
	}
	DPRINTF("\n");
	NORMAL_TEXT;
	
	DPRINTF("Merged cycle: ");
	STRONG_TEXT;
	for (n=0; n < A->size; n++)
	{
		DPRINTF("--> [%i]", A->city[n]->id);
	}
	DPRINTF("\n");
	NORMAL_TEXT;
#endif
#endif
}// mergeSubTours()

/**
 * creates a graph that consists of all of the edges in tour A and all of the edges in tour B
 * R : IN: allocated graph object OUT: the generated graph
 * tA : pointer to tour A
 * tB : pointer to tour B
 */
void mergeTours(graph_t* R, const tour_t* const tA, const tour_t* const tB) 
{
	// declarations
	//graph_t* R; // the graph we're going to create by combining tA and tB, this is also the return value
	node_t* curNode; // current node we're modifying or looking at in the graph
	city_t* curCity; // current city we're looking at
	int i; // loop counter
	int tourSize;
	
	// initializations
	tourSize = tA->size; // size of the tour
	
	// create an empty graph
#if PRINT_MISC
	DPRINTF("\ncreating empty graph (of size %i)...", tourSize);
#endif
	//R = (graph_t*)malloc(sizeof(graph_t)); // TODO: FREE_MEM: don't forget to free(R) when you're done using it
	R->size = tourSize;
	for (i=0; i < tourSize; i++)
	{
		//R->node[i] = (node_t*)malloc(sizeof(node_t)); // TODO: FREE_MEM: don't forget to free each node when freeing R
		R->node[i]->size = 0;
		
		R->node[i]->id = i; // Each node in the graph is numbered the same as the master city list
	}
#if PRINT_MISC
	DPRINTF("done!\n");
#endif
	
	// visit each city in both tours, adding the edges to R
	// node[i] is the same city as tA->city[i], but feel free to look at the id's to be sure
#if PRINT_MISC
	DPRINTF("populating the graph (merging the edges)...");
#endif
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
#if PRINT_MISC
	DPRINTF("Special cases..."); 
#endif
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
#if PRINT_MISC
	DPRINTF("done!\n"); // debug 
#endif
	
	// at this point R should be fully populated with all of the edges in tA and tB, so we can return what we calculated
	//return R;
} // mergeTours()

/**
 * creates a graph that consists of all of the edges in tour A.
 * R : IN: allocated graph object OUT: the generated graph
 * tA : pointer to tour A
 */
graph_t* createGraph(graph_t* R, const tour_t* const tA)
{
	// declarations
	//graph_t* R; // the graph we're going to create from the edges in tA, this is also the return value
	node_t* curNode; // current node we're modifying or looking at in the graph
	city_t* curCity; // current city we're looking at
	int i; // loop counter
	int tourSize;
	
	// initializations
	tourSize = tA->size; // size of the tour
	
	// create an empty graph
#if PRINT_MISC
	DPRINTF("\ncreating empty graph (of size %i)...", tourSize);
#endif
	//R = (graph_t*)malloc(sizeof(graph_t)); // TODO: FREE_MEM: don't forget to free(R) when you're done using it
	R->size = tourSize;
	for (i=0; i < tourSize; i++)
	{
		//R->node[i] = (node_t*)malloc(sizeof(node_t)); // TODO: FREE_MEM: don't forget to free each node when freeing R
		R->node[i]->size = 0;
		
		R->node[i]->id = i; // Each node in the graph is numbered the same as the master city list
	}
#if PRINT_MISC
	DPRINTF("done!\n");
#endif
	
	// visit each city in both tours, adding the edges to R
	// node[i] is the same city as tA->city[i], but feel free to look at the id's to be sure
#if PRINT_MISC
	DPRINTF("populating the graph (merging the edges)...");
#endif
	for (i=1; i < tourSize-1; i++)
	{
		// tourA's cities
		curNode = R->node[tA->city[i]->id]; // grab the node representing tour A's city[i]
		curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
		curNode->edge[curNode->size++] = R->node[tA->city[i-1]->id]; // previous node in tourA
		curNode->edge[curNode->size++] = R->node[tA->city[i+1]->id]; // next node in tourA
	}
#if PRINT_MISC
	DPRINTF("Special cases..."); // debug 
#endif
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
#if PRINT_MISC
	DPRINTF("done!\n"); // debug 
#endif
	
	// at this point R should be fully populated with all of the edges in tA, so we can return what we calculated
	return R;
} // createGraph()

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
 * memory_chunk : a pointer to the beginning of a large chunk memory that we can use for the cycles.
 * CitiesA, CitiesB : master array of all of the cities in the TSP set. This will not be modified. Cities are from A and B.
 * R : the graph object that all these edges belong to. R will be modified in the 
 *     process. (should be empty basically if all went well)
 * cycles : (pass by reference) an array of already allocated cycle structures for
 *          us to use. this is an array of pointers that will be made to point to
 *          sections inside of the memory_chunk
 * side-effects : modifies given graph object
 * side-effects : modifies cycle array
 * returns : (int) number of A-B cycles taht were generated
 */
int generateABCycles(char* memory_chunk, const tour_t* const CitiesA, const tour_t* const CitiesB, graph_t* R /*byref*/, tour_t** cycles /*byref*/)
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
	tour_t* curCycle = (tour_t*)memory_chunk; // current AB cycle we're working on, start at the beginning
	int v2i = -1; // stores the index of the edge of v1 that connects to v2. So, v1->edge[v2i] == v2

	// initializations
	v0 = v1 = v2 = 0;

	// generate AB cycles
	while (edges > 0)
	{
		cycles[size] = curCycle;
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("Generating AB Cycle, edges left: \033[032m%i[\033[0m...\n", edges);
		// pick starting edge
		DPRINTF("Choosing a random vertex...\n");//TODO: debug remove
#endif
		// choose a vertex with at least 2 edges (to create a loop)
		v2i = rand() % R->size;
		panic = 0;
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("first pick: [%i]->s:%i\n", v2i, R->node[v2i]->size);
#endif
		for (;R->node[v2i]->size < 2 && panic<PANIC_EXIT; panic++)
			v2i = (v2i+1) % R->size;
		if (panic >= PANIC_EXIT)
		{
			ERROR_TEXT;
			printf("generateABCycles() :: ERROR : panic_exit : too many iterations (%i >= %i) when trying to pick a vertex that has remaining edges. halting...\n", panic, PANIC_EXIT);
			terminate_program(32);
		}
		v2 = R->node[v2i];
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("v2:%i...\n", v2->id);//TODO: debug remove
#endif
		// set up current cycle
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("next cycle #%i...\n", size);
#endif
		curCycle->size = 0;
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("Initializing cycle...\n");
#endif
		memset(visited, 0, MAX_CITIES * sizeof(int)); // reset visited nodes for this cycle
		memset(iteration, 0, MAX_CITIES * sizeof(int)); // reset the iteration tracking array for this cycle
		currentIteration = 0;
		v0 = v2; // save the starting vertex
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("entering cycle loop...\n"); //TODO: debug remove
#endif
		do  // keep alternating until we've made a cycle
		{
			/////////////////////////////////////////////////////////////////////////////////////////////
			// LOOP UNROLLING : TOUR_A
			/////////////////////////////////////////////////////////////////////////////////////////////
			// first, add this edge to the current cycle
			//DPRINTF("initial increment...\n");
			curCycle->city[curCycle->size++] = CitiesB->city[v2->id]; // it's a connection from B, so use CitiesB
			//curCycle->tour[curCycle->size] = TOUR_A;
			if (!visited[v2->id]++) iteration[v2->id] = currentIteration;
			++currentIteration;
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("Adding edge to cycle...\n");//TODO: debug remove
#endif
			//DPRINTF("Alternating tour picking new edge...\n");//TODO: debug remove
			v1 = v2; // we need to pick a new v2
			c[0] = c[1] = 0; // NOTE: these are off by one indices to speed up boolean checks!
			switch(v1->size) // TODO: this could be moved into the other switch
			{
			case 1: // this should be the case only if we just created a closed loop
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0; // only one edge to pick
				break;
			case 2: // this would be the case when there is only one path left on these verts
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[c[0] ? 1 : 0] = (v1->tour[1] == TOUR_A) ? 2 : 0;
				break;
			case 3: // this would be the case when creating a closed loop, but also another loop
			        // could be on that vertex in another AB cycle
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[c[0] ? 1 : 0] = (v1->tour[1] == TOUR_A) ? 2 : 0;
				c[c[0] ? 1 : 0] = (v1->tour[2] == TOUR_A) ? 3 : c[1];
				break;
			case 4: // this would be the case when two AB cycles can be generated from that vertex
				c[0] = (v1->tour[0] == TOUR_A) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[c[0] ? 1 : 0] = (v1->tour[1] == TOUR_A) ? 2 : 0;
				c[c[0] ? 1 : 0] = (v1->tour[2] == TOUR_A) ? 3 : c[1];
				c[c[0] ? 1 : 0] = (v1->tour[3] == TOUR_A) ? 4 : c[1];
				break;
			default:
				ERROR_TEXT;
				printf("ERROR44: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				terminate_program(44);
				break;
			}// calculating choices for the next edge
			if (!c[0])
			{
				ERROR_TEXT;
				printf("ERROR45: no choices for next vertex found v[%i] (%i) HALT\n", v1->id, v1->size);
				terminate_program(45);
			}
			// pick one of the choices if there are more than one, otherwise the only choice
			v2i = c[1] ? c[rand() % 2]-1 : c[0]-1; // subtract one, since they're OBO, see above
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("v2i: %i, c[0]: %i, c[1]: %i\n",v2i, c[0]-1, c[1]-1);
			DPRINTF("v1e0: %i, v1e1: %i, v1e2: %i, v1e3: %i\n", 
				(v1->edge[0]?v1->edge[0]->id:-1), 
				(v1->edge[1]?v1->edge[1]->id:-1), 
				(v1->edge[2]?v1->edge[2]->id:-1), 
				(v1->edge[3]?v1->edge[3]->id:-1));
#endif
			v2 = v1->edge[v2i];
			// END PICKING V2
			//DPRINTF("next!\n");
#if PRINT_GENERATE_AB_CYCLES
			STRONG_TEXT;
			DPRINTF("next iteration...v1:%i, v2:%i\n", v1->id, v2->id);//TODO: debug remove
			NORMAL_TEXT;
#endif

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
				terminate_program(43);
				break;
			}// removing the edge from v2
			
			/////////////////////////////////////////////////////////////////////////////////////////////
			// LOOP UNROLLING : iterate again, but use an edge from TOUR_B
			/////////////////////////////////////////////////////////////////////////////////////////////
			// first, add this edge to the current cycle
			//DPRINTF("initial increment...\n");
			curCycle->city[curCycle->size++] = CitiesA->city[v2->id]; // it's a connection from A, so use A
			//curCycle->tour[curCycle->size] = TOUR_B;
			if (!visited[v2->id]++) iteration[v2->id] = currentIteration;
			++currentIteration;
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("Adding edge to cycle...\n");//TODO: debug remove
#endif
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
				c[c[0] ? 1 : 0] = (v1->tour[1] == TOUR_B) ? 2 : 0;
				break;
			case 3: // this would be the case when creating a closed loop, but also another loop
			        // could be on that vertex in another AB cycle
				c[0] = (v1->tour[0] == TOUR_B) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[c[0] ? 1 : 0] = (v1->tour[1] == TOUR_B) ? 2 : 0;
				c[c[0] ? 1 : 0] = (v1->tour[2] == TOUR_B) ? 3 : c[1];
				break;
			case 4: // this would be the case when two AB cycles can be generated from that vertex
				c[0] = (v1->tour[0] == TOUR_B) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[c[0] ? 1 : 0] = (v1->tour[1] == TOUR_B) ? 2 : 0;
				c[c[0] ? 1 : 0] = (v1->tour[2] == TOUR_B) ? 3 : c[1];
				c[c[0] ? 1 : 0] = (v1->tour[3] == TOUR_B) ? 4 : c[1];
				break;
			default:
				ERROR_TEXT;
				printf("ERROR244: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				terminate_program(244);
				break;
			}// calculating choices for the next edge
			if (!c[0])
			{
				ERROR_TEXT;
				printf("ERROR245: no choices for next vertex found v[%i] (%i) HALT\n", v1->id, v1->size);
				terminate_program(245);
			}
			// pick one of the choices if there are more than one, otherwise the only choice
			v2i = c[1] ? c[rand() % 2]-1 : c[0]-1; // subtract one, since they're OBO, see above
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("v2i: %i, c[0]: %i, c[1]: %i\n",v2i, c[0]-1, c[1]-1);
			DPRINTF("v1e0: %i, v1e1: %i, v1e2: %i, v1e3: %i\n", 
				(v1->edge[0]?v1->edge[0]->id:-1), 
				(v1->edge[1]?v1->edge[1]->id:-1), 
				(v1->edge[2]?v1->edge[2]->id:-1), 
				(v1->edge[3]?v1->edge[3]->id:-1));
#endif
			v2 = v1->edge[v2i];
			// END PICKING V2
#if PRINT_GENERATE_AB_CYCLES
			STRONG_TEXT;
			DPRINTF("next iteration...v1:%i, v2:%i\n", v1->id, v2->id);//TODO: debug remove
			NORMAL_TEXT;
#endif

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
				terminate_program(243);
				break;
			}// removing the edge from v2
		} while ((visited[v2->id]==0 || ((currentIteration-iteration[v2->id])%2==1)) && edges > 0); // while creating a cycle
#if PRINT_CYCLES
#if PRINT_GENERATE_AB_CYCLES
		// print iteration array
		DPRINTF("ITERATIONS : ");
		for (i=0; i < CitiesA->size; i++)
		{
			DPRINTF("%i, ", iteration[i]);
		}
		DPRINTF("\n");
		DPRINTF("Ab cycle generated: (s%i)\n", curCycle?curCycle->size:-1);
		STRONG_TEXT;
		for (n=0; n < curCycle->size; n++)
		{
			DPRINTF("-t%1i-> [%i]\n", curCycle->city[n]->tour, curCycle->city[n]->id);
		}
		DPRINTF("\n");
		NORMAL_TEXT;
#endif
#endif

		// check to see if we've made a cycle with a tail
		if (v2 != v0)
		{
#if PRINT_GENERATE_AB_CYCLES
			OOPS_TEXT;
			DPRINTF("Cycle with tail generated, removing tail...\n");
			NORMAL_TEXT;
#endif

			// flip the entire current cycle, this makes it easier to remove cities from it
			a = 0;
			b = curCycle->size;
			city_t* t;
			int t0 = curCycle->city[1]->tour;
			for (; a < --b; a++)// decrement b, swap if a<b, exit if a>=b, increment a
			{
				t = curCycle->city[a];
				curCycle->city[a] = curCycle->city[b];
				curCycle->city[b] = t;
			}
			// restore the tour values (since they're reversed now
			for (a=0; a < curCycle->size; a++)
			{
				// tours always alternate, the addition before the modulus
				// is to account for whether we started on tourA or tourB
				int id = curCycle->city[a]->id;
				curCycle->city[a] = (((a+(t0==TOUR_A?1:0))%2)==0 ? CitiesA : CitiesB)->city[id];
			}
			// Print out the reversed cycle for verification
#if PRINT_CYCLES
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("Ab cycle reversed: ");
			STRONG_TEXT;
			for (n=0; n < curCycle->size; n++)
			{
				DPRINTF("-t%1i-> [%i]", curCycle->city[n]->id, curCycle->city[n]->id);
			}
			DPRINTF("\n");
			NORMAL_TEXT;
#endif
#endif

			// now keep removing the last city in the cycle and adding that edge back to R
			// until v2 represents the last city in the cycle
			//v1 = v0; // move back to the original vertex
			v0 = v2; // the "new" original vertex is the actual end of the cycle
			v2 = R->node[curCycle->city[curCycle->size-1]->id];
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("v0,v1,v2: %i,%i,%i\n", v0->id, v1->id, v2->id);
#endif
			int curTour; // the current tour of the edge we're trying to restore. alternates
			curTour = TOUR_A; // every path starts with tourA, then we flipped it,
			                  // meaning that the tail edge of curCycle must be from tourA
			while (v0->id != curCycle->city[--curCycle->size]->id)
			{
#if PRINT_GENERATE_AB_CYCLES
				DPRINTF("curCycle->city[%i]->id = %i\n", curCycle->size, curCycle->city[curCycle->size]->id);
#endif
				// grab the new tip of the tail, and restore the edge
				v1 = v2;
				v2 = R->node[curCycle->city[curCycle->size-1]->id];
#if PRINT_GENERATE_AB_CYCLES
				DPRINTF("Restoring edge: %i->%i\n", v1->id, v2->id);
#endif
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
					terminate_program(53);
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
					terminate_program(54);
					break;
				}// restoring edge to v2

				// we're done restoring the edge
				++edges;

				// alternate tours
				curTour = (curTour+1)%2;
			}// while fixing the cycle
			++curCycle->size; // have to restore the last node onto the list

			// tell the next iteration to start on the problematic vertex
			v1 = v2;
		}// (end) if we needed to fix the cycle
		else // closed loop created normally
		{
			//v1 = v2 = 0; //TODO: i don't think this is actually necessary.
#if PRINT_GENERATE_AB_CYCLES
			DPRINTF("No fixing necessary (no 'tail' on cycle)\n.");
#endif
		}
		
		// make the cycle loop back on itself
		curCycle->city[curCycle->size] = ((curCycle->city[curCycle->size-1]->tour==TOUR_A) ? CitiesB : CitiesA)->city[curCycle->city[0]->id];
		++curCycle->size;
		//curCycle->city[curCycle->size++] = curCycle->city[0];
		
#if PRINT_CYCLES
#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("Ab cycle as fixed: ");
		//int n;
		STRONG_TEXT;
		for (n=0; n < curCycle->size; n++)
		{
			//DPRINTF("-t%1i-> [%i]", curCycle->city[n]->tour, curCycle->city[n]->id);
			DPRINTF("-%s-> [%i]", (curCycle->city[n]->tour == TOUR_A ? "A" : "B"), curCycle->city[n]->id);
		}
		DPRINTF("\n");
		NORMAL_TEXT;
#endif
#endif

#if PRINT_GENERATE_AB_CYCLES
		DPRINTF("next AB Cycle (back to top of outer while)\n"); //TODO: debug remove
#if PRINT_GRAPHS
		DPRINTF("\nGraph R contains %i nodes: \n", R->size);
		for (i=0; i < R->size; i++)
		{
			DPRINTF("%04i [id:\033[32m%04i\033[0m] -> %i edges: ", i, R->node[i]->id, R->node[i]->size);
			for (e=0; e < R->node[i]->size; e++)
				printf((e>0) ? ", [\033[32m%04i\033[0m:t%01i]" : "[%04i:t%01i]", R->node[i]->edge[e]->id, R->node[i]->tour[e]);
			DPRINTF("\n");
		}
#endif
#endif
		///////////////////////// E-SET CONSTRUCTION STEP //////////////////////////////////
		// see if the cycle passes the E-Set test, if it does, slide up curCycle,
		// otherwise, just overwrite curCycle in the next iteration
		float r = frand();
		if (curCycle->size > 2 && r > 0.5)
		{	
#if PRINT_GENERATE_ESET
			STRONG_TEXT;
			DPRINTF("Adding cycle to the E-Set (r:%f): ", r);
			dprint_tour(curCycle);
			NORMAL_TEXT;
#endif
			curCycle = (tour_t*)((char*)curCycle + sizeOfTour(curCycle)); // increment the pointer past the end of the current cycle
			++size;
		}
		else
		{
#if PRINT_GENERATE_ESET
			ERROR_TEXT;
			DPRINTF("Removing cycle from the E-Set (r:%f): ", r);
			dprint_tour(curCycle);
			NORMAL_TEXT;
#endif
		}
	}// while R has edges left

	return size;
} // generateABCycles()

/**
 * Applies an eset to a tour, generating an intermediate tour with disjoint
 * sub-cycles. If the edge in the cycle belongs to tourA, the edge is removed. if the edge
 * in the cycle belongs to tourB, the edge is added.
 * Modifies the E-set into a collection of the sub-cycles
 * memory_chunk : giant chunk of memory to put cycles into
 * Cities : master array of all cities
 * T : (byref) IN: tourA as a graph OUT: the intermediate created
 * E : (byref) IN: the E-set OUT: this gets transformed into a collection of the cycles
 * nCycles : number of cycles in the E-set
 * edges : (byref) IN: pre-allocated array of edge structures. The data needn't be specially initialized, but they
 *         do need to be allocated. OUT: All the edges of the graph will be placed into this array.
 * returns : the number of disjoint cycles in the graph
 */
int applyESet(char* memory_chunk, const tour_t* const Cities, graph_t* T /*byref*/, tour_t** E /*byref*/, int nCycles, edge_t* edges /*byref*/)
{
	int e = 0; // current ab cycle
	int vi = 0; // current vertex in the current cycle
	tour_t* curCycle = 0;
	
	// iterate over every cycle in the E-set, removing or adding edges as appropriate
	for (e = 0; e < nCycles; e++)
	{
		curCycle = E[e];
		// iterate over every city in the current cycle, removing or adding edges as appropriate
		for (vi = 0; vi < curCycle->size-1; vi++)
		{
			// find the edge
			node_t* v1 = T->node[curCycle->city[vi]->id]; // current node in the cycle
			node_t* v2 = T->node[curCycle->city[vi+1]->id]; // next node in the cycle
			
			// remove or add the edge to v1
			int removing = curCycle->city[vi+1]->tour == TOUR_A; // keep track for v2 whether we removed or added
			if (removing)
			{
				///////////////////////////////////// TOUR A ////////////////////////////////////////
				switch(v1->size)
				{
				case 0: // no edges on v1, have to be adding the edge
					DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
					terminate_program(80);
					break;
				case 1: // only one edge to check
					if (v1->edge[0] == v2)
					{
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v1)...\n", 
								(v1?v1->id:-1), 
								(v1 && v1->edge[0]?v1->edge[0]->id:-1), 
								(v1?v1->tour[0]:-1));
#endif
						--v1->size; // last node, safe to decrement
					}
					else // adding? wtf
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(81);
					}
					break;
				case 2: // two edges to check
					if (v1->edge[0] == v2)
					{
						REMOVE_EDGE(v1, 0);
					}
					else if (v1->edge[1] == v2)
					{
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v1)...\n", 
								(v1?v1->id:-1), 
								(v1 && v1->edge[1]?v1->edge[1]->id:-1), 
								(v1?v1->tour[1]:-1));
#endif
						--v1->size; // last node, safe to decrement
					}
					else // adding? wtf
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(82);
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
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v1)...\n", 
								(v1?v1->id:-1), 
								(v1 && v1->edge[2]?v1->edge[2]->id:-1), 
								(v1?v1->tour[2]:-1));
#endif
						--v1->size; // last node, safe to decrement
					}
					else // adding? wtf
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(83);
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
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v1)...\n", 
								(v1?v1->id:-1), 
								(v1 && v1->edge[3]?v1->edge[3]->id:-1), 
								(v1?v1->tour[3]:-1));
#endif
						--v1->size; // last node, safe to decrement
					}
					else // adding? wtf
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(84);
					}
					break;
				default:
					ERROR_TEXT;
					printf("ERROR339: invalid number of edges on vertex 1 [id:%i,size:%i]\n",
						(v1? v1->id:-1),
						(v1? v1->size:-1));
					break;
				} // switch on the number of edges on v1, to remove or add
				//////////////////////////////////// TOUR B ////////////////////////////////////////
				switch(v2->size)
				{
				case 0: // no edges on v2, must've already removed the edge in v1's processing
					DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
					terminate_program(333);
					break;
				case 1: // only one edge to check, if it's not v1 we have a problem
					if (v2->edge[0] == v1)
					{
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v2)...\n", 
							(v2?v2->id:-1), 
							(v2 && v2->edge[0]?v2->edge[0]->id:-1), 
							(v2?v2->tour[0]:-1));
#endif
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(334);
					}
					break;
				case 2: // two edges to check, if they're not v1 we have a problem
					if (v2->edge[0] == v1)
					{
						REMOVE_EDGE(v2, 0);
					}
					else if (v2->edge[1] == v1)
					{
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v2)...\n", 
							(v2?v2->id:-1), 
							(v2 && v2->edge[1]?v2->edge[1]->id:-1), 
							(v2?v2->tour[1]:-1));
#endif
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(336);
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
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v2)...\n", 
							(v2?v2->id:-1), 
							(v2 && v2->edge[2]?v2->edge[2]->id:-1), 
							(v2?v2->tour[2]:-1));
#endif
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(335);
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
#if PRINT_APPLY_ESET
						DPRINTF("\033[33mremoving\033[0m edge(v[%i]->v[%i]t%i from graph (final edge v2)...\n", 
							(v2?v2->id:-1), 
							(v2 && v2->edge[3]?v2->edge[3]->id:-1), 
							(v2?v2->tour[3]:-1));
#endif
						--v2->size; // last node, safe to decrement
					}
					else // we messed up
					{
						DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
						terminate_program(335);
					}
					break;
				default:
					DPRINTF("v[%i] -> v[%i], cycle: %it%1i -> %it%1i, removing:%i", 
							v1? v1->id:-1,
							v2? v2->id:-1,
							curCycle->city[vi]?curCycle->city[vi]->id:-1,
							curCycle->city[vi]->id,
							curCycle->city[vi+1]?curCycle->city[vi+1]->id:-1,
							curCycle->city[vi+1]->id,
							removing);
					terminate_program(336);
					break;
				}// switch on the number of edges on v2, to remove or add
			}// if removing
			else
			{
#if PRINT_APPLY_ESET
				DPRINTF("adding edge to v2 [%i]->(%i)...\n", v2->id, v1->id);
#endif
				v2->tour[v2->size] = TOUR_B;
				v2->edge[v2->size++] = v1;
#if PRINT_APPLY_ESET
				DPRINTF("adding edge to v1 [%i]->(%i)...\n", v1->id, v2->id);
#endif
				v1->tour[v1->size] = TOUR_B;
				v1->edge[v1->size++] = v2;
			}// else we're adding an edge
		}// for each city in the current cycle
	}// for each cycle
	
#if PRINT_APPLY_ESET
#if PRINT_GRAPHS
	DPRINTF("\033[32mIntermediate Tour T\033[0m contains (after applying E-set, in applyEset()): \n");
	int i2;
	for (i2=0; i2 < T->size; i2++)
	{
		DPRINTF("%04i [id:%04i] -> edges: ", i2, T->node[i2]->id);
		int e;
		for (e=0; e < T->node[i2]->size; e++)
		{
			DPRINTF((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i2]->edge[e]->id, T->node[i2]->tour[e]);
		}
		DPRINTF("\n");
	}
#endif
#endif
	
	//////////////////////////// Track Disjoint Cycles ///////////////////////////////////
	// iterate over the graph, finding the disjoint cycles
	int visited[MAX_CITIES];
	int curEdge = 0;
	memset((void*)visited, 0, sizeof(visited)); // reset visited nodes for this cycle
	// start at the first node, traverse the graph, keeping track of which nodes belong to which cycles
	int iteration = 1; // this is the current disjoint cycle number we're looking at
	int i; // don't confuse with iteration, this is just a generic loop counter
#if PRINT_APPLY_ESET
	DPRINTF("tracking disjoint cycles...\n");
#endif
	curCycle = (tour_t*)memory_chunk; // use up those memory chunks
	do // keep creating sub-cycles until there aren't any unvisited nodes
	{
		// grab the first unvisited node, in the unvisited nodes tracking array
#if PRINT_APPLY_ESET
		DPRINTF("grabbing first unvisited node: ");
#endif
		for (i=0; i < T->size; i++)
			if (!visited[i])
				break;
#if PRINT_APPLY_ESET
		DPRINTF("%i\n", i);
#endif
		// grab the next empty sub-cycle for us to fill up
		node_t* curNode, *startingNode, *lastNode, *tempNode;
		curNode = startingNode = T->node[i]; // current vertex
		E[iteration-1] = curCycle;
		// fill up this sub-cycle with connected vertices
		curCycle->size=0;
		curCycle->city[curCycle->size++] = Cities->city[curNode->id];
		lastNode = tempNode = 0; // tempNode for swapping, lastNode was previous node in the cycle
		do // keep going to the next node until we loop back
		{
			visited[curNode->id] = iteration;
			tempNode = curNode;
			curNode = curNode->edge[curNode->edge[0]==lastNode?1:0]; // make sure we don't back-track
#if PRINT_APPLY_ESET
			DPRINTF("next node : %i\n", curNode?curNode->id:-1);
#endif
			curCycle->city[curCycle->size++] = Cities->city[curNode->id]; // add this node onto the sub-cycle
			lastNode = tempNode; // swap temps
			INIT_EDGE(&edges[curEdge++], lastNode, curNode, iteration); // add the edge to the list
		} while (curNode != startingNode);
#if PRINT_CYCLES
#if PRINT_APPLY_ESET
		DPRINTF("disjointCycle %i: [%i]", iteration-1, curCycle->city[0]->id);
		int a;
		for (a=1; a < curCycle->size; a++)
		{
			DPRINTF(", [%i]", curCycle->city[a]->id);
		}
		DPRINTF("\n");
#endif
#endif
		// now curCycle has a complete sub-cycle in it
		++iteration;
		curCycle = (tour_t*)((char*)curCycle + sizeOfTour(curCycle)); // increment the pointer past the end of the current cycle
#if PRINT_APPLY_ESET
		DPRINTF("Checking visited array.\n");
#endif
		
		///////////////////////////////////////////////////////////////////////
		// check visited array for any unvisited vertices
		// yes, I know this looks like assembly programming but I think it's fast, and it makes sense to me :P
		for (i=0; i < T->size; i++)
			if (!visited[i])
				break; // found an unvisited node, keep iterating
		if (i < T->size) // if we broke earlier, then keep iterating
			continue;
		break;// if we made it this far, then all nodes were visited
	} while (1);
	
#if PRINT_CYCLES
#if PRINT_APPLY_ESET
	// output the sub-disjointCycles
	printf("Printing all %i cycles in the \033[32mIntermediate Tour\033[0m...\n", iteration-1);
	for (i=0; i < iteration-1; i++)
	{
		printf("Cycle[%i]: [%i]", i, E[i]->city[0]->id);
		int a;
		for (a=1; a < E[i]->size; a++)
			printf(", [%i]", E[i]->city[a]->id);
		printf("\n");
	}
#endif
#endif
	
	// sort the sub-cycles by size, I believe I implemented a basic selection sort here
	//*
	for (i=0; i < iteration-2; i++)
	{
#if PRINT_APPLY_ESET
		DPRINTF("Sorting by cycle size (%i)...", i);
#endif
		int a;
		int min = i;
		// find the next minimum
		for (a=i+1; a < iteration-1; a++)
		{
			if (E[a]->size < E[min]->size)
				min = a;
		}// for inner selection loop
		
		// check and do the swap iteration
		if (min != i) // if the cycle is out of position, swap it
		{
			tour_t* t = E[i];
			E[i] = E[min];
			E[min] = t;
			
			// need to swap the iteration numbers in the edge structure
#if PRINT_APPLY_ESET
			DPRINTF("Swapping cycles (actual cycleNum, edge->cycle is this +1) %i and %i.\n", i, min);
#endif
			int e;
			for (e=0; e < Cities->size; e++)
			{
				if (edges[e].cycle == i+1)
					edges[e].cycle = min+1;
				else if (edges[e].cycle == min+1)
					edges[e].cycle = i+1;
			}// for swapping edge cycle numbers
		}// if we needed to swap
#if PRINT_APPLY_ESET
		else
		{
			DPRINTF("noswap\n");
		}
#endif
	}// for outer selection loop
	// */
	
	return iteration-1; // return number of disjoint cycles
}// applyESet()

/**
 * merges the sub-tours in an intermediate tour creating a valid tour object
 * Cities : master cities structure containing all of the cities
 * T : IN : graph containing the disjoint sub-cycles in the intermediate tour OUT : fixed tour
 * cycles : IN: the array of the sub cycles in the intermediate tour
 * nCycles : number of disjoin sub-cycles in T
 * edges : an array that contains information about all of the edges in T
 * tourC : the "fixed" tour
 * returns : 1
 */
#define CUR_CYCLE 1  // current cycle is always the first cycle
int fixIntermediate(const tour_t* const Cities, graph_t* T /* byref */, tour_t** cycles, int nCycles, edge_t* edges, tour_t* tourC)
{
	int i; // loop counter
	int c; // loop counter for each city in the current subcycle
	int e; // current edge in the edge list that we're examining
	tour_t* curCycle; // current cycle
	//tour_t tempCycle; // this is what current cycle is most of the time
	
	// edges and costs
	edge_t e1, e2, e3, e4, e5, e6; // e1 and e2 are the two edges being examined for removal, e3-e6 are the 4 candidate edges, 2 of which to replace e1/e2
	edge_t b1, b2, b3, b4; // our current best choices, b1&b2 get removed, b3&b4 get added
	float bestCost; // cost of the current best edges found
	node_t *v1, *v2; // temporary nodes
	
	// edge "pruning"
	// since the only part that actually uses the edge structure is when searching for edges that
	// don't belong to the current cycle, we can remove all of the edges that belong to the current
	// cycle from the edges list.
	int numEdges = Cities->size;
	for (e=0; e < numEdges; e++)
	{
		while (e < numEdges && edges[e].cycle < 2)
		{
			// swap and remove
			edges[e] = edges[--numEdges];
#if PRINT_FIX_INTERMEDIATE
			DPRINTF("removed %i and now %i\n", e, numEdges);
#endif
		}
	}
	
	curCycle = cycles[0];
	memcpy(tourC, curCycle, sizeOfTour(curCycle));
	curCycle = tourC;
	while (nCycles > 1)
	{		
		// create the initial candidate edges
		v1 = T->node[curCycle->city[0]->id];
		v2 = T->node[curCycle->city[1]->id];
#if PRINT_FIX_INTERMEDIATE
		DPRINTF("Choosing starting edges.\n");
#endif
		INIT_EDGE(&b1, v1, v2, CUR_CYCLE);
		// find the first edge that doesn't belong to this cycle
		//for (e=0; e<numEdges;e++)
		//	if (edges[e].cycle != CUR_CYCLE)
		//		b2 = edges[e];
		b2 = edges[0];
		if (!b2.v1)
		{
			ERROR_TEXT;
			printf("there are no cycles left?\n");
			terminate_program(99);
		}
		// init starting point "guess" for best edge (it's just an arbitrary edge, in this case the first one)
		INIT_EDGE(&b3, v1, b2.v1, 0);
		INIT_EDGE(&b4, v2, b2.v2, 0);
		bestCost = b3.cost + b4.cost - b1.cost - b2.cost; // set bestCost to the original candidates
#if PRINT_FIX_INTERMEDIATE
		DPRINTF("Starting bestCost: %i\n", bestCost);
#endif
		
		// for each city in the sub-cycle
		for (c=0; c < curCycle->size-1; c++)
		{
			// grab edge we're examining
			v1 = T->node[curCycle->city[c]->id];
			v2 = T->node[curCycle->city[c+1]->id];
			
			// create an edge from those two vertices, this is the first edge to examine for removal
			INIT_EDGE(&e1, v1, v2, CUR_CYCLE);
			
			// now iterate over every edge in the graph that doesn't belong to curCycle
			for (e=0; e < numEdges; e++)
			{
				e2 = edges[e]; // e2 is the second edge to examine for removal
				//if (e2.cycle == CUR_CYCLE)
				//	continue;
				// construct the four candidate edges which would be created when removing e1 and e2
				INIT_EDGE(&e3, v1, e2.v1, 0); // one to one
				INIT_EDGE(&e4, v2, e2.v2, 0); //   "
				INIT_EDGE(&e5, v1, e2.v2, 0); // criss - cross
				INIT_EDGE(&e6, v2, e2.v1, 0); //   "
				// calculate the costs of using either set of edges
				float costA = e3.cost + e4.cost - e1.cost - e2.cost; // cost of edges being added minus the cost of the edges being removed
				float costB = e5.cost + e6.cost - e1.cost - e2.cost;
				// sort costs so that costA is the cheapest
				if (costB < costA)
				{
					// check to see if this cost is better than current best, if so replace it
					if (costB < bestCost)
					{
						bestCost = costB;
						b1 = e1;
						b2 = e2;
						b3 = e5;
						b4 = e6;
					}// if costB is better than current best
				}
				else
				{
					// check to see if this cost is better than current best, if so replace it
					if (costA < bestCost)
					{
						bestCost = costA;
						b1 = e1;
						b2 = e2;
						b3 = e3;
						b4 = e4;
					} // if costA is better than current best
				}// else costA < costB
			}// each edge in the graph
		}// each edge in the cycle
		
		// found the best candidates for merging, so merge them together
		tour_t* otherCycle = cycles[b2.cycle-1];
#if PRINT_FIX_INTERMEDIATE
		DPRINTF("best candidates for merging cycle %i with %i will cost %02.02f\n", CUR_CYCLE, b2.cycle, bestCost);
		DPRINTF("(\033[31mremoving\033[0m)b1: {%i->%i:c%f}\n", b1.v1->id, b1.v2->id, b1.cost);
		DPRINTF("(\033[31mremoving\033[0m)b2: {%i->%i:c%f}\n", b2.v1->id, b2.v2->id, b2.cost);
		DPRINTF("(\033[32m adding \033[0m)b3: {%i->%i:c%f}\n", b3.v1->id, b3.v2->id, b3.cost);
		DPRINTF("(\033[32m adding \033[0m)b4: {%i->%i:c%f}\n", b4.v1->id, b4.v2->id, b4.cost);
#endif
		// debug output the cycles
#if PRINT_FIX_INTERMEDIATE
#if PRINT_CYCLES
		DPRINTF("Before merging,   curCycle: ");
		int n;
		STRONG_TEXT;
		for (n=0; n < curCycle->size; n++)
		{
			DPRINTF("--> [%i]", curCycle->city[n]->id);
		}
		DPRINTF("\n");
		NORMAL_TEXT;
		DPRINTF("Before merging, otherCycle: ");
		STRONG_TEXT;
		for (n=0; n < otherCycle->size; n++)
		{
			DPRINTF("--> [%i]", otherCycle->city[n]->id);
		}
		DPRINTF("\n");
		NORMAL_TEXT;
#endif
#endif
		
		// merge the tour's together
		mergeSubTours(T, curCycle, otherCycle, &b1, &b2, &b3, &b4);
#if PRINT_FIX_INTERMEDIATE
#if PRINT_CYCLES
		DPRINTF("after merging, curCycle: ");
		STRONG_TEXT;
		for (n=0; n < curCycle->size; n++)
		{
			DPRINTF("--> [%i]", curCycle->city[n]->id);
		}
		DPRINTF("\n");
		NORMAL_TEXT;
#endif
#endif
		
#if PRINT_FIX_INTERMEDIATE
		DPRINTF("searching for b1 and b2 in edges array and replacing them with b3 and b4 respectively...\n");
#endif
		// Edge pruning (remove the edges that now belong to the main cycle)
		for (i=0; i < numEdges; i++)
		{
			/*
			edge_t* e1 = &edges[i];
			if ((e1->v1 == b1.v1 && e1->v2 == b1.v2)||(e1->v2 == b1.v1 && e1->v1 == b1.v2))
			{// replace b1 with b3
				INIT_EDGE(e1, b3.v1, b3.v2, 1); // now belongs to curCycle
			}
			else if ((e1->v1 == b2.v1 && e1->v2 == b2.v2)||(e1->v2 == b2.v1 && e1->v1 == b2.v2))
			{// replace b2 with b4
				INIT_EDGE(e1, b4.v1, b4.v2, 1); // now belongs to curCycle
			}// else replace b2
			else if (e1->cycle == b2.cycle)
			{
				e1->cycle = 1;
			}
			//*/
			while (i < numEdges && edges[i].cycle == b2.cycle)
			{
				//swap and remove
				edges[i] = edges[--numEdges];
#if PRINT_FIX_INTERMEDIATE
				DPRINTF("removed %i and now %i\n", i, numEdges);
#endif
			}
		}// for searching for edges to replace
		
		// remove otherCycle
		cycles[b2.cycle-1] = cycles[--nCycles];
		
#if PRINT_FIX_INTERMEDIATE
#if PRINT_GRAPHS
		DPRINTF("\033[32mIntermediate Tour T\033[0m contains (inside fixIntermediate, after combining two cycles): \n");
		int i2;
		for (i2=0; i2 < T->size; i2++)
		{
			DPRINTF("%04i [id:%04i] -> edges: ", i2, T->node[i2]->id);
			int e;
			for (e=0; e < T->node[i2]->size; e++)
				printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i2]->edge[e]->id, T->node[i2]->tour[e]);
			DPRINTF("\n");
		}
#endif
#endif
		
#if PRINT_FIX_INTERMEDIATE
		DPRINTF("Next cycle...\n");
#endif
	}// each cycle
	
	// copy curCycle back into cycles[0]
	//memcpy(cycles[0], curCycle, sizeOfTour(curCycle)); // TODO: this can be avoided by passing in a return cycle to store in
	--tourC->size;
	set_tour_fitness(tourC, tourC->size);
	
	return 1;
}// fixIntermediate()

/**
 * Perform the EAX algorithm on tour's A and B, storing the resulting tour into C
 * memory_chunk : a pointer to the beginning of a large chunk of memory (size MAX_CITIES*3/2*INTS_PER_CYCLE) that we can use to place the cycles into
 * CitiesA, CitiesB : master cities structure
 * tourA : parent A
 * tourB : parent B
 * tourC : OUT: parentC
 */
void performEAX(char* memory_chunk, tour_t* CitiesA, tour_t* CitiesB, tour_t* tourA, tour_t* tourB, tour_t* tourC) 
{
	int i;
	graph_t tempT, tempR;
	graph_t *R, *T;
	
	// set up graphs
	T = &tempT;
	R = &tempR;
	for (i=0; i < MAX_CITIES; i++)
	{
		R->node[i] = &R->alloc_node[i];
	}
	for (i=0; i < MAX_CITIES; i++)
	{
		T->node[i] = &T->alloc_node[i];
	}

#if PRINT_PARENT_TOURS
	// printing the tours
	print_tour(tourA);
	print_tour(tourB);
#endif
	
	///////////////////////////////////////////////////////////////////////////
	// merge the two tours
	// TODO: MEMORY the graph needs to be created once somewhere and then saved
#if PRINT_STEPS
	DPRINTF("\nMerging A with B...");
#endif
	mergeTours(R, tourA, tourB);
#if PRINT_STEPS
	DPRINTF("done!\n");
#endif

	// output the merged graph
#if PRINT_GRAPHS
	printf("\nGraph R contains: \n");
	for (i=0; i < CitiesA->size; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, R->node[i]->id);
		int e;
		for (e=0; e < R->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", R->node[i]->edge[e]->id, R->node[i]->tour[e]);
		printf("\n");
	}
#endif

	///////////////////////////////////////////////////////////////////////////
	// create A-B cycles on R
	// TODO: MEMORY cycles memory needs to be allocated once and then saved somewhere
#if PRINT_STEPS
	DPRINTF("Allocating cycles...");
#endif
	//tour_t** cycles;
	//tour_t* alloc_cycles_array[MAX_ABCYCLES];
	tour_t* cycles[MAX_ABCYCLES];
	//tour_t alloc_cycles[MAX_ABCYCLES];
	//cycles = malloc(sizeof(tour_t *) * MAX_ABCYCLES);
	//cycles = &alloc_cycles_array;
	//for (i=0; i < MAX_ABCYCLES; i++)
	//{
	//	cycles[i] = &alloc_cycles[i];
	//}
#if PRINT_STEPS
	DPRINTF("done!\n");
#endif
	int nCycles;
#if PRINT_STEPS
	DPRINTF("Generating AB Cycles....");
#endif
	// REMEMBER! R gets defiled by this call, and the contents of cycles isn't important, it all gets overwritten
#if PRINT_CYCLE_POINTERS
	//DPRINTF("\033[35mCycles (before generateABCycles)  : %i", cycles[0]);
	//for (i=1; i < 6; i++) DPRINTF(", %i", cycles[i]);
	//DPRINTF("\033[0m\n");
#endif
	nCycles = generateABCycles(memory_chunk, CitiesA, CitiesB, R, cycles);
#if PRINT_MISC
	DPRINTF("done!\n");
#endif

#if PRINT_CYCLES
	// output the cycles
	printf("Printing all %i ABcycles...\n", nCycles);
	for (i=0; i < nCycles; i++)
	{
		printf("Cycle[%i]: [%i]", i, cycles[i]->city[0]->id);
		int a;
		for (a=1; a < cycles[i]->size; a++)
			printf("-%s-> [%i]", (cycles[i]->city[a]->tour == TOUR_A ? "A" : "B"), cycles[i]->city[a]->id);
		printf("\n");
	}
#endif

	///////////////////////////////////////////////////////////////////////////
	// apply E-sets to generate intermediates
	createGraph(T, tourA);
#if PRINT_GRAPHS
	// output the created graph from tourA
	printf("\n\033[32mIntermediate Tour T\033[0m contains (this is tourA): \n");
	for (i=0; i < CitiesA->size; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, T->node[i]->id);
		int e;
		for (e=0; e < T->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i]->edge[e]->id, T->node[i]->tour[e]);
		printf("\n");
	}
#endif
	// create edges array
#if PRINT_STEPS
	DPRINTF("allocating edges array...\n");
#endif
	//edge_t** edges = (edge_t**)malloc(sizeof(edge_t *) * CitiesA->size);
	edge_t edges[MAX_CITIES];
	//edge_t alloc_edges[MAX_CITIES];
	//for (i=0; i < CitiesA->size; i++)
	//{
	//	edges[i] = &alloc_edges[i];
	//}
#if PRINT_STEPS
	DPRINTF("Applying the E-set.\n");
#endif
#if PRINT_CYCLE_POINTERS
	DPRINTF("\033[35mCycles (before applyESET) : %i", cycles[0]);
	for (i=1; i < 6; i++) { DPRINTF(", %i", cycles[i]); }
	DPRINTF("\033[0m\n");
#endif
	int disjointCycles = applyESet(memory_chunk, CitiesA, T, cycles, nCycles, edges);
#if PRINT_CYCLE_POINTERS
	DPRINTF("\033[35mCycles (after applyESET)  : %i", cycles[0]);
	for (i=1; i < 6; i++) { DPRINTF(", %i", cycles[i]); }
	DPRINTF("\033[0m\n");
#endif
#if PRINT_STEPS
	DPRINTF("there were \033[32m%i\033[0m disjoint cycles.\n", disjointCycles);
#endif
#if PRINT_INTERMEDIATE_INFO
#if PRINT_GRAPHS
	// output the intermediate
	printf("\n\033[32mIntermediate Tour T\033[0m contains (after returning from applyESEt): \n");
	for (i=0; i < CitiesA->size; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, T->node[i]->id);
		int e;
		for (e=0; e < T->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i]->edge[e]->id, T->node[i]->tour[e]);
		printf("\n");
	}
	dumpGraphToFile(T, "graphBefore.txt");
#endif
	// output the edges
#if PRINT_EDGES
	printf("Printing all %i edges in the graph: \n", CitiesA->size);
	for (i=0; i < CitiesA->size; i++)
	{
		printf("Edge[%i] = {%i -> %i : i%i : c%f}\n", i, edges[i].v1->id, edges[i].v2->id, edges[i].cycle, edges[i].cost);
	}
#endif
#if PRINT_CYCLES
	// output the sub-disjointCycles
	printf("Printing all %i cycles in the \033[32mIntermediate Tour\033[0m...\n", disjointCycles);
	for (i=0; i < disjointCycles; i++)
	{
		printf("Cycle[%i]: [%i]", i, cycles[i]->city[0]->id);
		int a;
		for (a=1; a < cycles[i]->size; a++)
			printf(", [%i]", cycles[i]->city[a]->id);
		printf("\n");
	}
#endif
#endif

	///////////////////////////////////////////////////////////////////////////
	// turn intermediates into valid tours
	/*int code=*/fixIntermediate(CitiesA, T /* byref */, cycles, disjointCycles, edges, tourC);
#if PRINT_INTERMEDIATE_INFO
#if PRINT_GRAPHS
	printf("\n\033[32mIntermediate Tour T\033[0m contains (after returning from fixIntermediate): \n");
	for (i=0; i < CitiesA->size; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, T->node[i]->id);
		int e;
		for (e=0; e < T->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i]->edge[e]->id, T->node[i]->tour[e]);
		printf("\n");
	}
	dumpGraphToFile(T, "graphAfter.txt");
#endif
#endif
	
#if PRINT_CYCLES
	STRONG_TEXT;
	printf("Resulting cycle:[%i]: [%i]", i, cycles[0]->city[0]->id);
	int a;
	for (a=1; a < cycles[0]->size; a++)
		printf(", [%i]", cycles[0]->city[a]->id);
	printf("\n");
	NORMAL_TEXT;
#endif

	///////////////////////////////////////////////////////////////////////////
	// GA step?
	// cycles[0] contains the only cycle, and should contain the new tour.
	// fix the tour by removing the last vertex
	//--cycles[0]->size;
	//set_tour_fitness(cycles[0], cycles[0]->size);
	//memcpy(tourC, cycles[0], sizeOfTour(cycles[0])); // TODO: this can be avoided by passing in tourC as the return cycle to fixIntermediate
	
	// clean up
#if CHOOSE_BEST_FROM_THREE
	// if we're doing this ga modification, instead of always returning the child,
	// instead return the best tour from the set: (tourA, tourB, and the child)
	if (tourA->fitness < tourB->fitness)
	{
		if (tourC->fitness < tourA->fitness)
		{
			// do nothing
		}
		else
		{
			memcpy(tourC, tourA, sizeOfTour(tourA));
		}
	}
	else
	{
		if (tourC->fitness < tourB->fitness)
		{
			// do nothing
		}
		else
		{
			memcpy(tourC, tourB, sizeOfTour(tourB));
		}
	}
#endif

} // performEAX()
