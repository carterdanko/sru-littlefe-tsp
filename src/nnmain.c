/**
 * main module for the TSP solver
 */

#include "include/tsp.h"
#include "include/eax.h"

// "global" variables. I try to start these with capital letters
tour_t *CitiesA, *CitiesB; // the "tour" that contains every city in their provided order. Not really a tour, just used as the master array of cities.
tour_t** Tours, **childrenTours; // all of the current tours in the population
// global variables about the running state of the program
int randSeed = 0;
char* citiesFile = 0;
char *toursFile = 0;
int mpi_rank = 0;
int numFileTours=0;

time_t startTime; // time the program started running
int* intTours;
int* intCities;
#if BEST_TOUR_TRACKING
tour_t** BestTours;          // array containing the best tours
int sizeBestTours; // how many best tours there are
tour_t* lastBestTour;        // the previous best tour, last iteration
#endif

/**
 * fills the population of tours with an initial set of tours
 * N : ?
 * mpi_rank : rank of the mpi process
 * arr_tours : an array of pointers to tours (the array must be allocated, each tour should not)
 * arr_cities : the master Cities structure
 */
void populate_tours(int N, int mpi_rank, tour_t** arr_tours, tour_t* arr_cities) {
	int i=0;
	while (i<MAX_POPULATION) {
		// the old tour generation
		arr_tours[i] = create_tour_nn(arr_cities->city[i%N], N, arr_cities);
		set_tour_fitness(arr_tours[i], N);
		i++;
	}
}// populate_tours()

#if BEST_TOUR_TRACKING
void initBestTourTracking()
{
	sizeBestTours = 0;
	lastBestTour = 0;
	BestTours = malloc(MAX_BEST_TOURS * sizeof(tour_t*));
	int i;
	for (i=0; i < MAX_BEST_TOURS; i++)
		BestTours[i] = malloc(sizeof(tour_t));
}

void dumpBestTours()
{
	FILE* out = fopen(BTT_FILE, "w");
	
	// header info
	fprintf(out, "%s\n%i\n", citiesFile, randSeed); // filename \n randSeed
	fprintf(out, "%i\n", sizeBestTours); // number of best tours
	
	// each tour in the list
	int i, k;
	tour_t* tour;
	for (i=0; i < sizeBestTours; i++)
	{
		tour = BestTours[i];
		fprintf(out, "%i: f%f %i", i, tour->fitness, tour->city[0]->id);
		for (k=1; k < tour->size; k++)
			fprintf(out, ",%i", tour->city[k]->id);
		fprintf(out, "\n");
	}
	
	fprintf(out, "\n");
	fclose(out);
}

/**
 * pass in current best tour for tracking
 */
void trackTours(tour_t* bestTour)
{
	if (bestTour != lastBestTour)
	{
		BestTours[sizeBestTours] = malloc(sizeOfTour(bestTour));
		memcpy(BestTours[sizeBestTours++], bestTour, sizeOfTour(bestTour));
		lastBestTour = bestTour;
	}
}
#endif // best tour tracking

int main(int argc, char** argv)
{
	startTime = time(0);
	int i; // loop counter
	int mpi_procs; // mpi rank (for each process) and number of processes
	char lcv = 1; // loop control variable for the while loop (run until lcv->0)
	
	intTours = 0;
	intCities = 0;

	//TODO: make argument handler set the number of procedures (mpi_procs) and mpi_flag.
	mpi_procs = 1;
	

	//####################################################
	// Argument Handler
	//####################################################
	// check number of parameters
	if (argc < 2)
	{
		printf("Usage: %s [flags] <filename of cities text document>\n", argv[0]);
		printf("Try -h or --help for more information.\n");
		terminate_program(1); // ERROR: must supply a filename for the cities
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
				printf("-t <tours file> : loads a file containing tours (must match your dataset).\n");
			}
			else if (strcmp(p, "-s") == 0)
			{
				// random seed
				randSeed = atoi(argv[++i]);
			}
			else if (strcmp(p, "-t") == 0)
			{
				// set tours file
				toursFile = argv[++i];
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
		terminate_program(3); // ERROR: no city file present
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
		DPRINTF("seed done.\n");
	}
	//----------------------------------------------------
	
	//####################################################
	// Load Cities, Initialize Tables, Create Init tours
	//####################################################
	// load the cities

	// Otherwise, just load the cities.
	DPRINTF("loading cities\n");
	load_cities(mpi_rank, citiesFile, &CitiesA);
	load_cities(mpi_rank, citiesFile, &CitiesB);
	
	// process the cities
	DPRINTF("Set up cities\n");
	int N = CitiesA->size;
	for (i=0; i < CitiesA->size; i++)
	{
		CitiesA->city[i]->tour = TOUR_A;
		CitiesB->city[i]->tour = TOUR_B;
	}
	// allocate memory for Tours and children_tours
	DPRINTF("Allocating tours array\n");
	Tours = malloc( sizeof(tour_t*) * MAX_POPULATION );
	DPRINTF("Allocating children tours array and each child tour struct.\n");
	childrenTours = malloc( sizeof(tour_t*) * MAX_PAIR_TOURS);
	for (i=0; i < MAX_PAIR_TOURS; i++)
		childrenTours[i] = malloc(sizeof(tour_t));
	

	// construct the distance table (on all processes)
#if USE_DISTANCE_TABLE
	construct_distTable(CitiesA,N);
#else
	DPRINTF("Not using a distance table.\n");
#endif


	// populate tours (on all processes)
	populate_tours(N,mpi_rank,Tours,CitiesA);
	sortTours(Tours,MAX_POPULATION);
	//----------------------------------------------------

	
#if BEST_TOUR_TRACKING
	initBestTourTracking();
#endif
	//----------------------------------------------------

	//####################################################
	// Print all tours to console.
	//####################################################
	for (i=0;i<MAX_POPULATION;i++) {
		print_tour(Tours[i]);
	}
	
	// TODO: send these best tours to a file.

	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}
