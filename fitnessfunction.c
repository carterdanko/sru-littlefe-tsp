/** Author: Mike Tasota
 *  Date:   18 September 2012
 *  Descr:  Pretending like this was never accidentally lost the first time
 *          and that I didn't have to rewrite this code again. OK? So, this
 *          is the original file I wrote, because I only wrote this once.
 */
#include <stdio.h>
#include <math.h>
#include "fitnessfunction.h"
#include "tsp.h"

#define DEBUG 1

/** Stores distances from every point to another. */
float distTable[TABLE_SIZE];

/**
 * Using The Pythagorean's Theorem, calculate the distance from p1 to p2.
 */
float get_distance_between(int p1, int p2) {
	float x,y;
	x = cities[p1].x - cities[p2].x;
	y = cities[p1].y - cities[p2].y;
	return sqrtf(x*x+y*y);
}

/**
 *  Constructs the distTable.
 */
void construct_distTable() {
	int i,j,index;
	index=0;
	for (i=0;i<MAX_CITIES;i++) {
		for (j=i+1;j<MAX_CITIES;j++) {
			distTable[index] = get_distance_between(i,j);
			index++;
		}
	}
}

/**
 * Returns the distance traveled from a point p1 to another point p2.
 *   This is retrieved from the distTable hashtable.
 */
float lookup_distance(int p1, int p2) {
	if (p1<p2) {
		return distTable[TABLE_SIZE-((MAX_CITIES-p1-1)*(MAX_CITIES-p1))/2+p2-p1-1];
	} else if (p1>p2) {
		return distTable[TABLE_SIZE-((MAX_CITIES-p2-1)*(MAX_CITIES-p2))/2+p1-p2-1];
	} else {
		printf("WARNING -- THIS SHOULD NEVER HAPPEN (p1==p2); returning 0...\n");
		return 0.0;
	}
}

/**
 * Given a route *arr, determine its fitness by computing the distance
 * required to traverse the route.
 *   -Assumes that the length of arr = MAX_CITIES
 */
float get_fitness_route(int *arr) {
	int i;
	float fitness=0;
	for (i=0;i<MAX_CITIES-1;i++) {
		fitness+=lookup_distance(arr[i],arr[i+1]);
	}
	// do we count arr[n] --> arr[0] ?
	return fitness;
}

tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities) {
	// Set up the cities_visited array; 0 for not visited, 1 for visited.
	char *cities_visited;
	cities_visited = (char *)malloc( num_cities * sizeof(char) );
	memset((void*)cities_visited, 0, sizeof(cities_visited));
	// The tour to be returned.
	tour_t* tour;
	tour = malloc( sizeof(tour_t) );
	// The next city to place in the tour.
	city_t* next_city;
	// Init to be the city passed into the function
	next_city = city;
	// The first city is city passed.
	tour->city[0] = city;
	cities_visited[ city->id ]=1;
	
	int i;

	// Iterate through the cities, adding new ones and marking them off.
	for (i=1;i<num_cities;i++) {
		next_city = find_nearest_neighbor(next_city,num_cities,cities,cities_visited);
		tour->city[i]=next_city;
		cities_visited[ next_city->id ]=1;
	}
}

city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited) {
	city_t* short_city;
	float temp_dist,short_dist;
	temp_dist=short_dist=0.0;
	int i;
	
	for (i=0;i<num_cities;i++) {
		temp_dist = get_distance_between(cities->city[i]->id,city->id);
		if (  temp_dist < short_dist && cities_visited[i]==0) {
			// If your distance was shorter than the shortest, use this instead.
			short_city = cities->city[i];
			short_dist = temp_dist;
		} else if (short_city==0 && cities_visited[i]==0) {
			// Otherwise, if not already set, get the first distance as your shortest.
			short_city = cities->city[i];
			short_dist = temp_dist;
		}
	}
	return short_city;
}
