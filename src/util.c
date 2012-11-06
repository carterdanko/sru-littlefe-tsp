#include "include/tsp.h"
#include "include/eax.h"

float frand() {
	return ((float)rand())/((float)RAND_MAX);
}

void print_tour(tour_t* tour) {
	int i;
	printf("Tour [f:\033[33m%f\033[0m]: [%i]", tour->fitness, tour->city[0]->id);
	for (i=1; i < tour->size; i++)
		printf(", [%i]", tour->city[i]->id);
	printf("\n");
}

void dprint_tour(tour_t* tour) {
	int i;
	DPRINTF("(DPRINTF)Tour [f:\033[33m%f\033[0m]: [%i]", tour->fitness, tour->city[0]->id);
	for (i=1; i < tour->size; i++)
		DPRINTF(", [%i]", tour->city[i]->id);
	DPRINTF("\n");
}

/*
 * Use of this function is not currently necessary but will be when MPI is implemented.
 */
void terminate_program(int ecode) 
{
#if MPIFLAG
	MPI_Finalize();
#endif

	// only runs for "successful" program termination.
	OOPS_TEXT;
	printf("randSeed: %i, citiesFile: '%s'\n", randSeed, citiesFile);
	NORMAL_TEXT;
	if (Tours[0])
	{
		STRONG_TEXT;
		printf(" -- BEST TOUR --\n");
		print_tour(Tours[0]);
		NORMAL_TEXT;
	}
	else
	{
		ERROR_TEXT;
		printf("Invalid best tour!\n");
		NORMAL_TEXT;
	}
	if (ecode==0) 
	{

		// done (just used to make sure that the program ran to completion)
		STRONG_TEXT;
		printf("Program ran to completion (done).\n");
		NORMAL_TEXT;
	}
	else
	{
		ERROR_TEXT;
		printf("PROGRAM ABNORMALLY TERMINATED, ECODE: %i\n", ecode);
		NORMAL_TEXT;
	}

	// exit the program.
	exit(ecode);
}

/**
 * converts an array of cities into an array of ints
 * in order to be transferred by mpi
 * C : an array of cities (as a tour structure)
 * nCities : how many cities
 * I : by-ref IN: pre-allocated array of ints OUT: converted array
 */
void city_tToInt(tour_t* C, int nCities, int* I)
{
	int i;
	DPRINTF("CITY_TO_INT::  ");
	for (i=0; i < nCities; i++)
	{
		I[i*3] = C->city[i]->x;
		I[i*3+1] = C->city[i]->y;
		I[i*3+2] = C->city[i]->id;
		DPRINTF("%i(%i,%i)  ",I[i*3+2],I[i*3],I[i*3+1]);
	}
	DPRINTF("\n");
}

/**
 * converts an array of cities into an array of ints
 * in order to be transferred by mpi
 * C : an array of cities (as a tour structure)
 * nCities : how many cities
 * I : by-ref IN: pre-allocated array of ints OUT: converted array
 */
void intToCity_t(int* I, int nCities, tour_t* C)
{
	int i;
//	C->size = nCities;
	DPRINTF("INT_TO_CITY::  ");
	for (i=0; i < nCities; i++)
	{
		C->city[i]=(city_t*)malloc(sizeof(city_t));
		C->city[i]->x = I[i*3];
		C->city[i]->y = I[i*3+1];
		C->city[i]->id = I[i*3+2];
		DPRINTF("%i(%i,%i)  ",I[i*3+2],I[i*3],I[i*3+1]);
	}
	DPRINTF("\n");
}

/**
 * converts an array of tours into
 * an integer array where each cell is the index of the city id.
 * tours : IN: the tours to convert
 * nTours : how many tours are in the array
 * I : IN: pre-allocated array of integers OUT: the converted array
 */
void tour_tToInt(tour_t** tours, int nTours, int* I)
{
	int i;
	int size = 0;
	for (i=0; i < nTours; i++)
	{
		DPRINTF("TOUR_TO_INT::  ");
		int a;
		for (a=0; a < tours[i]->size; a++) {
			DPRINTF("%i:%i->%i  ",i,a,tours[i]->city[a]->id);
			I[size++] = tours[i]->city[a]->id;
		}
		DPRINTF("\n");
	}
}

/**
 * converts an array of integers containing indices of cities back
 * into an array of tours
 * Cities : the master cities structure
 * I : IN: the array of indicies
 * nTours : how many tours
 * tours : IN: pre-allocated array of tours, and each tour's size field should be initialized
 *         OUT: the converted array of tours
 */
void intToTour_t(tour_t* Cities, int* I, int nTours, tour_t** tours)
{
	DPRINTF("in intToTour_t()\n");
	int i;
	int position = 0;
	for (i=0; i < nTours; i++)
	{
		int a;
		tours[i] = (tour_t*)malloc(sizeof(tour_t));
		DPRINTF("INT_TO_TOUR::  ");
//		for (a=0; a < tours[i]->size; a++) {
		for (a=0; a < Cities->size; a++) {
			tours[i]->city[a] = Cities->city[I[position++]];
			DPRINTF("%i:%i->%i  ",i,a,tours[i]->city[a]->id);
		}
		DPRINTF("\n");
		tours[i]->size = Cities->size;
	}
}

void dumpGraphToFile(graph_t* G, char* fn)
{
	FILE* f = fopen(fn, "w");
	fprintf(f, "%i\n", G->size);
	
	int a, b;
	for (a=0; a < G->size; a++)
	{
		node_t* curNode = G->node[a];
		fprintf(f, "%i %i", curNode->id, curNode->size);
		for (b=0; b < curNode->size; b++)
		{
			fprintf(f, " %i %i", curNode->tour[b], curNode->edge[b]->id);
		}
		fprintf(f, "\n");
	}
	
	fclose(f);
}

void getBestTours(int max, tour_t** tours, tour_t** bestTours) {
//	bestTours = malloc(sizeof(tour_t*)*max);
	int i;

	for (i=0;i<max;i++) {
		bestTours[i]=tours[i];
	}
}

/**
 * merge mergetour into the list of tours by COPYING THE MEMORY
 * that is, every pointer in tours may be moved around, but tours will contain the same list of pointers,
 * and if mergetour belongs in tours, a memcpy will be performed.
 * tours : IN: array of tours OUT: array of tours modified to contain mergetour if mergetour belongs
 * num_tours : number of tours in tours
 * mergetour : the tour to merge
 */
void mergeTourToPop(tour_t** tours, int num_tours, tour_t* mergetour) 
{
	// fast exit if we're ignoring this tour
	if (mergetour->fitness >= tours[num_tours-1]->fitness)
		return;
		
	int i, j; // loop control
	tour_t* temp; // for swapping
	for (i=0;i<num_tours;i++) // keep stepping through tours until we find a tour that has higher cost than mergetour
	{
		if (mergetour->fitness < tours[i]->fitness) // we found a tour that has a higher cost than mergetour, so insert mergetour here
		{
			// since the last spot in the array is gonna be over-written, swap it for the current spot
			temp = tours[num_tours-1];
			// note: this makes the array lose value at tours[num_tours-1] (the last value).
			for (j=num_tours-1; j > i; j--) 
			{
				tours[j] = tours[j-1]; // shift every cell up one
			}
			// insert merge tour
			tours[i]=temp;
			memcpy(temp, mergetour, sizeof(*mergetour));
			return;
		}// if found a tour with higher cost than merge tour
	}// for each tour
}// mergeTourToPop()

/**
 * merges an array of tours into the master array
 * tours : IN: master array of tours OUT: combined array
 * num_tours : number of tours in the master array (both before and after)
 * toursToMerge : IN: the tours coming in
 * numToursToMerge : the number of tours coming in
 */
void mergeToursToPop(tour_t** tours, int num_tours, tour_t** toursToMerge, int numToursToMerge) {
	// given a sorted list "tours", merge new tours based on their fitness.
	DPRINTF("Merging tours to population . . .\n");
	int i;
	for (i=0;i<numToursToMerge;i++) {
		mergeTourToPop(tours, num_tours, toursToMerge[i]);
	}
	DPRINTF("OK! Merged tours to pop.\n");
}

void sortTours(tour_t** tours, int numTours) {
	DPRINTF("Sorting tours . . .\n");
	merge_sort(tours, 0, numTours-1);
	DPRINTF("OK! Sorted Tours.\n");
}

/**
 * Helper function; swaps two tour elements from an array.
 */
void merge_swap(tour_t** elem1, tour_t** elem2) {
	tour_t* temp;
	temp = *elem1;
	*elem1 = *elem2;
	*elem2 = temp;
}

/**
 * Helper function.
 *  PRECONDITION: a<=mid<=b
 */
void merge_recursive(tour_t** tours, int a, int mid, int b) {
	int i,j;
	for (i=a;i<=mid;i++) {
		for (j=mid+1;j<=b;j++) {
			if (tours[j]->fitness >= tours[i]->fitness) {
				break;
			} else {
				merge_swap(&tours[i],&tours[j]);
				merge_sort(tours,j,b);
			}
		}
	}
}

/**
 * Helper function that should only be called by sortTours().
 *  PRECONDITION: a<b
 */
void merge_sort(tour_t** tours, int a, int b) {
	if (b-a > 1) {
		// recursive step
		int middex = (a+b)/2;

		merge_sort(tours, a, middex);
		merge_sort(tours, middex+1, b);

		// return the recursive merge
		merge_recursive(tours,a,middex,b);
	} else {
		// base step
		if (tours[b]->fitness < tours[a]->fitness) {
			merge_swap(&tours[a],&tours[b]);
		}
	}
}
