/**
 * main module for the TSP solver
 */

#include "include/tsp.h"
#include "include/eax.h"

// "global" variables. I try to start these with capital letters
tour_t* Cities; // the "tour" that contains every city in their provided order. Not really a tour, just used as the master array of cities.
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


void populate_tours(int N, int mpi_rank, tour_t** arr_tours, tour_t* arr_cities) {
	int i=0;
	while (i<MAX_POPULATION) {
		// the old tour generation
		//arr_tours[i] = create_tour_nn(arr_cities->city[i%N], N, arr_cities);
		arr_tours[i] = create_tour_rand(arr_cities);
		set_tour_fitness(arr_tours[i], N);
		i++;
	}
}

#if DEBUG
void testTourConversion()
{
	tour_t cities;
	memcpy(&cities, Cities, sizeof(*Cities));
	int citiesInts[MAX_CITIES*3];

	printf("Converting to ints\n");
	city_tToInt(&cities, cities.size, &citiesInts);
	// verify that they're all equal
	int i;
	for (i=0; i < cities.size; i++)
	{
		city_t* city = cities.city[i];
		if (city->x != citiesInts[i*3] || city->y != citiesInts[i*3+1] || city->id != citiesInts[i*3+2])
			printf("(converting to):City invalid city: {%i, %i, %i} cityInt: {%i, %i, %i}\n", city->x, city->y, city->id, citiesInts[i*3], citiesInts[i*3+1], citiesInts[i*3+2]);
	}

	printf("Converting to cities\n");
	intToCity_t(&citiesInts, cities.size, &cities);
	// verify that they're all equal
	for (i=0; i < cities.size; i++)
	{
		city_t* city = cities.city[i];
		if (city->x != citiesInts[i*3] || city->y != citiesInts[i*3+1] || city->id != citiesInts[i*3+2])
			printf("(converting back): City invalid city: {%i, %i, %i} cityInt: {%i, %i, %i}\n", city->x, city->y, city->id, citiesInts[i*3], citiesInts[i*3+1], citiesInts[i*3+2]);
	}
}
#endif

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
		BestTours[sizeBestTours] = malloc(sizeof(tour_t));
		memcpy(BestTours[sizeBestTours++], bestTour, sizeof(tour_t));
		lastBestTour = bestTour;
	}
}
#endif // best tour tracking

void MPI_init(char *mpi_flag, int *mpi_rank, int *mpi_procs, int *argc, char ***argv) {
	if (*mpi_flag>0) {
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
}

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
			MPI_Send(intTours, Cities->size*NUM_TOP_TOURS, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
		}

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: note -- this will have to listen for ANY island & not just the sequence 1..num_procs
		// MPI receive (tours from each island)
		for (i=1;i<mpi_procs;i++) {
			DPRINTF("Master waiting to receive tours from islands...\n");
			MPI_Recv(intTours, Cities->size * NUM_TOP_TOURS, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD, &status);
			intToTour_t(Cities, intTours, NUM_TOP_TOURS, tempTours);
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
}
#endif

void serial_listener(int *iter, int *delta_iter, char *lcv, tour_t** arr_tours, int N) {
	// if you are within the constraints, perform actions
	if (((*iter)<MAX_ITERATIONS) && ((*delta_iter)<MAX_DELTA)) {
		float delta_fit=0.0;

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		// run the GA (also updates population)
		run_genalg(N, lcv, arr_tours, 0);
		
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
	}
	DPRINTF("done! (loaded %i cities from the file)\n", Cities->size);
	// process the cities
	int N = Cities->size;
	construct_distTable(Cities,N);// compute distances as soon as we can (now)
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
	DPRINTF("\nNum Cities: %04i\n", Cities->size);
	DPRINTF("---------------------------\n");
	int i;
	for (i=0; i < Cities->size; i++)
	{
		DPRINTF("City[%04i] at %04i, %04i   [id: %04i]\n", i, Cities->city[i]->x, Cities->city[i]->y, Cities->city[i]->id);
	}
}

void run_genalg(int N, char *lcv, tour_t** arr_tours, int mpi_flag) {
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

	if (mpi_flag==0) {
		///////////////////////////////////////
		// run EAX on each pair of parents
		for (i=0;i<MAX_PAIR_TOURS;i++) {
			performEAX(Cities, parentTourPop[i][0], parentTourPop[i][1], children[i]);
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
		MPI_Recv(intTours, Cities->size * NUM_TOP_TOURS, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status);

		// if the tour is not empty...
		if (intTours[0]!=-1) {
			printf("OK! Island received non-empty tour.\n");
			// Udpate the island's population (sort it)
			intToTour_t(Cities, intTours, NUM_TOP_TOURS, tempTours);
			mergeToursToPop(arr_tours, MAX_POPULATION, tempTours, NUM_TOP_TOURS);
			
			// Generate children
			printf(">> NOW RUNNING EAX <<\n");
			///////////////////////////////////////
			// run EAX on each pair of parents
			for (i=0;i<MAX_PAIR_TOURS;i++) {
				performEAX(Cities, parentTourPop[i][0], parentTourPop[i][1], children[i]);
				mergeTourToPop(Tours, MAX_POPULATION, children[i]);
			}
			/////////////////////////////////////////
			printf(">> EXIT EAX <<\n");

			// MPI send (tours to master)
			getBestTours(NUM_TOP_TOURS, arr_tours, tempTours);
			tour_tToInt(tempTours, NUM_TOP_TOURS, intTours);
			DPRINTF("Island is sending its tours to master.\n");
			MPI_Send(intTours, Cities->size*NUM_TOP_TOURS, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD);
		} else {
			// else, set lcv->0
			DPRINTF("Island recognizes order and commences halt procedure.\n");
			*lcv=0;
		}
#endif
	}// else MPI
}// run_genalg()


int main(int argc, char** argv)
{
	int i; // loop counter
	char mpi_flag; // mpi is on (1) or off (0)
	int mpi_procs; // mpi rank (for each process) and number of processes
	char lcv = 1; // loop control variable for the while loop (run until lcv->0)
	mpi_flag = MPIFLAG;
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
	MPI_init(&mpi_flag,&mpi_rank,&mpi_procs,&argc,&argv);
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
	if (mpi_flag==1) {
#if MPIFLAG
		MPI_Status status;

		// If MPI active, then let the master load cities and broadcast
		if (mpi_rank==0) {
			load_cities(mpi_rank,citiesFile,Cities);

			// MPI Send cities
			city_tToInt(Cities, Cities->size, intCities);
			print_tour(Cities);
			for (i=1;i<mpi_procs;i++) {
				DPRINTF("Master sends city size to %i\n",i);
				MPI_Send(&(Cities->size), 1, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
				DPRINTF("Master sends tours to %i\n",i);
				MPI_Send(intCities, Cities->size*3, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD);
			}

		} else {
			// MPI Receive cities
			Cities = (tour_t*)malloc(sizeof(tour_t));
			DPRINTF("Island receiving citysize...\n");
			MPI_Recv(&(Cities->size), 1, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status);
			DPRINTF("Got city size of %i!\n",Cities->size);
			DPRINTF("Island waiting for cities...\n");
			MPI_Recv(intCities, Cities->size*3, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status); // size of intCities has x, y, id values.
			DPRINTF("Island got cities!\n");
			for (i=0;i<Cities->size;i++) {
				Cities->city[i]=malloc(sizeof(city_t));
			}
			intToCity_t(intCities, Cities->size, Cities);
			DPRINTF("Cities converted. Now printing...\n");
			print_tour(Cities);
		}
#endif
	} else {
		// Otherwise, just load the cities.
		load_cities(mpi_rank,citiesFile,Cities);
	}
	// process the cities
	int N = Cities->size;
	// allocate memory for Tours
	Tours = malloc( sizeof(tour_t*) * MAX_POPULATION );

	// construct the distance table (on all processes)
	construct_distTable(Cities,N);

	// populate tours (on all processes)
	populate_tours(N,mpi_rank,Tours,Cities);
	
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
		if (mpi_flag==1 && mpi_procs>1) {
#if MPIFLAG
			if (mpi_rank==0) {
				// if you are the master AND mpi is on, start listening
				DPRINTF("Running listener on %i...\n",mpi_rank);
				master_listener(&iter,&delta_iter,&lcv,Tours,mpi_procs);
			}
			else {
				// if you are a slave and MPI is on, run the GA
				DPRINTF("Running GA on %i...\n",mpi_rank);
				run_genalg(N,&lcv,Tours,mpi_flag);
			}
#endif
		} else {
			// otherwise, run the GA and perform the loop condition checks manually.

			serial_listener(&iter,&delta_iter,&lcv,Tours,N);
		}
	}
	//----------------------------------------------------



	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}
