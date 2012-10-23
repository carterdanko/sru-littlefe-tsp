/**
 * contains types and what not that will be used throughout the entirety of the TSP algorithm
 */
#ifndef TSP_H // header guard
#define TSP_H

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
 
#define MAX_CITIES 100
#define MAX_TOUR 100     // this should basically be the same as MAX_CITIES
#define MAX_POPULATION 200
#define TABLE_SIZE (MAX_CITIES*(MAX_CITIES-1))/2 // size based on a counting argument

#define COLOR_TEXT 1 // set to false to disable coloring the console output

#define NORMAL_TEXT printf(COLOR_TEXT?"\033[0m":"")
#define ERROR_TEXT printf(COLOR_TEXT?"\033[31m":"")
#define OOPS_TEXT printf(COLOR_TEXT?"\033[33m":"")
#define STRONG_TEXT printf(COLOR_TEXT?"\033[32m":"")

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

/** Data structure containing the coordinates of every city. */
city_t cities[MAX_CITIES];
/** Data structure that holds all of the tours in the population. */
tour_t tours[MAX_POPULATION];

tour_t* loadCities(const char* const fileName); // loads cities from file
void freeCities(tour_t* cities); // frees the memory used by the structure

/**
 * Returns a random float between 0.0 and 1.0.
 */
float frand();

void print_tour(tour_t* tour);

#endif // header guard
