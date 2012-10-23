#ifndef FIT_FUNH // header guard
#define FIT_FUNH 1

#include "tsp.h"

/** Stores distances from every point to another. */
float distTable[TABLE_SIZE];

/**
 * Using The Pythagorean's Theorem, calculate the distance from p1 to p2.
 */
float get_distance_between(int p1, int p2, tour_t* cities);

/**
 *  Constructs the distTable.
 */
void construct_distTable(tour_t* cities, int num_cities);

/**
 * Returns the distance traveled from a point p1 to another point p2.
 *   This is retrieved from the distTable hashtable.
 */
float lookup_distance(int p1, int p2);

/**
 * Given a tour and the number of cities, determine its fitness by
 * computing the distance required to traverse the route.
 */
void set_tour_fitness(tour_t* tour, int num_cities);

/**
 * Generates the nearest neighbor tour based on a random city.
 */
tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities);

/**
 * Given a city, find its nearest neighbor.
 */
city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited);

#endif // header guard
