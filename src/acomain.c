/**
 * Tries to solve TSP using ACO
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
int mpi_rank = -1;
int numFileTours=0;

time_t startTime; // time the program started running
int* intTours;
int* intCities;
tour_t** tempTours;
tour_t** bestTours;

/*
 * Files consist of numFileTours, numCities, and the list of tours based on ID.
 *   Assumes that the file has less tours than MAX_POPULATION
 */
void loadTours(const char* const fileName, int* I, int *nTours) {
	FILE* in;
	int numCities,numFileTours,i,j,index;
	
	// open the file
	in = fopen(fileName, "r");
	
	// verification
	if (!in)
	{
		fprintf(stderr, "Error opening %s for input. Check filename", fileName);
		exit(1);
	}

	// get number of tours/cities from first line
	fscanf(in, "%i %i", &numFileTours, &numCities); 
	
	printf("loadTours received %i for tours, %i for cities.\n",numFileTours,numCities);

	// set up the tours
	index=0;

	for (i=0;i<numFileTours;i++) {
		for (j=0;j<numCities-1;j++) {
			fscanf(in, "%i+", &I[index++]);
		}
		fscanf(in, "%i", &I[index++]);
	}
	*nTours = numFileTours;
}


#if BEST_TOUR_TRACKING
void initBestTourTracking()
{
	
}

void dumpBestTours()
{
	
}

/**
 * pass in current best tour for tracking
 */
void trackTours(tour_t** allTours)
{
	static int iteration = 0;
	int i, a;
	
	// dump all of the current tours to disk
	char buffer[50];
	sprintf(buffer, "%s%03i", "output/iteration", iteration++);
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
	sprintf(buffer, "%s %s output/iteration%03i", "scripts/submit.sh", "DATA_SET", iteration-1);
	system(buffer);
#else
	OOPS_TEXT;
	printf("SUBMISSION_TO_SERVER IS TURNED OFF, NOT SUBMITTING TO SERVER.\n");
	NORMAL_TEXT;
#endif
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



int main(int argc, char** argv)
{
	startTime = time(0);
	int i; // loop counter
	int mpi_procs; // mpi rank (for each process) and number of processes
	char lcv = 1; // loop control variable for the while loop (run until lcv->0)
	citiesFile = 0;
	
#if MPIFLAG
	intTours = malloc(MAX_CITIES*NUM_TOP_TOURS*sizeof(int));
	intCities = malloc(MAX_CITIES*3*sizeof(int));
	MPI_Status status;
#else
	intTours = 0;
	intCities = 0;
#endif
	
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

	//set mpi_procs to 1 incase we're not using mpi, but it really gets set in MPI_INIT
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
	
	//####################################################
	// MPI Initializations
	//####################################################
	// Handles MPI initializations and sets its variables.
	printf("MPI_INIT...\n");
	MPI_init(&mpi_rank,&mpi_procs,&argc,&argv);
	//----------------------------------------------------
	
	// check to make sure we got a city file
	if (!citiesFile)
	{
		printf("no city file present. halting\n");
		terminate_program(3); // ERROR: no city file present
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
	//----------------------------------------------------
	
	printf("Using '%s' for cities and '%s' for initial tours.\n", citiesFile, toursFile);
	
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
			CitiesA = malloc(sizeof(tour_t));
			CitiesB = malloc(sizeof(tour_t));
			DPRINTF("Island %i receiving citysize...CitiesA: %x ->size: %i\n", mpi_rank, CitiesA, (CitiesA?CitiesA->size:-1));
			MPI_Recv(&(CitiesA->size), 1, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status);
			CitiesB->size = CitiesA->size;
			DPRINTF("%i Got city size of %i!\n", mpi_rank, CitiesA->size);
			// receive actual array of cities as intarray
			DPRINTF("Island %i waiting for cities...\n", mpi_rank);
			MPI_Recv(intCities, CitiesA->size*3, MPI_INT, 0, MPI_TAG, MPI_COMM_WORLD, &status); // size of intCities has x, y, id values.
			DPRINTF("Island %i got cities!\n", mpi_rank);
			// malloc the cities in the array
			for (i=0;i<CitiesA->size;i++) {
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
	} else {
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
	
	

	// construct the distance table (on all processes)
#if USE_DISTANCE_TABLE
	construct_distTable(CitiesA,N);
	DPRINTF("Table constructed.\n");
#else
	DPRINTF("Not using a distance table.\n");
#endif

	

	// populate tours (on all processes)
//	populate_tours(N,mpi_rank,Tours,CitiesA, I, numFileTours);
	//----------------------------------------------------

	
#if BEST_TOUR_TRACKING
	initBestTourTracking();
#endif
	//----------------------------------------------------

	// parameters
	int ants = 200;
	float pMin = -0.5;
	float pMax = 1.0;
	int pResetInterval = 20; // resets pTable to 0 every so many iterations
	float pFactor = 0.25; // factor of pheremone
	float distFactor = 0.75; // factor of distance
	float randFactor = 0.25; // random factor
	float pLocalEnhance = pMax*0.25/(ants*CitiesA->size); // local pheremone enhancement factor
	float pGlobalEnhance = pMax; // global pheremone enhancement factor
	float pDecay = 0.5; // pheremone decay
	
	// ant tours
	int numTours = ants+1;
	tour_t** tours = malloc(sizeof(tour_t*) * numTours);
	for (i=0; i < numTours; i++)
		tours[i] = malloc(sizeof(tour_t));
	tours[numTours-1]->fitness = 1000000.0;
	
	// pheromone table
	float* pTable = malloc(sizeof(float) * CitiesA->size * CitiesA->size);
	for (i=0; i < CitiesA->size*CitiesA->size; i++)
		pTable[i] = 0.0;
		
	// visited table
	char visited[MAX_CITIES];
	
	//TODO: candidate list?
	
	printf("Entering main loop.\n");
	int iterations = 0;
	while (iterations++ < 50) // TODO: exit condition?
	{
		float bestFitness = 1000000000.0;
		tour_t* bestTour = 0;
		int m; // current ant
		int c; // current city being examined to decide to travel to it or not
		int bestC; // best choice so far
		float bestCfactor; // the factor of the best choice
		for (m = 0; m < ants; m++)
		{
			tours[m]->size = 0;
			memset(visited, 0, sizeof(char)*CitiesA->size);
			int curCity = rand() % CitiesA->size;
			//tours[m]->city[tours[m]->size++] = CitiesA->city[curCity];
			for (i = 0; i < CitiesA->size; i++)
			{
				// visit current city
				visited[curCity] = 1;
				tours[m]->city[tours[m]->size++] = CitiesA->city[curCity];
				//printf("ant[%i] visiting city[%i]...\n", m, curCity);
				
				// pick next city by decision function
				bestC = 0;
				bestCfactor = 100000000.0;
				for (c=0; c < CitiesA->size; c++)
				{
					if (visited[c]) continue;
					float dist = lookup_distance(curCity, c);
					float factor = dist*distFactor - pTable[curCity*CitiesA->size + c]*pFactor*dist + frand()*randFactor*dist;
					//printf("it%ia%ii%i :: d%f    p%f    f%f\n", iterations, m, i, 
					//			dist*distFactor, 
					//			pTable[curCity*CitiesA->size + c]*pFactor*dist, factor);
					if (factor < bestCfactor)
					{
						bestC = c;
						bestCfactor = factor;
					}
				}// each city
				
				// move to the best city, depositing some pheremones
				pTable[curCity*CitiesA->size + bestC]+= pLocalEnhance;
				pTable[bestC*CitiesA->size + curCity]+= pLocalEnhance;
				curCity = bestC;
				
			}// each vertex needs to be visited
			//printf("ant[%i] visited all cities.\n", m);
			
			set_tour_fitness(tours[m], tours[m]->size);
			printf("ant[%i]: ", m);
			print_tour(tours[m]);
			//printf("next ant.\n");
		}// for each ant
		
		// sort the tours
		sortTours(tours, ants);
		// only replace the best tour if any of the new tours was actually better
		if (tours[0]->fitness < tours[numTours-1]->fitness)
			memcpy(tours[numTours-1], tours[0], sizeof(tour_t));
		
		// output the best one
		print_tour(tours[numTours-1]);
		
		// decay pheremones
		if (iterations % pResetInterval == 0)
		{
			for (i=0; i < CitiesA->size*CitiesA->size; i++)
			{
				pTable[i] = 0;
			}	
		}
		else
		{
			for (i=0; i < CitiesA->size*CitiesA->size; i++)
			{
				pTable[i] -= pTable[i] * pDecay;
				if (pTable[i] < pMin) pTable[i] = pMin;
				if (pTable[i] > pMax) pTable[i] = pMax;
			}
		}
		
		// deposit pheremones on the global best tour
		for (i=0; i < CitiesA->size; i++)
		{
			pTable[tours[numTours-1]->city[i]->id*CitiesA->size + tours[numTours-1]->city[(i+1)%CitiesA->size]->id]+= pGlobalEnhance;
			pTable[tours[numTours-1]->city[(i+1)%CitiesA->size]->id*CitiesA->size + tours[numTours-1]->city[i]->id]+= pGlobalEnhance;
		}
	}

	//####################################################
	// Free Memory, Terminate Program
	//####################################################
	terminate_program(0);
}
