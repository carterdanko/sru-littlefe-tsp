#ifndef FIT_H
#define FIT_H
#include "tsp.h"

/**
 * Returns the distance traveled from a point p1 to another point p2.
 *   This is retrieved from the distTable hashtable.
 */
float lookup_distance(int p1, int p2);

/**
 * Generates the nearest neighbor tour based on a random city.
 */
tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities);


/**
 * Given a city, find its nearest neighbor.
 */
city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited);

#endif
