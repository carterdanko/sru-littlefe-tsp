/**
 * contains types and what not that will be used throughout the entirety of the TSP algorithm
 */
#ifndef TSP_H // header guard
#define TSP_H

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "fitnessfunction.h"
 
#define MAX_CITIES 100
#define MAX_TOUR 100     // this should basically be the same as MAX_CITIES
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
} tour_t;

/** Data structure containing the coordinates of every city. */
city_t cities[MAX_CITIES];



tour_t* loadCities(const char* const fileName); // loads cities from file
void freeCities(tour_t* cities); // frees the memory used by the structure

#endif // header guard
