/**
 * main module for the TSP solver
 */

#include "include/tsp.h"
#include "include/eax.h"
#if MPIFLAG==1
	#include "mpi.h"
#endif


// "global" variables. I try to start these with capital letters
tour_t* Cities; // the "tour" that contains every city in their provided order. Not really a tour, just used as the master array of cities.
tour_t** Tours; // all of the current tours in the population

void populate_tours(int N, int mpi_rank, tour_t** arr_tours, tour_t* arr_cities) {
	int i;

	for (i=0;i<N;i++) {
		arr_tours[i] = create_tour_nn(arr_cities->city[i], N, arr_cities);
		set_tour_fitness(arr_tours[i], N);
	}
}

#ifdef DEBUG
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

void testTourArrayConversion(tour_t** t)
{

}
#endif

void MPI_init(char *mpi_flag, int *mpi_rank, int *mpi_procs, int *argc, char ***argv) {
	if (*mpi_flag>0) {
		// If running MPI, then use the MPI commands.
#if MPIFLAG
		MPI_Init(argc,argv);
		MPI_Comm_rank(MPI_COMM_WORLD,mpi_rank);
		MPI_Comm_size(MPI_COMM_WORLD,mpi_procs);
#endif
	} else {
		*mpi_rank = 0;
		*mpi_procs = 1;
	}
}

#if MPIFLAG
void master_listener(int *iter, int *delta_iter, char *lcv, tour_t** arr_tours, int mpi_procs) {
	// if you are within the constraints, perform actions
	if (*iter<MAX_ITERATIONS && *delta_iter<MAX_DELTA) {
		float delta_fit=0.0;
		MPI_Status status;
		tour_t** tempTours = malloc(MAX_CITIES * sizeof(tour_t*) * 5);

		// MPI send (tours back to each island)
		tour_t** bestTours = malloc(MAX_CITIES * sizeof(tour_t*) * 5);
		getBestTours(5, arr_tours, bestTours);
		int *intTours = malloc(MAX_CITIES * sizeof(int) * 5);
		tour_tToInt(bestTours, 5, intTours);
		int arraysize = MAX_CITIES * 5;
		MPI_Bcast(intTours, MAX_CITIES*5, MPI_INT, 0, MPI_COMM_WORLD);

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: note -- this will have to listen for ANY island & not just the sequence 1..num_procs
		// MPI receive (tours from each island)
		int i;
		for (i=1;i<mpi_procs;i++) {
			//TODO: does master always receive 5 tours?
			MPI_Recv(intTours, MAX_CITIES * sizeof(int) * MAX_TOUR, MPI_INT, i, MPI_TAG, MPI_COMM_WORLD, &status);
			intToTour_t(Cities, intTours, 5, tempTours);
			// udpate master population (sort it)
			mergeToursToPop(arr_tours, tempTours, 5);
		}

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

		// free memory
		free(tempTours);
		free(bestTours);
		free(intTours);
	}
	// Otherwise, order other processes to halt
	else {
		*lcv = 0;
		int *stoparray;
		stoparray = malloc(sizeof(int));
		stoparray[0]=-1;
		MPI_Bcast(stoparray, sizeof(int), MPI_INT, 0, MPI_COMM_WORLD); // 0 is the rank of master
		free(stoparray);
	}
}
#endif

void serial_listener(int *iter,int *delta_iter,char *lcv,tour_t** arr_tours, int N) {
	// if you are within the constraints, perform actions
	if (((*iter)<MAX_ITERATIONS) && ((*delta_iter)<MAX_DELTA)) {
		float delta_fit=0.0;

		// run the GA
		run_genalg(N,lcv,Tours,0);

		// First, grab the original best tour's fitness.
		delta_fit=arr_tours[0]->fitness;

		//TODO: udpate master population

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
		//TODO: MPI send cities
	}
	// otherwise...
	else {
		//TODO: MPI receive cities
	}
	DPRINTF("done! (loaded %i cities from the file)\n", Cities->size);
	// process the cities
	int N = Cities->size;
	construct_distTable(Cities,N);// compute distances as soon as we can (now)
	// output the distance table
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

#if DEBUG
	// run conversion tests
	testTourConversion();
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
	if (mpi_flag==0) {
		//TODO: Select parents for child creation (roulette wheel)

		// Create children
		performEAX(Cities, arr_tours[0], arr_tours[1], arr_tours[2]);
	} else {
#if MPIFLAG
		int *intTours;
		MPI_Status status;
		tour_t** tempTours = malloc(MAX_CITIES * sizeof(tour_t*) * 5);

		// notice -- the slaves will start by reciving tours.
		// this way, if we are to stop, master will send an empty tour.

		// MPI Recv new tours from master
		MPI_Recv(intTours, MAX_CITIES * sizeof(int) * MAX_TOUR, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status);

		// if the tour is not empty...
		if (intTours[0]!=-1) {
			// Udpate the island's population (sort it)
			intToTour_t(Cities, intTours, 5, tempTours);
			mergeToursToPop(arr_tours, tempTours, 5);

			//TODO: Select parents for child creation (roulette wheel)

			// Create children
			performEAX(Cities, Tours[0], Tours[1], Tours[2]);

			// MPI send (tours to master)
			getBestTours(5, arr_tours, tempTours);
			int *intTours = malloc(MAX_CITIES * sizeof(int) * 5);
			tour_tToInt(tempTours, 5, intTours);
			int arraysize = MAX_CITIES * 5;
			MPI_Send(intTours, MAX_CITIES*5, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD);
		} else {
			// else, set lcv->0
			*lcv=0;
		}

		// free memory
		free(tempTours);
		free(intTours);
#endif
	}
}

// global variables about the running state of the program
int randSeed = 0;
char* citiesFile = 0;

int main(int argc, char** argv)
{
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
	MPI_init(&mpi_flag,&mpi_rank,&mpi_procs,&argc,&argv);
	//----------------------------------------------------


	//####################################################
	// Load Cities, Initialize Tables, Create Init tours
	//####################################################
	// load the cities
	int *intCities = malloc(MAX_CITIES * sizeof(city_t*));
	if (mpi_flag==1) {
#if MPIFLAG
		MPI_Status status;

		// If MPI active, then let the master load cities and broadcast
		if (mpi_rank==0) {
			load_cities(mpi_rank,citiesFile,Cities);

			// MPI Send cities
			city_tToInt(Cities, Cities->size, intCities);
			MPI_Bcast(intCities, sizeof(intCities), MPI_INT, 0, MPI_COMM_WORLD);
		} else {
			// MPI Receive cities
			MPI_Recv(intCities, MAX_CITIES*sizeof(int)*3, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status); // size of intCities has x, y, id values.
			intToCity_t(intCities, Cities->size, Cities);
		}
#endif
	} else {
		// Otherwise, just load the cities.
		load_cities(mpi_rank,citiesFile,Cities);
	}
	free(intCities);
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
	//----------------------------------------------------


	//####################################################
	// Run Genetic Algorithm (Enter "The Islands")
	//####################################################
	int iter=0; // the number of iterations performed.
	int delta_iter=0; // the number of iterations performed with fitness consecutively within DELTA.
	while (lcv) {
		if (mpi_flag==1) {
#if MPIFLAG
			if (mpi_rank==0) {
				// if you are the master AND mpi is on, start listening
				master_listener(&iter,&delta_iter,&lcv,Tours,mpi_procs);
			}
			else {
				// if you are a slave and MPI is on, run the GA
				run_genalg(N,&lcv,Tours,mpi_flag);
			}
#endif
		} else {
			// otherwise, run the GA and perform the loop condition checks manually.
			run_genalg(N,&lcv,Tours,mpi_flag);
			serial_listener(&iter,&delta_iter,&lcv,Tours,N);
		}
	}
	//----------------------------------------------------


	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}

