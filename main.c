/**
 * main module for the TSP solver
 */
 
#include "tsp.h"
#include "eax.h"

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
	
	// process the cities
	
	
	// clean up
	printf("Clean up...");
	freeCities(Cities);
	printf("done!\n");
	
	// done (just used to make sure that the program ran to completion)
	printf("Program ran to completion (done).\n");
	exit(0);
}
