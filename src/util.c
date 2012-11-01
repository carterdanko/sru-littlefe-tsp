#include "include/tsp.h"

float frand() {
	return ((float)rand())/((float)RAND_MAX);
}

void print_tour(tour_t* tour) {
	int i;
	printf("Tour: [%i]", tour->city[0]->id);
	for (i=1; i < tour->size; i++)
		printf(", [%i]", tour->city[i]->id);
	printf("\n");
}

void dprint_tour(tour_t* tour) {
	int i;
	DPRINTF("Tour: [%i]", tour->city[0]->id);
	for (i=1; i < tour->size; i++)
		DPRINTF(", [%i]", tour->city[i]->id);
	DPRINTF("\n");
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
	for (i=0; i < nCities; i++)
	{
		I[i*3] = C->city[i]->x;
		I[i*3+1] = C->city[i]->y;
		I[i*3+2] = C->city[i]->id;
	}
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
	for (i=0; i < nCities; i++)
	{
		C->city[i]->x = I[i*3];
		C->city[i]->y = I[i*3+1];
		C->city[i]->id = I[i*3+2];
	}
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
		int a;
		for (a=0; a < tours[i]->size; a++)
			I[size++] = tours[i]->city[a]->id;
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
	int i;
	int position = 0;
	for (i=0; i < nTours; i++)
	{
		int a;
		for (a=0; a < tours[i]->size; a++)
			tours[i]->city[a] = Cities->city[I[position++]];
	}
}
