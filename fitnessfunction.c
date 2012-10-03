/** Author: Mike Tasota
 *  Date:   18 September 2012
 *  Descr:  Pretending like this was never accidentally lost the first time
 *          and that I didn't have to rewrite this code again. OK? So, this
 *          is the original file I wrote, because I only wrote this once.
 */
#include <stdio.h>
#include <math.h>
#include "tsp.h"

#define DEBUG 1

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

int main() {
	int my_route[MAX_CITIES];
	if (DEBUG) {
		/*
		location[0][0] = 0;
		location[0][1] = 0;
		location[1][0] = 1;
		location[1][1] = 1;
		location[2][0] = 1;
		location[2][1] = 0;
		location[3][0] = 0;
		location[3][1] = 1;

		//int i;
		//for (i=0;i<MAX_CITIES;i++) {
		//	my_route[i]=i;
		//}
		my_route[0]=0;
		my_route[1]=3;
		my_route[2]=1;
		my_route[3]=2;
		*/
	}
	construct_distTable();
	if (DEBUG) {
		printf("fitness of simple route is: %f\n",get_fitness_route(my_route));
	}
}
