/*
######################################################################
-Take your list of routes and add up the fitness from every route.
-Mark off values that indicate the end of each route's fitness.
-Pick a random number from 0 to max_fitness, then choose the route
 which is within the bounds of this random number.
######################################################################
*/
#include <stdio.h>
#include <math.h>
#include "tsp.h"

tour_t roulette_select(int num_tours) {
	int i,sum_fitness;
	float rand,rand_fit;
	sum_fitness=0;
	srand(0);

	// sum up the total fitness
	for (i=0;i<num_tours;i++) {
		sum_fitness+=tours[i].fitness;
	}

	// random number from 0 to 1
	rand=frand();

	// some random point between 0 and top fitness
	rand_fit = sum_fitness * rand;

	for (i=0;i<num_tours;i++) {
		if (rand_fit<tours[i].fitness) {
			// If your fitness is in this tour, return it.
			return tours[i];
		} else {
			// Otherwise, subtract this tour's fitness from sum_fitness and try again.
			sum_fitness-=tours[i].fitness;
		}
	}
	// never executes.
	return;
}
