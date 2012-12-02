////////////////////////////////////////////////////////////////////////////////
//	DESC:	Main module for the TSP solver.
////////////////////////////////////////////////////////////////////////////////

#include "include/tsp.h"
#include "include/eax.h"

//// GLOBAL VARIABLES //////////////////////////////////////////////////////////
tour_t *CitiesA, *CitiesB; /*	the "tour" that contains every city in their
						provided order. Not really a tour, just used as 
						the master array of cities. */
tour_t** Tours, **childrenTours; /*	all of the current tours
								in the population. */

// global variables about the running state of the program
int randSeed = 0;
char* citiesFile = 0;
char *toursFile = 0;
char* dataSet = 0;
char* outputPrefix = 0;
int mpi_rank = -1;
int numFileTours=0;

time_t startTime; // time the program started running.
int* intTours;
int* intCities;
tour_t** tempTours;
tour_t** bestTours;
////////////////////////////////////////////////////////////////////////////////


/*
 * DESC: Given a file name fileName containing a list of tours and the number
 * 	of tours nTours in the file, fill int array I with every value.
 *
 * Files consist of numFileTours, numCities, and the list of tours based on ID.
 *   Assumes that the file has less tours than MAX_POPULATION
 */
void loadTours(const char* const fileName, int* I, int *nTours)
{
	FILE* in;
	int numCities,numFileTours,i,j,index;
	
	// open the file
	in = fopen(fileName, "r");
	
	// verification
	if (!in)
	{
		fprintf(stderr, "Error opening %s for input. Check filename",
			fileName);
		exit(1);
	}

	// get number of tours/cities from first line
	fscanf(in, "%i %i", &numFileTours, &numCities); 
	
	printf("loadTours received %i for tours, %i for cities.\n",
		numFileTours,numCities);

	// set up the tours
	index=0;

	for (i=0;i<numFileTours;i++)
	{
		for (j=0;j<numCities-1;j++)
		{
			fscanf(in, "%i+", &I[index++]);
		}
		fscanf(in, "%i", &I[index++]);
	}
	*nTours = numFileTours;
}

/**
 * DESC: Fills the population of tours with an initial set of tours.
 *
 * N : size of each tour.
 * mpi_rank : rank of the mpi process
 * arr_tours : an array of pointers to tours (the array must be allocated, each tour should not)
 * arr_cities : the master Cities structure
 */
void populate_tours(int N, int mpi_rank, tour_t** arr_tours, tour_t* arr_cities,
 int* I, int *numFileTours)
{
	int i=0;

	while (i<MAX_POPULATION)
	{
#if USE_NEAREST_NEIGHBOR
#if USE_HYBRID_PROBABILITY
		// use hybrid
		arr_tours[i] = ((rand() % 100) > USE_HYBRID_PROBABILITY) ? 
			create_tour_rand(arr_cities) : 
			create_tour_nn(arr_cities->city[i%N], N, arr_cities);
#else
		// not hybrid
		arr_tours[i] = create_tour_nn(arr_cities->city[i%N], N, arr_cities);
#endif
#else
		// not nearest neighbor
		arr_tours[i] = create_tour_rand(arr_cities);
#endif
		set_tour_fitness(arr_tours[i], N);
		i++;
	}
	if (*numFileTours>0)
	{
		printf("converting the tours...\n");
		intToTour_t(arr_cities, I, *numFileTours, arr_tours);
	}

	for (i=0;i<*numFileTours;i++)
	{
		print_tour(arr_tours[i]);
	}
}


//// BEST TOUR TRACKING ////////////////////////////////////////////////////////
#if BEST_TOUR_TRACKING
void initBestTourTracking()
{
	
}

void dumpBestTours()
{
	
}

/**
 * DESC: Pass in current best tour for tracking.
 */
void trackTours(tour_t** allTours)
{
	static int iteration = 0;
	int i, a;
	
	// dump all of the current tours to disk
	char buffer[50];
	sprintf(buffer, "%s%s_%03i", "output/", outputPrefix, iteration++);
	FILE* dump = fopen(buffer, "w");
	
	// write it to disk
	fprintf(dump, "%i %i\n", NUM_TOP_TOURS, allTours[0]->size);
	for (i=0; i < NUM_TOP_TOURS; i++)
	{
		fprintf(dump, "%i", allTours[i]->city[0]->id);
		for (a=1; a < allTours[i]->size; a++)
		{
			fprintf(dump, "+%i", allTours[i]->city[a]->id);
		}// for each city
		fprintf(dump, "\n");
	}// for each tour
	
	// close the file
	fclose(dump);
	
	// submission script
#if SUBMIT_TO_SERVER
	sprintf(buffer, "%s %s output/%s_%03i", "python scripts/mapping.py",
		dataSet, outputPrefix, iteration-1);
	system(buffer);
#else
	OOPS_TEXT;
	printf("SUBMISSION_TO_SERVER IS TURNED OFF, NOT SUBMITTING TO SERVER.\n");
	NORMAL_TEXT;
#endif
}
#endif
//// END BEST TOUR TRACKING ////////////////////////////////////////////////////


/**
 * DESC: Initializes the MPI procedures/variables.
 */
void MPI_init(int *mpi_rank, int *mpi_procs, int *argc, char ***argv)
{
	if (MPIFLAG>0)
	{
		// If running MPI, then use the MPI commands.
#if MPIFLAG
		MPI_Init(argc,argv);
		MPI_Comm_rank(MPI_COMM_WORLD,mpi_rank);
		MPI_Comm_size(MPI_COMM_WORLD,mpi_procs);
		DPRINTF("Just set up MPI on rank %i!\n",*mpi_rank);
#endif
	}
	else
	{
		*mpi_rank = 0;
		*mpi_procs = 1;
	}
}

/**
 * DESC: Runs the Genetic Algorithm, including the GA steps and the performEAX
 *	on pairs of parents.
 * memory_chunk : giant chunk of memory used for sub cycles
 * lcv : when set to false (0), exits the main loop
 * arr_tours : array of pointers to the tours in the population
 */
void run_genalg(char* memory_chunk, char *lcv, tour_t** arr_tours,
 tour_t** arr_children, int *mpi_procs)
{
	// pick 100 pairs of parent tours
	int i;
	tour_t *parentTourPop[MAX_PAIR_TOURS][2]; /*	the array of parent pairs
										[0]=parentA, [1]=parentB */

#if PRINT_TOURS_DURING_MERGING
	DPRINTF("-- printing tours BEFORE anything -- \n");
	for (i=0; i < MAX_PAIR_TOURS; i++)
	{
		DPRINTF("m(\033[31m%i\033[0m)Tours[%i]: ", Tours[i], i);
		print_tour(Tours[i]);
	}
#endif

	// Select parents for child creation (roulette wheel)
	for (i=0;i<MAX_PAIR_TOURS;i++)
	{
		parentTourPop[i][0]=roulette_select(arr_tours, MAX_POPULATION, 0);
		parentTourPop[i][1]=roulette_select(arr_tours, MAX_POPULATION,
			parentTourPop[i][0]);
	}

	if (MPIFLAG==0)
	{
#if PRINT_ITERATION_PROGRESS
		int progressMultiple = MAX_PAIR_TOURS / 10;
		DPRINTF("Progress (starting at %i): \n", time(0));
#endif
		// run EAX on each pair of parents
		for (i=0;i<MAX_PAIR_TOURS;i++)
		{
			performEAX(memory_chunk, CitiesA, CitiesB, parentTourPop[i][0],
				parentTourPop[i][1], arr_children[i]);
#if PRINT_ITERATION_PROGRESS
			if (i % progressMultiple == 0)
			{
				DPRINTF("%i percent at %i\n",
					(i / progressMultiple) * 10, time(0));
			}
		}
		DPRINTF("...iteration finished!\n");
#else
		}
#endif

#if PRINT_TOURS_DURING_MERGING
		DPRINTF("-- printing tours BEFORE merging -- \n");
		for (i=0; i < MAX_PAIR_TOURS; i++)
		{
			DPRINTF("m(\033[31m%i\033[0m)children[%i]: ",
				arr_children[i], i);
			print_tour(arr_children[i]);
		}
#endif

		// Merge the generated tours back into the population
		mergeToursToPop(arr_tours, MAX_POPULATION, arr_children,
			MAX_PAIR_TOURS);

#if PRINT_TOURS_DURING_MERGING
		DPRINTF("-- printing tours AFTER merging -- \n");
		for (i=0; i < MAX_PAIR_TOURS; i++)
		{
			DPRINTF("m(\033[31m%i\033[0m)arr_tours[%i]: ", arr_tours[i], i);
			print_tour(arr_tours[i]);
		}
#endif

	}
	else
	{
#if MPIFLAG
		MPI_Status status;
		// notice -- the slaves will start by reciving tours.
		// this way, if we are to stop, master will send an empty tour.

		// MPI Recv new tours from master
		if (*mpi_procs>1)
		{
			DPRINTF("Island %i waiting to receive tours from master...\n",
				mpi_rank);
			MPI_Recv(intTours, CitiesA->size * NUM_TOP_TOURS, MPI_INT, 0,
				MPI_TAG, MPI_COMM_WORLD, &status);
		}

		// if the tour is not empty...
		if (intTours[0]!=-1)
		{
			if (*mpi_procs>1)
			{
				printf("OK! Island %i eceived non-empty tour.\n", mpi_rank);
				// Udpate the island's population (sort it)
				intToTour_t(CitiesA, intTours, NUM_TOP_TOURS, tempTours);
				mergeToursToPop(arr_tours, MAX_POPULATION, tempTours,
					NUM_TOP_TOURS);
			}
			
			// Generate children
			// run EAX on each pair of parents
#if PRINT_ITERATION_PROGRESS
			int progressMultiple = MAX_PAIR_TOURS / 10;
			DPRINTF("Progress (starting at %i): \n", time(0));
#endif
			for (i=0;i<MAX_PAIR_TOURS;i++)
			{
				performEAX(memory_chunk, CitiesA, CitiesB,
					parentTourPop[i][0], parentTourPop[i][1],
					arr_children[i]);
				mergeTourToPop(Tours, MAX_POPULATION, arr_children[i]);
#if PRINT_ITERATION_PROGRESS
				if (i % progressMultiple == 0)
				{
					DPRINTF("%i percent at %i\n",
						(i / progressMultiple) * 10, time(0));
				}
			}
			DPRINTF("...doneEAX!\n");
#else
			}
#endif
			// MPI send (tours to master)
			if (*mpi_procs>1)
			{
				getBestTours(SEND_TO_MASTER, arr_tours, tempTours);
				tour_tToInt(tempTours, SEND_TO_MASTER, intTours);
				DPRINTF("Island %i is sending its tours to master.\n",
					mpi_rank);
				MPI_Send(intTours, CitiesA->size*SEND_TO_MASTER, MPI_INT, 0,
					MPI_TAG, MPI_COMM_WORLD);
			}
		}
		else
		{
			// else, set lcv->0
			DPRINTF("Island %i commencing halt procedure.\n", mpi_rank);
			*lcv=0;
		}
#endif
	}
}

#if MPIFLAG
/**
 * DESC: Function run by master for listening for the slaves to finish their
 *	computations.
 *
 * NOTE: this function should only run when MPI is turned on
 */
void master_listener(int *iter, int *delta_iter, char *lcv, tour_t** arr_tours,
 int mpi_procs, tour_t** childrenTours)
{
	// if you are within the constraints, perform actions
	if ((*iter)<MAX_ITERATIONS && (*delta_iter)<MAX_DELTA)
	{
		float delta_fit=0.0;
		int i;
		MPI_Status status;

		// print the best tour
#if PRINT_BEST_TOUR_EACH_ITERATION
		STRONG_TEXT;
		printf("Best Tour: ");
		print_tour(arr_tours[0]);
		NORMAL_TEXT;
#endif

		// MPI send (tours back to each island)
		getBestTours(NUM_TOP_TOURS, arr_tours, bestTours);
		tour_tToInt(bestTours, NUM_TOP_TOURS, intTours);
		for (i=1;i<mpi_procs;i++)
		{
			DPRINTF("Master sends tours to %i\n",i);
			MPI_Send(intTours, CitiesA->size*NUM_TOP_TOURS, MPI_INT, i,
				MPI_TAG, MPI_COMM_WORLD);

		}

#if BEST_TOUR_TRACKING
		trackTours(arr_tours);
#endif

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: Make asynchronous
		// MPI receive (tours from each island)
		for (i=1;i<mpi_procs;i++)
		{
			DPRINTF("Master waiting to receive tours from islands...\n");

			MPI_Recv(intTours, CitiesA->size * SEND_TO_MASTER, MPI_INT, i,
				MPI_TAG, MPI_COMM_WORLD, &status);
			intToTour_t(CitiesA, intTours, SEND_TO_MASTER, tempTours);

			mergeToursToPop(arr_tours, MAX_POPULATION, tempTours,
				SEND_TO_MASTER);
		}
		DPRINTF("Master has received all updates from all islands!\n");

		// Next, subtract by the new best tour's fitness
		delta_fit-=arr_tours[0]->fitness;
		if (delta_fit<=DELTA)
		{
			// If you are within DELTA, increment counter
			(*delta_iter)++;
		}
		else
		{
			//Otherwise, reset counter
			*delta_iter=0;
		}
		// increment the iteration number.
		(*iter)++;
	}
	// Otherwise, order other processes to halt
	else
	{
		int stoparray=-1,i;
		DPRINTF("OK, master is ordering the halt.\n");
		*lcv = 0;
		for (i=1;i<mpi_procs;i++)
		{
			DPRINTF("Master sends tours to %i\n",i);
			MPI_Send(&stoparray, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
		}
	}
}
#endif

/**
 * DESC: Used when the program is running on a single core (i.e. not using MPI)
 *
 * memory_chunk : that giant allocation of memory used for sub cycles
 * iter : current iteration, passed by reference
 * delta_iter : change in best fitness, passed by reference
 * lcv : when set to false (0), exits the main loop
 * arr_tours : array of pointers to the master list of tours
 * childrenTours : an array of pointers to already allocated tour structs
 * 	we can use to store children inside.
 */
void serial_listener(char* memory_chunk, int *iter, int *delta_iter, char *lcv,
 tour_t** arr_tours, tour_t** childrenTours, int *mpi_procs)
{
	// if you are within the constraints, perform actions
	DPRINTF("SERIAL LISTENER!");
	if (((*iter)<MAX_ITERATIONS) && ((*delta_iter)<MAX_DELTA))
	{
		float delta_fit=0.0;

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		// run the GA (also updates population)
		DPRINTF("RUN GENALG");
		run_genalg(memory_chunk, lcv, arr_tours, childrenTours,mpi_procs);
		
		// print the best tour
#if PRINT_BEST_TOUR_EACH_ITERATION
		STRONG_TEXT;
		printf("Best Tour: ");
		print_tour(arr_tours[0]);
		NORMAL_TEXT;
#endif
#if BEST_TOUR_TRACKING
		trackTours(arr_tours);
#endif

		// Next, subtract by the new best tour's fitness
		delta_fit-=arr_tours[0]->fitness;
		if (delta_fit<=DELTA)
		{
			// If you are within DELTA, increment counter
			(*delta_iter)++;
		}
		else
		{
			//Otherwise, reset counter
			(*delta_iter)=0;
		}

		// increment the iteration number.
		(*iter)++;
		STRONG_TEXT;
		DPRINTF("**************  Iteration %i *****************\n", *iter);
		NORMAL_TEXT;
	}
	// Otherwise, order other processes to halt
	else
	{
		(*lcv) = 0;
		OOPS_TEXT;
		DPRINTF("Exiting because ");
		if ((*delta_iter)<MAX_DELTA)	
		{
			DPRINTF("of too many iterations!\n");
		}
		else
		{
			DPRINTF("the tours converged!\n");
		}
		NORMAL_TEXT;
	}
}

/**
 * DESC: main method for the entire program. This is what gets called first.
 */
int main(int argc, char** argv)
{
	startTime = time(0);
	int i; // loop counter
	int mpi_procs; // mpi rank (for each process) and number of processes
	char lcv = 1; /* 	loop control variable for the while loop
					(run until lcv->0) */
	citiesFile = 0;
	
#if MPIFLAG
	intTours = malloc(MAX_CITIES*NUM_TOP_TOURS*sizeof(int));
	intCities = malloc(MAX_CITIES*3*sizeof(int));
	MPI_Status status;
#else
	intTours = 0;
	intCities = 0;
#endif
	
	// see "memory chunk description" at the bottom of tsp.h for documentation.
	int memory_chunk_size =
		(MAX_CITIES/4) * (sizeof(int)+sizeof(float)+sizeof(city_t*)*5);
		/* *5 for 4 cities in the cycle PLUS the extra loop-back node */
	DPRINTF("Allocating memory_chunk to be of size: %i\n", memory_chunk_size);
	char* memory_chunk = malloc(memory_chunk_size);
	DPRINTF("Allocated at memory_chunk = %x\n", memory_chunk);
	DPRINTF("memory_chunk itself is at &memory_chunk = %x\n", &memory_chunk);

	//set mpi_procs to 1 incase we're not using mpi, but it really gets set in MPI_INIT
	mpi_procs = 1;

	//####################################################
	// Argument Handler
	//####################################################
	// check number of parameters
	if (argc < 2)
	{
		printf("Usage: %s [flags] <filename of cities text document>\n",
			argv[0]);
		printf("Try -h or --help for more information.\n");
		terminate_program(1); // ERROR: must supply a filename for the cities
	}
	else
	{
		for (i=1; i < argc; i++)
		{
			char* p = argv[i];
			if (strcmp(p, "-h") == 0 || strcmp(p, "-H") == 0 ||
				strcmp(p, "--help") == 0 || strcmp(p, "--HELP") == 0)
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
				printf("-d <dataSetPath> : this is the path of the dataset that is passed into the conversion submission.\n");
				printf("-o <outputPrefix> : prefix to output files during iterations (the tour dumps).\n");
			}
			else if (strcmp(p, "-d") == 0)
			{
				// dataSet
				dataSet = argv[++i];
			}
			else if (strcmp(p, "-o") == 0)
			{
				// outputPrefix
				outputPrefix = argv[++i];
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
	
	//####################################################
	// MPI Initializations
	//####################################################
	// Handles MPI initializations and sets its variables.
	printf("MPI_INIT...\n");
	MPI_init(&mpi_rank,&mpi_procs,&argc,&argv);
	
	// check to make sure we got a city file
	if (!citiesFile)
	{
		printf("no city file present. halting\n");
		terminate_program(3); // ERROR: no city file present
	}
	// init dataSet
	if (!dataSet)
	{
		OOPS_TEXT;
		printf("No data set name loaded.\n");
		NORMAL_TEXT;
		dataSet = "NO_DATA_SET";
	}
	else
	{
		printf("DatSet: '%s'\n", dataSet);
	}
	// init outputPrefix
	if (!outputPrefix)
	{
		OOPS_TEXT;
		printf("No data set name loaded.\n");
		NORMAL_TEXT;
		outputPrefix = "NONAME";
	}
	else
	{
		printf("outputPrefix: '%s'\n", outputPrefix);
	}
	// initialize srand
	if (randSeed)
	{
		randSeed += mpi_rank*3;
		DPRINTF("Using \033[31m%i\033[0m as random seed.\n", randSeed);
		srand(randSeed);
	}
	else // otherwise use a random seed
	{
		randSeed = time(0);
		randSeed += mpi_rank*3;
		DPRINTF("Picked a random seed (\033[31m%i\033[0m).\n", randSeed);
		srand(randSeed);
	}
	printf("Using '%s' for cities and '%s' for initial tours.\n",
		citiesFile, toursFile);
	
	// Initialize the memory you will need for the entire program.
	tempTours = malloc(sizeof(tour_t*)*NUM_TOP_TOURS);
	for (i=0;i<NUM_TOP_TOURS;i++)
	{
		tempTours[i]=malloc(sizeof(tour_t));
	}
	bestTours = malloc(sizeof(tour_t*)*NUM_TOP_TOURS);
	
	//####################################################
	// Load Cities, Initialize Tables, Create Init tours
	//####################################################
	// load the cities
	int *intCities = malloc(MAX_CITIES * sizeof(int) * 3);
	if (MPIFLAG==1)
	{
#if MPIFLAG

		// If MPI active, then let the master load cities and broadcast
		if (mpi_rank==0)
		{
			load_cities(mpi_rank, citiesFile, &CitiesA);
			load_cities(mpi_rank, citiesFile, &CitiesB);

			// MPI Send cities
			city_tToInt(CitiesA, CitiesA->size, intCities);

			for (i=1;i<mpi_procs;i++)
			{
				DPRINTF("Master sends city size to %i\n",i);
				MPI_Send(&(CitiesA->size), 1, MPI_INT, i, MPI_TAG,
					MPI_COMM_WORLD);
				DPRINTF("Master sends tours to %i\n",i);
				MPI_Send(intCities, CitiesA->size*3, MPI_INT, i, MPI_TAG,
					MPI_COMM_WORLD);
			}
		}
		else
		{
			// MPI Receive cities
			CitiesA = malloc(sizeof(tour_t));
			CitiesB = malloc(sizeof(tour_t));
			DPRINTF("Island %i receiving citysize...CitiesA: %x ->size: %i\n"
				, mpi_rank, CitiesA, (CitiesA?CitiesA->size:-1));
			MPI_Recv(&(CitiesA->size), 1, MPI_INT, 0, MPI_TAG,
				MPI_COMM_WORLD, &status);
			CitiesB->size = CitiesA->size;
			DPRINTF("%i Got city size of %i!\n", mpi_rank, CitiesA->size);
			// receive actual array of cities as intarray
			DPRINTF("Island %i waiting for cities...\n", mpi_rank);
			// size of intCities has x, y, id values.
			MPI_Recv(intCities, CitiesA->size*3, MPI_INT, 0, MPI_TAG,
				MPI_COMM_WORLD, &status);
			DPRINTF("Island %i got cities!\n", mpi_rank);

			// malloc the cities in the array
			for (i=0;i<CitiesA->size;i++)
			{
				CitiesA->city[i]=malloc(sizeof(city_t));
				CitiesB->city[i]=malloc(sizeof(city_t));
			}
			// convert the int array to a city array
			intToCity_t(intCities, CitiesA->size, CitiesA);
			intToCity_t(intCities, CitiesB->size, CitiesB);
			DPRINTF("%i Cities converted. Now printing...\n", mpi_rank);
			print_tour(CitiesA);
			print_tour(CitiesB);
			DPRINTF("%i Done printing!\n", mpi_rank);
		}
#endif
	}
	else
	{
		// Otherwise, just load the cities.
		load_cities(mpi_rank, citiesFile, &CitiesA);
		load_cities(mpi_rank, citiesFile, &CitiesB);
	}
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
	DPRINTF("Table constructed.\n");
#else
	DPRINTF("Not using a distance table.\n");
#endif

	// first, try to load tours.
	int *I = 0;
	if (toursFile) {
		I = malloc(sizeof(int)*MAX_CITIES*MAX_POPULATION);
		if (MPIFLAG)
		{
#if MPIFLAG
			if (mpi_rank==0)
			{
				loadTours(toursFile,I,&numFileTours);
				for (i=1;i<mpi_procs;i++)
				{
					DPRINTF("Master sends number of inputTours to %i\n",i);
					MPI_Send(&numFileTours, 1, MPI_INT, i, MPI_TAG,
					MPI_COMM_WORLD);
					DPRINTF("Master sends inputTours array to %i\n",i);
					MPI_Send(I, CitiesA->size*numFileTours, MPI_INT, i,
						MPI_TAG, MPI_COMM_WORLD);
				}
			}
			else
			{
				MPI_Recv(&numFileTours, 1, MPI_INT, 0, MPI_TAG,
					MPI_COMM_WORLD, &status);
				MPI_Recv(I, CitiesA->size*numFileTours, MPI_INT, 0, MPI_TAG,
					MPI_COMM_WORLD, &status);
			}
#endif
		}
		else loadTours(toursFile,I,&numFileTours);
	}
	populate_tours(N,mpi_rank,Tours,CitiesA, I, &numFileTours);
	DPRINTF("Tours Populated...\n");
	if (I) free(I);
	sortTours(Tours,MAX_POPULATION);
	
#if BEST_TOUR_TRACKING
	initBestTourTracking();
#endif

	//####################################################
	// Run Genetic Algorithm (Enter "The Islands")
	//####################################################
	int iter=0; // the number of iterations performed.
	int delta_iter=0; /*	the number of iterations performed with fitness
						consecutively within DELTA. */
	while (lcv)
	{
		DPRINTF("Loop from %i...\n",mpi_rank);
		if (MPIFLAG==1 && mpi_procs>1)
		{
#if MPIFLAG
			if (mpi_rank==0)
			{
				// if you are the master AND mpi is on, start listening
				DPRINTF("Running listener on %i...\n",mpi_rank);
				master_listener(&iter,&delta_iter,&lcv,Tours,mpi_procs,
					childrenTours);
			}
			else
			{
				// if you are a slave and MPI is on, run the GA
				DPRINTF("Running GA on %i...\n",mpi_rank);
				run_genalg(memory_chunk, &lcv, Tours, childrenTours,
					&mpi_procs);
			}
#endif
		}
		else
		{
			// otherwise, run the GA and perform the loop condition checks manually.
			serial_listener(memory_chunk, &iter, &delta_iter, &lcv, Tours,
				childrenTours, &mpi_procs);
		}// else serial execution
	}// while lcv

	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}
