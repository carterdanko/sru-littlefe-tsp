/** Author: Mike Tasota
 *  Date:   18 September 2012
 *  Descr:  Pretending like this was never accidentally lost the first time
 *          and that I didn't have to rewrite this code again. OK? So, this
 *          is the original file I wrote, because I only wrote this once.
 */

#include <stdio.h>
#include <math.h>
#include "include/tsp.h"
#include "include/eax.h"
/** Stores distances from every point to another. */
float* distTable;// initialized in fitness.c inside construct_distTable


/**
 * Using The Pythagorean's Theorem, calculate the distance from p1 to p2.
 */
float get_distance_between(int p1, int p2, tour_t* cities) {
	float x,y;
	x = cities->city[p1]->x - cities->city[p2]->x;
	y = cities->city[p1]->y - cities->city[p2]->y;
	return sqrtf(x*x+y*y);
}

/**
 *  Constructs the distTable.
 */
void construct_distTable(tour_t* cities, int num_cities) {
	distTable = malloc(sizeof(*distTable) * ((num_cities * (num_cities-1)) / 2));
	int i,j,index;
	index=0;
	for (i=0;i<num_cities;i++) {
		for (j=0;j<i;j++) {
			distTable[index] = get_distance_between(i,j,cities);
			DPRINTF("(%i,%i)->%f\t",i,j,distTable[index]);
			index++;
		}
		printf("\n");
	}
}

/**
 * Returns the distance traveled from a point p1 to another point p2.
 *   This is retrieved from the distTable hashtable.
 */
float lookup_distance(int p1, int p2) {
	if (p1<p2) {
		DPRINTF("p1<p2 inside lookup_distance: distTable[(%i*(%i-1)/2)+%i==%i]=%f\n", p2, p2, p1, (p2*(p2-1)/2)+p1, distTable[(p2*(p2-1)/2)+p1]);
		float f = distTable[(p2*(p2-1)/2)+p1];
		//DPRINTF("f=%f\n", f);
		return distTable[(p2*(p2-1)/2)+p1];
	} else if (p1>p2) {
		DPRINTF("p1>p2 inside lookup_distance: distTable[(%i*(%i-1)/2)+%i==%i]=%f\n", p1, p1, p2, (p1*(p1-1)/2)+p2, distTable[(p1*(p1-1)/2)+p2]);
		float f = distTable[(p1*(p1-1)/2)+p2];
		//DPRINTF("f=%f\n", f);
		return distTable[(p1*(p1-1)/2)+p2];
	} else {
		printf("WARNING -- THIS SHOULD NEVER HAPPEN (p1==p2); returning 0...\n");
		return 0.0;
	}
}

/**
 * Given a tour and the number of cities, determine its fitness by
 * computing the distance required to traverse the route.
 */
void set_tour_fitness(tour_t* tour, int num_cities) {
	int i;
	float fitness=0.0;
	for (i=0;i<num_cities-1;i++) {
		fitness+=lookup_distance(tour->city[i]->id,tour->city[i+1]->id);
	}
	// do we count arr[n] --> arr[0] ?
	tour->fitness=fitness;
//	return fitness;
}

/**
 * Generates the nearest neighbor tour based on a random city.
 */
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

	// Before returning, set the tour's size.
	tour->size=num_cities;
	return tour;
}

/**
 * Given a city, find its nearest neighbor. The array cities_visited denotes the id of cities
 *  which are available (0) and unavaiable/already visited (1).
 */
city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited) {
	city_t* short_city;
	short_city=malloc( sizeof(city_t) );
	float temp_dist,short_dist;
	temp_dist=short_dist=0.0;
	int i;

	for (i=0;i<num_cities;i++) {
		if (cities->city[i]->id == city->id) {
			continue;
		}
		temp_dist = get_distance_between(cities->city[i]->id,city->id,cities);
		if (  temp_dist < short_dist && cities_visited[i]==0) {
			// If your distance was shorter than the shortest, use this instead.
			short_city = cities->city[i];
			short_dist = temp_dist;
		} else if (short_dist==0 && cities_visited[i]==0) {
			// Otherwise, if not already set, get the first distance as your shortest.
			short_city = cities->city[i];
			short_dist = temp_dist;
		}
	}
	return short_city;
}

/**
 * Given an array of yours and the number of tours in the array, randomly
 * choose one of the tours. The choice is weighted based on the fitness
 * of the function, inversely. In other words, for fitness F1 for tour T1,
 * your probability of receiving tour T1 is (1/F1) / sum( 1/Fi ).
 */
tour_t* roulette_select(tour_t tours[], int num_tours) {
	int i;
	float rand,rand_fit,sum_fitness,temp;
	sum_fitness=0.0;

	// sum up the inverted total fitnesses
	for (i=0;i<num_tours;i++) {
		temp = tours[i].fitness;
		temp = 1.0 / temp;
		sum_fitness+= temp;
	}

	// random float from 0 to 1
	rand=frand();

	// some random point between 0 and top fitness
	rand_fit = sum_fitness * rand;

	for (i=0;i<num_tours;i++) {
		temp = 1.0 / tours[i].fitness;
		if (rand_fit < temp) {
			// If your fitness is in this tour, return it.
			return &tours[i];
		} else {
			// Otherwise, subtract this tour's fitness from sum_fitness and try again.
			rand_fit-=temp;
		}
	}
	// never executes.
	return;
}

