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
		curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 0; // these edges belong to A
		curNode->edge[curNode->size++] = R->node[tA->city[i-1]->id]; // previous node in tourA
		curNode->edge[curNode->size++] = R->node[tA->city[i+1]->id]; // next node in tourA
		
		// tourB's cities
		curNode = R->node[tB->city[i]->id]; // grab the node representing tour B's city[i]
		curNode->tour[curNode->size] = curNode->tour[curNode->size+1] = 1; // these edges belong to B
		curNode->edge[curNode->size++] = R->node[tB->city[(i-1+tourSize)%tourSize]->id]; // previous node in tourB
		curNode->edge[curNode->size++] = R->node[tB->city[(i+1)%tourSize]->id]; // next node in tourB
	}
	printf("Special cases..."); //TODO debug remove
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
	int visited[MAX_CITIES];
	int size = 0; // size of the cycles array, number of AB cycles
	node_t *v0, *v1, *v2; // vertex one and two of the current edge, v0 is the first v1
	//note: if v1==v2 at start of iteration, choose an edge incident to v1
	int edges = R->size*MAX_EDGES/2; // R should have MAX_EDGES/2 edges for every vertex, we half it because it's an undirected graph and don't want duplicates. (NOTE: could still have duplicates if both tours share an edge, this is ok).
	tour_t* curCycle = 0; // current AB cycle we're working on

	// initializations
	v0 = v1 = v2 = 0;

	// generate AB cycles
	while (edges > 0)
	{
		printf("Generating AB Cycle, edges left: %i...\n", edges);
		// pick starting edge
		if (v1==v2 && v1 && v2)
		{
			printf("Picking a vertex incident to %i...\n", v1->id);
			// choose an edge incident to v1
			// we can choose any edge from v1, because the edges
			// contained in the ab cycle should've already been removed.
			if (v1->size < 1)
			{
				printf("generateABCycles() :: ERROR : v1 has no more edges. (edge: %i -> %i)\n", v1->id, v2->id);
				printf("halting...\n");
				exit(31);
			}

			// pick incident vertex (this happens below)
			// v1 is already set from previous iteration
			v2 = 0;
		}
		else
		{
			printf("Choosing a random vertex...\n");//TODO: debug remove
			// choose a vertex with at least 2 edges (to create a loop)
			v1 = R->node[rand() % R->size];
			int panic = 0;
			printf("first pick: [%i]->s:%i\n", v1->id, v1->size);
			for (;v1->size == 0 && panic<PANIC_EXIT; panic++)
				v1 = R->node[rand() % R->size];
			if (panic >= PANIC_EXIT)
			{
				printf("generateABCycles() :: ERROR : panic_exit : too many iterations (%i >= %i) when trying to pick a vertex that has remaining edges. halting...\n", panic, PANIC_EXIT);
				exit(32);
			}
		}
		printf("v1:%i...\n", v1->id);//TODO: debug remove
		// grab vertex 2
		int v2i; // temporary, we need to know the index of the edge that will become v2
		v2i = rand() % v1->size;
		v2 = v1->edge[v2i];
		int nextTour = (v1->tour[v2i]+1)%2; // next time, pick an edge from the other tour
		printf("v2:%i...\n", v2->id); //TODO: debug remove

		// set up current cycle
		curCycle = cycles[size++];
		curCycle->size = 0;
		curCycle->city[curCycle->size++] = Cities->city[v1->id]; // add the first vertex to the cycle
		memset((void*)visited, 0, sizeof(visited)); // reset visited nodes for this cycle
		visited[v1->id] = 1;
		v0 = v1; // save the starting vertex
		printf("entering cycle loop...\n"); //TODO: debug remove
		while (visited[v2->id]==0) // keep alternating until we've made a loop
		{
			// first, add this edge to the current cycle
			//printf("initial increment...\n");
			curCycle->city[curCycle->size++] = Cities->city[v2->id];
			visited[v2->id] = 1;
			printf("Adding edge to cycle...\n");//TODO: debug remove

			// remove the edge from the graph
			// undirected graph, so first remove the edge from v1
			--edges;

			// turns out, v2i is always preserved as the edge we last choice from v1
			// so we can just re-use it here!
			printf("removing edge(v[%i]->e[%i] from graph...\n", 
				(v1?v1->id:-1), 
				(v1 && v1->edge[v2i]?v1->edge[v2i]->id:-1));//TODO: debug remove
			REMOVE_EDGE(v1, v2i); // boo-yah for O(1) operations
			/*
			//TODO: it may be possible to speed this up by caching the edge index
			switch(v1->size) // I'm using a switch here to avoid iteration
			{
			// a vertex at this stage will always have 2, 3, or 4 edges
			// it is impossible for it to not have any, as how would we have gotten to it
			// and it is also impossible for it to only have 1, as we would be unable
			// to generate a cycle
			case 2:
				if (v1->edge[0] == v2) // one of two possible edges
					REMOVE_EDGE(v1, 0);
				else
					REMOVE_EDGE(v1, 1);
				break;
			case 3:
				if (v1->edge[0] == v2) // three possible edges to check
					REMOVE_EDGE(v1, 0);
				else if (v1->edge[1] == v2)
					REMOVE_EDGE(v1, 1);
				else
					REMOVE_EDGE(v1, 2);
				break;
			case 4:
				if (v1->edge[0] == v2) // four possible edges to check
					REMOVE_EDGE(v1, 0);
				else if (v1->edge[1] == v2)
					REMOVE_EDGE(v1, 1);
				else if (v1->edge[2] == v2)
					REMOVE_EDGE(v1, 2);
				else
					REMOVE_EDGE(v1, 3);
				break;
			default:
				printf("ERROR : invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				exit(43);
				break;
			}// removing edge from v1 */
			// undirected graph, so now we must remove the edge from v2
			//TODO: it may be possible to speed this up by caching the edge index
			switch(v2->size) // using a switch here to avoid iteration
			{
			case 2:
				if (v2->edge[0] == v1) // one of two possible edges
					REMOVE_EDGE(v2, 0);
				else
					REMOVE_EDGE(v2, 1);
				break;
			case 3:
				if (v2->edge[0] == v1) // three possible edges to check
					REMOVE_EDGE(v2, 0);
				else if (v2->edge[1] == v1)
					REMOVE_EDGE(v2, 1);
				else
					REMOVE_EDGE(v2, 2);
				break;
			case 4:
				if (v2->edge[0] == v1) // four possible edges to check
					REMOVE_EDGE(v2, 0);
				else if (v2->edge[1] == v1)
					REMOVE_EDGE(v2, 1);
				else if (v2->edge[2] == v1)
					REMOVE_EDGE(v2, 2);
				else
					REMOVE_EDGE(v2, 3);
				break;
			default:
				printf("ERROR43: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				exit(43);
				break;
			}// removing the edge from v2

			//TODO: alternate and choose an edge from the other tour
			//printf("Alternating tour picking new edge...\n");//TODO: debug remove
			v1 = v2; // we need to pick a new v2
			int c[2]; // choice1 and an optional choice 2
			c[0] = c[1] = 0; // NOTE: these are off by one indices to speed up boolean checks!
			switch(v1->size) // TODO: this could be moved into the other switch
			{
			case 1: // this should be the case only if we just created a closed loop
				c[0] = (v1->tour[0] == nextTour) ? 1 : 0; // only one edge to pick
				break;
			case 2: // this would be the case when there is only one path left on these verts
				c[0] = (v1->tour[0] == nextTour) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == nextTour) ? 2 : 0;
				break;
			case 3: // this would be the case when creating a closed loop, but also another loop
			        // could be on that vertex in another AB cycle
				c[0] = (v1->tour[0] == nextTour) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == nextTour) ? 2 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[2] == nextTour) ? 3 : 0;
				break;
			case 4: // this would be the case when two AB cycles can be generated from that vertex
				c[0] = (v1->tour[0] == nextTour) ? 1 : 0;
				// use boolean indices to decide if it's an optional choice or the only choice
				c[(c[0] == 1)?1:0] = (v1->tour[1] == nextTour) ? 2 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[2] == nextTour) ? 3 : 0;
				c[(c[0] == 1)?1:0] = (v1->tour[3] == nextTour) ? 4 : 0;
				break;
			default:
				printf("ERROR44: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
				exit(44);
				break;
			}// calculating choices for the next edge
			if (!c[0])
			{
				printf("ERROR45: no choices for next vertex found v[%i] (%i) HALT\n", v1->id, v1->size);
				exit(45);
			}
			
			// pick one of the choices if there are more than one, otherwise the only choice
			v2i = c[1] ? c[rand() % 2]-1 : c[0]-1; // subtract one, since they're OBO, see above
			printf("v2i: %i, c[0]: %i, c[1]: %i\n",v2i, c[0]-1, c[1]-1);
			printf("v1e0: %i, v1e1: %i, v1e2: %i, v1e3: %i\n", 
				(v1->edge[0]?v1->edge[0]->id:-1), 
				(v1->edge[1]?v1->edge[1]->id:-1), 
				(v1->edge[2]?v1->edge[2]->id:-1), 
				(v1->edge[3]?v1->edge[3]->id:-1));
			v2 = v1->edge[v2i];
			//printf("next!\n");
			printf("next iteration...v1:%i, v2:%i\n", v1->id, v2->id);//TODO: debug remove

			// alternate to the next tour
			nextTour=(nextTour+1)%2;
			//printf("Next tour: %i\n", nextTour);

		} // while creating a cycle
		// first, add the final edge to the current cycle
		curCycle->city[curCycle->size++] = Cities->city[v2->id];
		printf("Adding edge to cycle...\n");//TODO: debug remove
		// remove the edge from the graph
		// undirected graph, so first remove the edge from v1
		--edges;
		// turns out, v2i is always preserved as the edge we last choice from v1
		// so we can just re-use it here!
		printf("removing edge(v[%i]->e[%i] from graph...\n", 
			(v1?v1->id:-1), 
			(v1 && v1->edge[v2i]?v1->edge[v2i]->id:-1));//TODO: debug remove
		REMOVE_EDGE(v1, v2i); // boo-yah for O(1) operations
		// undirected graph, so now we must remove the edge from v2
		//TODO: it may be possible to speed this up by caching the edge index
		switch(v2->size) // using a switch here to avoid iteration
		{
		case 2:
			if (v2->edge[0] == v1) // one of two possible edges
				REMOVE_EDGE(v2, 0);
			else
				REMOVE_EDGE(v2, 1);
			break;
		case 3:
			if (v2->edge[0] == v1) // three possible edges to check
				REMOVE_EDGE(v2, 0);
			else if (v2->edge[1] == v1)
				REMOVE_EDGE(v2, 1);
			else
				REMOVE_EDGE(v2, 2);
			break;
		case 4:
			if (v2->edge[0] == v1) // four possible edges to check
				REMOVE_EDGE(v2, 0);
			else if (v2->edge[1] == v1)
				REMOVE_EDGE(v2, 1);
			else if (v2->edge[2] == v1)
				REMOVE_EDGE(v2, 2);
			else
				REMOVE_EDGE(v2, 3);
			break;
		default:
			printf("ERROR43: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
			exit(43);
			break;
		}// removing the edge from v2
		// printing cycle as it is
		printf("Ab cycle generated: ");
		int n;
		for (n=0; n < curCycle->size; n++)
			printf("->[%i]", curCycle->city[n]->id);
		printf("\n");

		// check to see if we've made a cycle with a tail
		if (v2 != v0)
		{
			printf("Cycle with tail generated, removing tail...\n");
			//v1 = v0; // move back to the original vertex
			v0 = v2; // the "new" original vertex is the actual end of the cycle

			// flip the entire current cycle, this makes it easier to remove cities from it
			int a,b;
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
			printf("Ab cycle reversed: ");
			int n;
			for (n=0; n < curCycle->size; n++)
				printf("->[%i]", curCycle->city[n]->id);
			printf("\n");

			// now keep removing the last city in the cycle and adding that edge back to R
			// until v2 represents the last city in the cycle
			v2 = R->node[curCycle->city[curCycle->size-1]->id];
			while (v0->id != curCycle->city[--curCycle->size]->id)
			{
				// grab the new tip of the tail, and restore the edge
				v1 = v2;
				v2 = R->node[curCycle->city[curCycle->size-1]->id];
				printf("Restoring edge: %i->%i\n", v1->id, v2->id);
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
					if (v1->edge[1] == v2)
						RESTORE_EDGE(v1, 1);
					else if (v1->edge[2] == v2)
						RESTORE_EDGE(v1, 2);
					else
						RESTORE_EDGE(v1, 3);
					break;
				case 3:
					RESTORE_EDGE(v1, 3);
					break;
				default:
					printf("ERROR53: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
					exit(53);
					break;
				}// removing edge from v1 */
				// undirected graph, so now we must remove the edge from v2
				//TODO: it may be possible to speed this up by caching the edge index
				switch(v2->size) // using a switch here to avoid iteration
				{
				case 0:
					if (v2->edge[0] == v1)
						RESTORE_EDGE(v2, 0);
					else if (v2->edge[1] == v1)
						RESTORE_EDGE(v2, 1);
					else if (v2->edge[2] == v1)
						RESTORE_EDGE(v2, 2);
					else
						RESTORE_EDGE(v2, 3);
					break;
				case 1: // terminal V edge case
					if (v2->edge[1] == v1)
						RESTORE_EDGE(v2, 1);
					else if (v2->edge[2] == v1)
						RESTORE_EDGE(v2, 2);
					else
						RESTORE_EDGE(v2, 3);
					break;
				case 2:
					if (v2->edge[2] == v1)
						RESTORE_EDGE(v2, 2);
					else
						RESTORE_EDGE(v2, 3);
					break;
				default:
					printf("ERROR54: invalid # of edges on vertex[%i] (%i) halting...\n", v1->id, v1->size);
					exit(54);
					break;
				}// restoring edge to v2

				// we're done restoring the edge
				++edges;
			}// while fixing the cycle

			printf("Ab cycle as fixed: ");
			//int n;
			for (n=0; n < curCycle->size; n++)
				printf("->[%i]", curCycle->city[n]->id);
			printf("\n");
			// tell the next iteration to start on the problematic vertex
			v1 = v2;
		}// (end) if we needed to fix the cycle
		else // closed loop created normally
		{
			//v1 = v2 = 0; //TODO: i don't think this is actually necessary.
			printf("No fixing necessary (no 'tail' on cycle)\n.");
		}

		printf("next AB Cycle (back to top of outer while)\n"); //TODO: debug remove
	}// while R has edges left

	return size;
}

