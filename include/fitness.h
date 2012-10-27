#ifndef FIT_FUNH // header guard
#define FIT_FUNH 1

#include "include/tsp.h"

/** Stores distances from every point to another. */
float distTable[TABLE_SIZE];

float get_distance_between(int p1, int p2, tour_t* cities); // calculate distance between 2 cities.

void construct_distTable(tour_t* cities, int num_cities); // constructs distTable variable.

float lookup_distance(int p1, int p2); // distance between two points; retrieved in constant time.

void set_tour_fitness(tour_t* tour, int num_cities); // compute and set a tour's fitness.

tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities); // create nearest neighbor tour based on an initial city.

city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited); // find nearest neighbor of a given city.

#endif // header guard
