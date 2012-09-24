/**
 * contains types and what not that will be used throughout the entirety of the TSP algorithm
 */
#ifndef TSP_H // header guard
#define TSP_H

#include <stdlib.h>
#include <stdio.h>
 
#define MAX_CITIES 100
#define MAX_TOUR 100     // this should basically be the same as MAX_CITIES

/**
 * represents a city that must be visited to create a complete tour
 */
typedef struct {
	int x,y; // x and y position of the city
} city_t;

/**
 * represents a possible tour of the cities
 */
typedef struct {
	city_t* city[MAX_TOUR]; // a pointer to each city in the tour
	int size; // size of the tour
} tour_t;

#endif // header guard
