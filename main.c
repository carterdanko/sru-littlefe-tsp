/**
 * main module for the TSP solver
 */
 
#include "tsp.h"
#include "eax.h" //TODO: im pretty sure you want this to be #include "eax.c" [Mike]

// "global" variables. I try to start these with capital letters
tour_t* Cities; // the "tour" that contains every city in their provided order. Not really a tour, just used as the master array of cities
 
int main(int argc, char** argv)
{
	/*
	printf("numargs: %i\n", argc);
	printf("filename? '%s'\n", argv[0]);
	*/
	
	// check number of parameters
	if (argc < 2)
	{
		printf("Usage: tsp <filename of cities text document> [flags]\n");
		exit(1); // ERROR: must supply a filename for the cities
	}
	
	// search input for cities file, and any relevant flags
	char* citiesFile = 0;
	int i;
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			//TODO: process the flags
			if (strcmp(argv[i], "--help") == 0)
			{
				printf("Usage: tsp <filename of cities text document> [flags]\n");
				printf("The first line of the cities text document is the number of cities.\n");
				printf("The following lines are each city. 2 integers, space separated, are the x and y of that city. Example: \n");
				printf("23 45\n");
			}
		}
		else if (citiesFile)
		{
			printf("Please only supply one city file. Use tsp --help for help\n");
			exit(2); // ERROR: must supply exactly one city file
		}
		else
		{
			citiesFile = argv[i];
			printf("City file: '%s'\n", citiesFile);
		}
	} // for each parameter
	// check to make sure we got a city file
	if (!citiesFile)
	{
		printf("no city file present. halting\n");
		exit(3); // ERROR: no city file present
	}
	
	// load the cities specified by the file
	printf("Loading cities...");
	Cities = loadCities(citiesFile);
	if (!Cities)
	{
		printf("Error while loading cities. refer to error log? halting.\n");
		exit(5); // ERROR: error while loading cities
	}
	printf("done! (loaded %i cities from the file)\n", Cities->size);
	
	///////////////////////////////////////////////////////////////////////////
	// process the cities
	
	// output the city information to the console
	printf("\nNum Cities: %04i\n", Cities->size);
	printf("---------------------------\n");
	for (i=0; i < Cities->size; i++)
	{
		printf("City[%04i] at %04i, %04i   [id: %04i]\n", i, Cities->city[i]->x, Cities->city[i]->y, Cities->city[i]->id);
	}
	
	// create two new tours by some arbitrary but reproducible means
	//~~! removed the old methods for creating tours to replace with the new method.
	tour_t tourA, tourB;
	tour_t* tstar;
//	tourA.size = tourB.size = Cities->size;
	tourA.size = Cities->size;
	int N = Cities->size;
	for (i=0; i < N; i++)
	{
		tourA.city[i] = Cities->city[(i*2)%N];
//		tourB.city[i] = Cities->city[Cities->size-i-1];
	}
	tstar = create_tour_nn((city_t*)Cities->city[0], Cities->size, Cities);
	tourB = *tstar;

	// now, find fitness of the tours.
	construct_distTable(Cities,N);
	set_tour_fitness(&tourA,N);
	set_tour_fitness(&tourB,N);
	
	printf("fitness of A,B is %f,%f.\n", tourA.fitness,tourB.fitness);
	// output the two tours
	printf("TourA: [%i]", tourA.city[0]->id);
	for (i=1; i < N; i++)
		printf(", [%i]", tourA.city[i]->id);
	printf("\nTourB: [%i]", tourB.city[0]->id);
	for (i=1; i < N; i++)
		printf(", [%i]", tourB.city[i]->id);
	printf("\n");

	// now, testing print tour function for A and B.
	print_tour(&tourA,N);
	print_tour(&tourB,N);

	// now, testing roulette wheel 5 times.
	srand(0);
	tours[0] = &tourA;
	tours[1] = &tourB;
	tour_t* tempt;
	tempt = malloc( sizeof(tour_t) );
	printf(" ~~~ ROULETTE WHEEL ~~\n");
	for (i=0;i<5;i++) {
		//tempt = roulette_select(tours, 2);
		//print_tour(tempt, N);
	}
	printf(" ~~~  END ROULETTE  ~~\n");
	
	// merge the two tours
	printf("\nMerging A with B...");
	graph_t* R = mergeTours(&tourA, &tourB);
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

	//TODO: create E-sets from the cycles

	//TODO: apply E-sets to generate intermediates

	//TODO: turn intermediates into valid tours
	
	// clean up
	printf("\nClean up...");
	freeGraph(R);
	printf("graph gone...");
	freeCities(Cities);
	printf("main cities gone...");
	//for (i=0; i < MAX_ABCYCLES; i++)
	//	freeCities(cycles[i]);
	printf("AB cycles structure gone...");
	printf("done!\n");
	
	// done (just used to make sure that the program ran to completion)
	printf("Program ran to completion (done).\n");
	exit(0);
}
