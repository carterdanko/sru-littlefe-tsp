/**
 * main module for the TSP solver
 */

#include "include/tsp.h"
#include "include/eax.h"

// "global" variables. I try to start these with capital letters
tour_t* Cities; // the "tour" that contains every city in their provided order. Not really a tour, just used as the master array of cities.
tour_t** Tours; // all of the current tours in the population


/*
 * Use of this function is not currently necessary but will be when MPI is implemented.
 */
void terminate_program(int ecode) {
	//TODO: MPI close function calls

	// only runs for "successful" program termination.
	if (ecode==0) {
		// clean up
		printf("graph gone...");
		freeCities(Cities);
		printf("main cities gone...");
		//for (i=0; i < MAX_ABCYCLES; i++)
		//	freeCities(cycles[i]);
		printf("AB cycles structure gone...");
		printf("done!\n");

		//free tours
		free(Tours);

		// done (just used to make sure that the program ran to completion)
		printf("Program ran to completion (done).\n");
	}

	// exit the program.
	exit(ecode);
}

void populate_tours(int N, int mpi_rank, tour_t** arr_tours, tour_t* arr_cities) {
	int i;

	for (i=0;i<N;i++) {
		arr_tours[i] = create_tour_nn(arr_cities->city[i], N, arr_cities);
		set_tour_fitness(arr_tours[i], N);
	}
}

void MPI_init(char *mpi_flag, int *mpi_rank, int *mpi_procs) {
	if (*mpi_flag>0) {
		// If running MPI, then use the MPI commands.
		//TODO: MPI initlizations
	} else {
		*mpi_rank = 0;
		*mpi_procs = 1;
	}
}

void master_listener(int *iter, int *delta_iter, char *lcv, tour_t** arr_tours) {
	// if you are within the constraints, perform actions
	if (*iter<MAX_ITERATIONS && *delta_iter<MAX_DELTA) {
		float delta_fit=0.0;

		//TODO: MPI send (tours back to each island)

		//TODO: note -- this will have to listen for ANY island & not just the sequence 1..num_procs

		//TODO: MPI receive (tours from each island)

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: udpate master population (sort it)

		// Next, subtract by the new best tour's fitness
		delta_fit-=arr_tours[0]->fitness;
		if (delta_fit<=DELTA) {
			// If you are within DELTA, increment counter
			*delta_iter++;
		} else {
			//Otherwise, reset counter
			*delta_iter=0;
		}

		// increment the iteration number.
		*iter++;
	}
	// Otherwise, order other processes to halt
	else {
		*lcv = 0;
		//TODO: MPI send "stop"
	}
}

void serial_listener(int *iter,int *delta_iter,char *lcv,tour_t** arr_tours, int N) {
	// if you are within the constraints, perform actions
	if (*iter<MAX_ITERATIONS && *delta_iter<MAX_DELTA) {
		float delta_fit=0.0;

		// run the GA
		run_genalg(N,lcv);

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: udpate master population

		// Next, subtract by the new best tour's fitness
		delta_fit-=arr_tours[0]->fitness;
		if (delta_fit<=DELTA) {
			// If you are within DELTA, increment counter
			*delta_iter++;
		} else {
			//Otherwise, reset counter
			*delta_iter=0;
		}

		// increment the iteration number.
		*iter++;
	}
	// Otherwise, order other processes to halt
	else {
		*lcv = 0;
	}
}


void load_cities(int mpi_rank, char *citiesFile, tour_t *arr_cities) {
	// if master...
	if (mpi_rank==0) {
		// load the cities specified by the file
		DPRINTF("Loading cities...");
		Cities = loadCities(citiesFile);
		if (!Cities)
		{
			printf("Error while loading cities. refer to error log? halting.\n");
			terminate_program(5); // ERROR: error while loading cities
		}
		DPRINTF("done! (loaded %i cities from the file)\n", Cities->size);
		//TODO: MPI send cities
	}
	// otherwise...
	else {
		//TODO: MPI receive cities
	}
}

void run_genalg(int N, char *lcv) {
	//TODO: notice -- the slaves will start by reciving tours.
		// this way, if we are to stop, master will send an empty tour.
	//TODO: MPI Recv new tours from master

	// if the tour is not empty...

		//TODO: Update sorted population array

		//TODO: Select parents for child creation (roulette wheel)

		// Create children
		perform_eax(N);

		//TODO: MPI Send tours

	// else, set lcv->0
}

void perform_eax(int N) {
	int i;

	// merge the two tours
	printf("\nMerging A with B...");
	graph_t* R = mergeTours(Tours[0], Tours[1]);
	printf("done!\n");

	// output the merged graph
	printf("\nGraph R contains: \n");
	for (i=0; i < N; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, R->node[i]->id);
		int e;
		for (e=0; e < R->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", R->node[i]->edge[e]->id, R->node[i]->tour[e]);
		printf("\n");
	}

	// create A-B cycles on R
	printf("Allocating cycles...");
	tour_t** cycles;
	cycles = (tour_t**)malloc(sizeof(tour_t *) * MAX_ABCYCLES);
	for (i=0; i < MAX_ABCYCLES; i++)
	{
		cycles[i] = (tour_t*)malloc(sizeof(tour_t));
	}
	printf("done!\n");
	int nCycles;
	printf("Generating AB Cycles....");
	// REMEMBER! R gets defiled by this call, and the contents of cycles isn't important, it all gets overwritten
	nCycles = generateABCycles(Cities, R, cycles);
	printf("done!\n");

	// output the cycles
	printf("Printing all %i cycles...\n", nCycles);
	for (i=0; i < nCycles; i++)
	{
		printf("Cycle[%i]: [%i]", i, cycles[i]->city[0]->id);
		int a;
		for (a=1; a < cycles[i]->size; a++)
			printf(", [%i]", cycles[i]->city[a]->id);
		printf("\n");
	}

	// create E-sets from the cycles
	nCycles = generateESetRAND(Cities, cycles, nCycles);
	// output the E-set
	printf("Printing all %i cycles in the \033[32mE-set\033[0m...\n", nCycles);
	for (i=0; i < nCycles; i++)
	{
		printf("Cycle[%i]: [%i]", i, cycles[i]->city[0]->id);
		int a;
		for (a=1; a < cycles[i]->size; a++)
			printf(", [%i]", cycles[i]->city[a]->id);
		printf("\n");
	}

	// apply E-sets to generate intermediates
	graph_t* T = createGraph(Tours[0]);
	// output the created graph from tourA
	printf("\n\033[32mIntermediate Tour T\033[0m contains: \n");
	for (i=0; i < N; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, T->node[i]->id);
		int e;
		for (e=0; e < T->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i]->edge[e]->id, T->node[i]->tour[e]);
		printf("\n");
	}
	// apply the eset to the graph
	printf("allocating edges array...\n");
	edge_t** edges = (edge_t**)malloc(sizeof(edge_t *) * Cities->size);
	for (i=0; i < Cities->size; i++)
	{
		edges[i] = (edge_t*)malloc(sizeof(edge_t));
	}
	printf("Applying the E-set.\n");
	int disjointCycles = applyESet(Cities, T, cycles, nCycles, edges);
	printf("there were \033[32m%i\033[0m disjoint cycles.\n", disjointCycles);
	// output the intermediate
	printf("\n\033[32mIntermediate Tour T\033[0m contains: \n");
	for (i=0; i < N; i++)
	{
		printf("%04i [id:%04i] -> edges: ", i, T->node[i]->id);
		int e;
		for (e=0; e < T->node[i]->size; e++)
			printf((e>0) ? ", [%04i:t%01i]" : "[%04i:t%01i]", T->node[i]->edge[e]->id, T->node[i]->tour[e]);
		printf("\n");
	}
	// output the edges
	printf("Printing all %i edges in the graph: \n", Cities->size);
	for (i=0; i < Cities->size; i++)
	{
		printf("Edge[%i] = {%i -> %i : i%i : c%f}\n", i, edges[i]->v1->id, edges[i]->v2->id, edges[i]->cycle, edges[i]->cost);
	}
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

	//TODO: turn intermediates into valid tours
	/*int code=*/fixIntermediate(Cities, T /* byref */, cycles, nCycles, edges);

	// clean up
	printf("\nClean up...");
	freeGraph(R);
	freeGraph(T);

}

int main(int argc, char** argv)
{
	int randSeed = 0; // random seed to use
	char* citiesFile = 0; // cities file name
	int i; // loop counter
	char mpi_flag; // mpi is on (1) or off (0)
	int mpi_rank,mpi_procs; // mpi rank (for each process) and number of processes
	char lcv = 1; // loop control variable for the while loop (run until lcv->0)

	//TODO: make argument handler set the number of procedures (mpi_procs) and mpi_flag.
	mpi_flag = 0;
	mpi_procs = 1;

	//####################################################
	// Argument Handler
	//####################################################
	// check number of parameters
	if (argc < 2)
	{
		printf("Usage: %s [flags] <filename of cities text document>\n", argv[0]);
		printf("Try -h or --help for more information.\n");
		exit(1); // ERROR: must supply a filename for the cities
	}
	else // process params
	{
		for (i=1; i < argc; i++)
		{
			char* p = argv[i];
			if (strcmp(p, "-h") == 0 || strcmp(p, "-H") == 0 || strcmp(p, "--help") == 0 || strcmp(p, "--HELP") == 0)
			{
				printf("Usage: %s [flags] <filename of cities text document>\n", argv[0]);
				printf(" -- File Format Explanation --\n");
				printf("  The first line of the cities text document is the number of cities.\n");
				printf("  The following lines are each city. 2 integers, space separated, are the x and y of that city. Example: \n");
				printf("  23 45\n");
				printf(" -- optional flags --\n");
				printf("-h, --help : this screen.\n");
				printf("-s <random seed> : random seed to initialize srand with.\n");
				//printf("-d <maximum distance> : max x or y.\n");
			}
			else if (strcmp(p, "-s") == 0)
			{
				// random seed
				randSeed = atoi(argv[++i]);
			}
			else
			{
				citiesFile = argv[i];
			}// else filename
		}// for each argument
	}// else process the arguments

	// check to make sure we got a city file
	if (!citiesFile)
	{
		printf("no city file present. halting\n");
		exit(3); // ERROR: no city file present
	}

	// initialize srand
	if (randSeed)
	{
		DPRINTF("Using \033[31m%i\033[0m as random seed.\n", randSeed);
		srand(randSeed);
	}
	else // otherwise use a random seed
	{
		randSeed = time(0);
		DPRINTF("Picked a random seed (\033[31m%i\033[0m).\n", randSeed);
		srand(randSeed);
	}
	//----------------------------------------------------


	//####################################################
	// MPI Initializations
	//####################################################
	// Handles MPI initializations and sets its variables.
	MPI_init(&mpi_flag,&mpi_rank,&mpi_procs);
	//----------------------------------------------------


	//####################################################
	// Load Cities, Initialize Tables, Create Init tours
	//####################################################
	// load the cities
	if (mpi_flag==1) {
		// If MPI active, then let the master load cities and broadcast
		if (mpi_rank==0) {
			load_cities(mpi_rank,citiesFile,Cities);
			//TODO: MPI Send cities
		} else {
			//TODO: MPI Receive cities
		}
	} else {
		// Otherwise, just load the cities.
		load_cities(mpi_rank,citiesFile,Cities);
	}
	// process the cities
	int N = Cities->size;
	// allocate memory for Tours
	Tours = malloc( sizeof(tour_t*) * MAX_POPULATION );

	// construct the distance table (on all processes)
	printf("Enter dist table\n");
	construct_distTable(Cities,N);
	printf("Exit dist table\n");

	// output the city information to the console
	DPRINTF("\nNum Cities: %04i\n", Cities->size);
	DPRINTF("---------------------------\n");
	for (i=0; i < Cities->size; i++)
	{
		DPRINTF("City[%04i] at %04i, %04i   [id: %04i]\n", i, Cities->city[i]->x, Cities->city[i]->y, Cities->city[i]->id);
	}

	// populate tours (on all processes)
	populate_tours(N,mpi_rank,Tours,Cities);
	print_tour(Tours[0]);
	print_tour(Tours[1]);
	//----------------------------------------------------


	//####################################################
	// Run Genetic Algorithm (Enter "The Islands")
	//####################################################
	int iter=0; // the number of iterations performed.
	int delta_iter=0; // the number of iterations performed with fitness consecutively within DELTA.
	while (lcv) {
		if (mpi_flag==1) {
			if (mpi_rank==0) {
				// if you are the master AND mpi is on, start listening
				master_listener(&iter,&delta_iter,&lcv,Tours);
			}
			else {
				// if you are a slave and MPI is on, run the GA
				run_genalg(N,&lcv);
			}
		} else {
			// otherwise, run the GA and perform the loop condition checks manually.
			run_genalg(N,&lcv);
			serial_listener(&iter,&delta_iter,&lcv,Tours,N);
		}
	}
	//----------------------------------------------------


	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}

