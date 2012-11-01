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

/*
 * Given an array of tours, sort them in ascending order based
 *  on the tour's fitness. (Used to first sort the tour)
 */
void tours_sort(tour_t** tours) {

}

/*
 * Given a population and an array of new tours, determine
 *  which of the new tours will be kept in the population
 *  based on the fitness of each tour.
 */
void merge_tours(tour_t** pop, int pop_size, tour_t** migrated, int migrated_size) {

}

/*
 * Convert a tour into an array of ints (used for MPI)
 */
void tour_to_int(tour_t* tour, int* itour) {

}

/*
 * Convert an array of ints into a tour (used for MPI)
 */
void int_to_tour(int* itour, tour_t* tour) {

}

/*
 * Convert a list of cities into an array of ints (used for MPI)
 */
void city_to_int(city_t** city, int num_cities, int* icity) {

}

/*
 * Convert an array of ints into a list of cities (used for MPI)
 */
void int_to_city(int* icity, city_t** city, int num_cities) {

}
