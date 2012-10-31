/**
 * contains types and what not that will be used throughout the entirety of the TSP algorithm
 */
#ifndef TSP_H // header guard
#define TSP_H

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/printcolors.h"

#define DEBUG 1     // set to zero to remove a lot of debugging output and speed up the code 
#define DPRINTF if (DEBUG) printf
#define MAX_CITIES 100
#define MAX_TOUR 100     // this should basically be the same as MAX_CITIES
#define MAX_POPULATION 200
#define TABLE_SIZE (MAX_CITIES*(MAX_CITIES-1))/2 // size based on a counting argument

/**
 * represents a city that must be visited to create a complete tour
 */
typedef struct {
	int x,y; // x and y position of the city
	int id; // a unique number for each city in the map. It should be equal to the city's index in the cities array.
} city_t;

/**
 * represents a possible tour of the cities
 */
typedef struct {
	city_t* city[MAX_TOUR]; // a pointer to each city in the tour
	int size; // size of the tour
	// ~~!
	float fitness; // the fitness of the entire tour.
} tour_t;

float get_distance_between(int p1, int p2, tour_t* cities); // calculate distance between 2 cities.

void construct_distTable(tour_t* cities, int num_cities); // constructs distTable variable.

float lookup_distance(int p1, int p2); // distance between two points; retrieved in constant time.

void set_tour_fitness(tour_t* tour, int num_cities); // compute and set a tour's fitness.

tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities); // create nearest neighbor tour based on an initial city.

city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited); // find nearest neighbor of a given city.

tour_t* loadCities(const char* const fileName); // loads cities from file

void freeCities(tour_t* cities); // frees the memory used by the structure

float frand(); // Returns a random float between 0.0 and 1.0.

void print_tour(tour_t* tour); // Simple print procedure for a tour.

tour_t* roulette_select(tour_t tours[], int num_tours); // selects a random tour weighted by its fitness.

#endif // header guard
