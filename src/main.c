/**
 * main module for the TSP solver
 */

#include "include/tsp.h"
#include "include/eax.h"

// "global" variables. I try to start these with capital letters
tour_t *CitiesA, *CitiesB; // the "tour" that contains every city in their provided order. Not really a tour, just used as the master array of cities.
tour_t** Tours; // all of the current tours in the population
// global variables about the running state of the program
int randSeed = 0;
char* citiesFile = 0;
int mpi_rank = -1;

int intTours[MAX_CITIES*NUM_TOP_TOURS];
int intCities[MAX_CITIES*3];
tour_t** tempTours;
tour_t** bestTours;
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
		//arr_tours[i] = create_tour_nn(arr_cities->city[i%N], N, arr_cities);
		arr_tours[i] = create_tour_rand(arr_cities);
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

/**
 * initializes the MPI system and stuff
 */
void MPI_init(int *mpi_rank, int *mpi_procs, int *argc, char ***argv) {
	if (MPIFLAG>0) {
		// If running MPI, then use the MPI commands.
#if MPIFLAG
		MPI_Init(argc,argv);
		MPI_Comm_rank(MPI_COMM_WORLD,mpi_rank);
		MPI_Comm_size(MPI_COMM_WORLD,mpi_procs);
		DPRINTF("Just set up MPI on rank %i!\n",*mpi_rank);
#endif
	} else {
		*mpi_rank = 0;
		*mpi_procs = 1;
	}
}// MPI_init()

/**
 * runs the actual algorithm, including the GA steps and the performEAX on pairs of parents
 * memory_chunk : giant chunk of memory used for sub cycles
 * N : number of cities? or tours? I have no idea
 * lcv : no clue
 * arr_tours : array of pointers to the tours in the population
 */
void run_genalg(char* memory_chunk, int N, char *lcv, tour_t** arr_tours) {
	/////////////////////////////////////////////
	// pick 100 pairs of parent tours
	int i;
	tour_t *parentTourPop[MAX_PAIR_TOURS][2]; // the array of parent pairs [0] = parentA, [1] = parentB
	tour_t *children[MAX_PAIR_TOURS]; // use pointers to keep everything consistent (I know it's tacky, but it should be faster than malloc)
	tour_t temp2[MAX_PAIR_TOURS]; // allocate a bunch of tours on the stack to store the generated tours
#if PRINT_TOURS_DURING_MERGING
	DPRINTF("-- printing tours BEFORE anything -- \n");
	for (i=0; i < MAX_PAIR_TOURS; i++)
	{
		DPRINTF("m(\033[31m%i\033[0m)Tours[%i]: ", Tours[i], i);
		print_tour(Tours[i]);
	}
#endif
	// Select parents for child creation (roulette wheel)
	for (i=0;i<MAX_PAIR_TOURS;i++) {
		parentTourPop[i][0]=roulette_select(arr_tours, MAX_POPULATION, 0);
		parentTourPop[i][1]=roulette_select(arr_tours, MAX_POPULATION, parentTourPop[i][0]);
		children[i] = &temp2[i]; // fill up the array of pointers
	}

	if (MPIFLAG==0) {
		///////////////////////////////////////
		// run EAX on each pair of parents
		for (i=0;i<MAX_PAIR_TOURS;i++) {
			performEAX(memory_chunk, CitiesA, CitiesB, parentTourPop[i][0], parentTourPop[i][1], children[i]);
		}
		
		/////////////////////////////////////////
		// Merge the generated tours back into the population
#if PRINT_TOURS_DURING_MERGING
		DPRINTF("-- printing tours BEFORE merging -- \n");
		for (i=0; i < MAX_PAIR_TOURS; i++)
		{
			DPRINTF("m(\033[31m%i\033[0m)children[%i]: ", children[i], i);
			print_tour(children[i]);
		}
#endif
		mergeToursToPop(arr_tours, MAX_POPULATION, children, MAX_PAIR_TOURS);
#if PRINT_TOURS_DURING_MERGING
		DPRINTF("-- printing tours AFTER merging -- \n");
		for (i=0; i < MAX_PAIR_TOURS; i++)
		{
			DPRINTF("m(\033[31m%i\033[0m)arr_tours[%i]: ", arr_tours[i], i);
			print_tour(arr_tours[i]);
		}
#endif
	} else {
#if MPIFLAG
		MPI_Status status;

		// notice -- the slaves will start by reciving tours.
		// this way, if we are to stop, master will send an empty tour.

		// MPI Recv new tours from master
		DPRINTF("Island waiting to receive tours from master...\n");
		MPI_Recv(intTours, CitiesA->size * NUM_TOP_TOURS, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status);

		// if the tour is not empty...
		if (intTours[0]!=-1) {
			printf("OK! Island received non-empty tour.\n");
			// Udpate the island's population (sort it)
			intToTour_t(CitiesA, intTours, NUM_TOP_TOURS, tempTours);
			mergeToursToPop(arr_tours, MAX_POPULATION, tempTours, NUM_TOP_TOURS);
			
			// Generate children
			printf(">> NOW RUNNING EAX <<\n");
			///////////////////////////////////////
			// run EAX on each pair of parents
			for (i=0;i<MAX_PAIR_TOURS;i++) {
				performEAX(CitiesA, parentTourPop[i][0], parentTourPop[i][1], children[i]);
				mergeTourToPop(Tours, MAX_POPULATION, children[i]);
			}
			/////////////////////////////////////////
			printf(">> EXIT EAX <<\n");

			// MPI send (tours to master)
			getBestTours(NUM_TOP_TOURS, arr_tours, tempTours);
			tour_tToInt(tempTours, NUM_TOP_TOURS, intTours);
			DPRINTF("Island is sending its tours to master.\n");
			MPI_Send(intTours, CitiesA->size*NUM_TOP_TOURS, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD);
		} else {
			// else, set lcv->0
			DPRINTF("Island recognizes order and commences halt procedure.\n");
			*lcv=0;
		}
#endif
	}// else MPI
}// run_genalg()

#if MPIFLAG
// This function should only run when MPI is turned on!
void master_listener(int *iter, int *delta_iter, char *lcv, tour_t** arr_tours, int mpi_procs) {
	// if you are within the constraints, perform actions
	if ((*iter)<MAX_ITERATIONS && (*delta_iter)<MAX_DELTA) {
		float delta_fit=0.0;
		int i;
		MPI_Status status;

		// MPI send (tours back to each island)
		getBestTours(NUM_TOP_TOURS, arr_tours, bestTours);
		tour_tToInt(bestTours, NUM_TOP_TOURS, intTours);
		for (i=1;i<mpi_procs;i++) {
			DPRINTF("Master sends tours to %i\n",i);
			MPI_Send(intTours, CitiesA->size*NUM_TOP_TOURS, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);

		}

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: note -- this will have to listen for ANY island & not just the sequence 1..num_procs
		// MPI receive (tours from each island)
		for (i=1;i<mpi_procs;i++) {
			DPRINTF("Master waiting to receive tours from islands...\n");

			MPI_Recv(intTours, CitiesA->size * NUM_TOP_TOURS, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD, &status);
			intToTour_t(CitiesA, intTours, NUM_TOP_TOURS, tempTours);

			// udpate master population (sort it)
			mergeToursToPop(arr_tours, MAX_POPULATION, tempTours, NUM_TOP_TOURS);
		}
		DPRINTF("Master has received all updates from all islands!\n");

		// Next, subtract by the new best tour's fitness
		delta_fit-=arr_tours[0]->fitness;
		if (delta_fit<=DELTA) {
			// If you are within DELTA, increment counter
			(*delta_iter)++;
		} else {
			//Otherwise, reset counter
			*delta_iter=0;
		}

		// increment the iteration number.
		(*iter)++;
	}
	// Otherwise, order other processes to halt
	else {
		DPRINTF("OK, master is ordering the halt.\n");
		*lcv = 0;
		int stoparray=-1;
		
		int i;
		for (i=1;i<mpi_procs;i++) {
			DPRINTF("Master sends tours to %i\n",i);
			MPI_Send(&stoparray, 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
		}
	}
}// master_listener()
#endif

/**
 * used when the program is running on a single core, not using mpi
 * memory_chunk : that giant allocation of memory used for sub cycles
 * iter : current iteration, passed by reference
 * delta_iter : change in best fitness, passed by reference
 * lcv : I have no fucking clue, passed by reference
 * arr_tours : array of pointers to the master list of tours
 * N : number of tours?
 */
void serial_listener(char* memory_chunk, int *iter, int *delta_iter, char *lcv, tour_t** arr_tours, int N) {
	// if you are within the constraints, perform actions
	if (((*iter)<MAX_ITERATIONS) && ((*delta_iter)<MAX_DELTA)) {
		float delta_fit=0.0;

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		// run the GA (also updates population)
		run_genalg(memory_chunk, N, lcv, arr_tours);
		
		// print the best tour
#if PRINT_BEST_TOUR_EACH_ITERATION
		STRONG_TEXT;
		printf("Best Tour: ");
		print_tour(arr_tours[0]);
		NORMAL_TEXT;
#endif
#if BEST_TOUR_TRACKING
		trackTours(arr_tours[0]);
#endif

		// Next, subtract by the new best tour's fitness
		delta_fit-=arr_tours[0]->fitness;
		if (delta_fit<=DELTA) {
			// If you are within DELTA, increment counter
			(*delta_iter)++;
		} else {
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
	else {
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
}// serial_listener()

/**
 * loads cities from a file
 * mpi_rank : the rank of the mpi process
 * citiesFile : the string that is the file name
 * arr_cities : a pointer, passed by reference (so whatever pointer was passed in will now point to the cities loaded)
 */
void load_cities(int mpi_rank, char *citiesFile, tour_t **arr_cities) {
	// if master...
	if (mpi_rank==0) {
		// load the cities specified by the file
		DPRINTF("Loading cities...");
		*arr_cities = loadCities(citiesFile);
		if (!(*arr_cities))
		{
			printf("Error while loading cities. refer to error log? halting.\n");
			terminate_program(5); // ERROR: error while loading cities
		}
		DPRINTF("done! (loaded %i cities from the file)\n", (*arr_cities)->size);
	}
	DPRINTF("done! (loaded %i cities from the file)\n", (*arr_cities)->size);
	// process the cities
	int N = (*arr_cities)->size;
	construct_distTable(*arr_cities,N);// compute distances as soon as we can (now)
	// output the distance table
#if PRINT_DISTANCE_TABLE
	int x,y;
	printf(" -- DISTANCE TABLE --\n");
	printf("    ");
	for (x=0; x < N; x++)
		printf("  %02i ", x);
	printf("\n");
	for (y=0; y < N; y++)
	{
		printf("%02i :", y);
		for (x=0; x < N; x++)
			printf("%4.2f ", (y!=x)?lookup_distance(x, y):0);
		printf("\n");
	}
#endif

	// output the city information to the console
	DPRINTF("\nNum Cities: %04i\n", (*arr_cities)->size);
	DPRINTF("---------------------------\n");
	int i;
	for (i=0; i < (*arr_cities)->size; i++)
	{
		DPRINTF("City[%04i] at %04i, %04i   [id: %04i]\n", i, (*arr_cities)->city[i]->x, (*arr_cities)->city[i]->y, (*arr_cities)->city[i]->id);
	}
}// load_cities()

int main(int argc, char** argv)
{
	int i; // loop counter
	int mpi_procs; // mpi rank (for each process) and number of processes
	char lcv = 1; // loop control variable for the while loop (run until lcv->0)
	
	// oh boy, where to begin:
	// so, rather than do a bunch of mallocs inside of performEAX, or use some n^2 stack allocation (impossible)
	// we're going to allocate a giant chunk of memory on the heap that if necessary could fit the worst-case
	// storage required for the sub-cycles in the graph (a graph completely partitioned into sub-cycles of length 4)
	// and keep re-using that memory for the sub-cycles.
	// memory_chunk points to the beginning of this chunk of memory.
	// figure A:
	// [][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]
	// /\            /\                                                /\
	// |              |                                                 |
	// |              |                                                  \ cycles[2] ------------------------
	// |               \- cycles[1] --------------------------------------------------------|
	// |
	// |\- cycles[0] ------------------------------------------|
	// |
	//  \- memory_chunk
	//////////////////////////////////////////////////////////////////////
	// since the end of each tour_t stuct is the array of pointers to cities in the tour,
	// and if a sub-cycle is only of, say, length 5, then the remaining MAX_CITIES-5 cells
	// in the array are going to be useless memory. What this structure allows is for other
	// cycles to overlap into that unused section of cycles[0]'s memory, starting at the
	// first unused cell in their cities array.
	// Since the worst case is all sub-cycles of length 4, we know the maximum number of these
	// cycles to be MAX_CITIES/4, so we allocate space to hold that many tour_ts, anything other
	// than that will be smaller than that much memory, so we're in the clear.
	// this allows us to fit all of the subcycles into memory of O(N) where N is the number of cities.
	// I think this also assumes that a float is smaller or equal in size to an integer. (hopefully that's true!)
	// it also assumes that the C compiler doesn't do anything weird like pad memory between structs (hopefully it doesn't!)
	// it also assumes that Programmo, the god of programming, is with us and we can actually finish this code before the conference (hopefully he is!)
	int memory_chunk_size = (MAX_CITIES/4) * (sizeof(int)+sizeof(float)+sizeof(city_t*)*5); // *5 for 4 cities in the cycle PLUS the extra loop-back node
	DPRINTF("Allocating memory_chunk to be of size: %i\n", memory_chunk_size);
	char* memory_chunk = malloc(memory_chunk_size);
	DPRINTF("Allocated at memory_chunk = %x\n", memory_chunk);
	DPRINTF("memory_chunk itself is at &memory_chunk = %x\n", &memory_chunk);

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
	}
	//----------------------------------------------------

	//####################################################
	// MPI Initializations
	//####################################################
	// Handles MPI initializations and sets its variables.
	MPI_init(&mpi_rank,&mpi_procs,&argc,&argv);
	//----------------------------------------------------
	
	// Initialize the memory you will need for the entire program.
	tempTours = malloc(sizeof(tour_t*)*NUM_TOP_TOURS);
	for (i=0;i<NUM_TOP_TOURS;i++) {
		tempTours[i]=malloc(sizeof(tour_t));
	}
	bestTours = malloc(sizeof(tour_t*)*NUM_TOP_TOURS);
	
	//####################################################
	// Load Cities, Initialize Tables, Create Init tours
	//####################################################
	// load the cities

	int *intCities = malloc(MAX_CITIES * sizeof(int) * 3);
	if (MPIFLAG==1) {
#if MPIFLAG
		MPI_Status status;

		// If MPI active, then let the master load cities and broadcast
		if (mpi_rank==0) {
			load_cities(mpi_rank, citiesFile, &CitiesA);
			load_cities(mpi_rank, citiesFile, &CitiesB);

			// MPI Send cities
			city_tToInt(CitiesA, CitiesA->size, intCities);

			for (i=1;i<mpi_procs;i++) {
				DPRINTF("Master sends city size to %i\n",i);
				MPI_Send(&(CitiesA->size), 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
				DPRINTF("Master sends tours to %i\n",i);
				MPI_Send(intCities, CitiesA->size*3, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
			}

		} else {
			// MPI Receive cities
			DPRINTF("Island receiving citysize...\n");
			MPI_Recv(&(CitiesA->size), 1, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status);
			DPRINTF("Got city size of %i!\n",CitiesA->size);
			// receive actual array of cities as intarray
			DPRINTF("Island waiting for cities...\n");
			MPI_Recv(intCities, CitiesA->size*3, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status); // size of intCities has x, y, id values.
			DPRINTF("Island got cities!\n");
			// malloc the cities in the array
			for (i=0;i<CitiesA->size;i++) {
				CitiesA->city[i]=malloc(sizeof(city_t));
			}
			// convert the int array to a city array
			intToCity_t(intCities, CitiesA->size, CitiesA);
			intToCity_t(intCities, CitiesB->size, CitiesB);
			DPRINTF("Cities converted. Now printing...\n");
			print_tour(CitiesA);
		}
#endif
	} else {
		// Otherwise, just load the cities.
		load_cities(mpi_rank, citiesFile, &CitiesA);
		load_cities(mpi_rank, citiesFile, &CitiesB);
	}
	// process the cities
	int N = CitiesA->size;
	for (i=0; i < CitiesA->size; i++)
	{
		CitiesA->city[i]->tour = TOUR_A;
		CitiesB->city[i]->tour = TOUR_B;
	}
	// allocate memory for Tours
	Tours = malloc( sizeof(tour_t*) * MAX_POPULATION );

	// construct the distance table (on all processes)
	construct_distTable(CitiesA,N);

	// populate tours (on all processes)
	populate_tours(N,mpi_rank,Tours,CitiesA);
	//----------------------------------------------------

	
#if BEST_TOUR_TRACKING
	initBestTourTracking();
#endif
	//----------------------------------------------------

	//####################################################
	// Run Genetic Algorithm (Enter "The Islands")
	//####################################################
	int iter=0; // the number of iterations performed.
	int delta_iter=0; // the number of iterations performed with fitness consecutively within DELTA.
	while (lcv) {
		DPRINTF("Loop from %i...\n",mpi_rank);
		if (MPIFLAG==1 && mpi_procs>1) {
#if MPIFLAG
			if (mpi_rank==0) {
				// if you are the master AND mpi is on, start listening
				DPRINTF("Running listener on %i...\n",mpi_rank);
				master_listener(&iter,&delta_iter,&lcv,Tours,mpi_procs);
			}
			else {
				// if you are a slave and MPI is on, run the GA
				DPRINTF("Running GA on %i...\n",mpi_rank);
				run_genalg(memory_chunk, N, &lcv, Tours);
			}
#endif
		} else {
			// otherwise, run the GA and perform the loop condition checks manually.

			serial_listener(memory_chunk, &iter, &delta_iter, &lcv, Tours, N);

		}
	}
	//----------------------------------------------------

	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}
