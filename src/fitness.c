////////////////////////////////////////////////////////////////////////////////
//	DESC:	Contains functions that involve the manipulation of a tour's
//			fitness.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include "include/tsp.h"
#include "include/eax.h"

/** Stores distances from every point to another. */
float* distTable;// initialized in fitness.c inside construct_distTable
edge_t** edgeTable; // a table initialized to contain all possible edges

/**
 * DESC: Computes the euclidean distance from (x1,y1) to (x2,y2).
 */
float dist(float x1, float y1, float x2, float y2)
{
	float x,y;
	x = x2-x1;
	y = y2-y1;
#if LEAVE_SQUARED
	return x*x+y*y;
#else
	return sqrt(x*x+y*y);
#endif
}

/**
 * DESC: Using The Pythagorean's Theorem, calculate the distance from p1 to p2.
 */
float get_distance_between(int p1, int p2, tour_t* cities)
{
	return dist(cities->city[p1]->x, cities->city[p1]->y, cities->city[p2]->x,
		cities->city[p2]->y);
}

/**
 * DESC: Constructs the distTable.
 */
tour_t* Cities;
void construct_distTable(tour_t* cities, int num_cities)
{
#if USE_DISTANCE_TABLE
	Cities = cities;
#if USE_BIG_TABLE
	distTable = malloc(sizeof(*distTable) * (num_cities*num_cities));
#else
	distTable = malloc(sizeof(*distTable) * ((num_cities * (num_cities-1)) / 2));
#endif
#if USE_EDGE_TABLE
	edgeTable = malloc(sizeof(edge_t*) * ((num_cities * (num_cities-1)) / 2));
#endif
	int i,j,index;
	int bound1, bound2; // loop bounds
#if USE_BIG_TABLE
	bound1 = bound2 = num_cities;
#else
	bound1 = num_cities;
#endif
	index=0;
	for (i=0;i<bound1;i++)
	{
#if USE_BIG_TABLE
#else
		bound2 = i;
#endif
		for (j=0;j<bound2;j++)
		{
			distTable[index] = get_distance_between(i,j,cities);
#if USE_EDGE_TABLE
			edgeTable[index] = malloc(sizeof(edge_t));
			edgeTable[index]->v1 = i;
			edgeTable[index]->v2 = j;
			edgeTable[index]->cost = distTable[index];
			edgeTable[index]->cycle = -1;
#endif // use edge table
#if PRINT_DISTANCE
			DPRINTF("(%i,%i)->%f\t",i,j,distTable[index]);
#endif
			index++;
		}
#if PRINT_DISTANCE
		printf("\n");
#endif
	}
#endif
}

/**
 * DESC: Returns the distance traveled from a point p1 to another point p2.
 *   This is retrieved from the distTable hashtable.
 */
float lookup_distance(int p1, int p2)
{
#if USE_DISTANCE_TABLE
#if USE_BIG_TABLE
	return distTable[p1*CitiesA->size+p2];
#else
	if (p1<p2)
	{
#if ENFORCE_LOOKUP_TABLE_CORRECTNESS
		if (distTable[(p2*(p2-1)/2)+p1] != get_distance_between(p1, p2, Cities))
		{
			ERROR_TEXT;
			printf("INVALID VALUE IN DISTANCE TABLE OR INVALID LOOKUP: p1=%i p2=%i\n", p1, p2);
			NORMAL_TEXT;
			terminate_program(783);
		}
#endif
		return distTable[(p2*(p2-1)/2)+p1];
	}
	else if (p1>p2)
	{
#if ENFORCE_LOOKUP_TABLE_CORRECTNESS
		if (distTable[(p1*(p1-1)/2)+p2] != get_distance_between(p1, p2, Cities))
		{
			ERROR_TEXT;
			printf("INVALID VALUE IN DISTANCE TABLE OR INVALID LOOKUP: p1=%i p2=%i\n", p1, p2);
			NORMAL_TEXT;
			terminate_program(784);
		}
#endif
		return distTable[(p1*(p1-1)/2)+p2];
	}
	else
	{
		ERROR_TEXT;
		DPRINTF("WARNING -- THIS SHOULD NEVER HAPPEN (p1[%i]==p2[%i]); terminating...\n", p1, p2);
		NORMAL_TEXT;
		terminate_program(787);
		return 0.0;
	}
#endif// don't use big table
#else // don't use distance table
#if USE_NAIVE_DISTANCE
	int x, y, mask;
	// dist x
	x = CitiesA->city[p1]->x - CitiesA->city[p2]->x;
	// absolute value
	mask = x >> (sizeof(int)*8 - 1);
	x = (x+mask) ^ mask;
	// dist y
	y = CitiesA->city[p1]->y - CitiesA->city[p2]->y;
	// absolute value
	mask = y >> (sizeof(int)*8 - 1);
	y = (y+mask) ^ mask;
	return x + y;
#else
	return get_distance_between(p1, p2, CitiesA);
#endif// use_naive_distance
#endif// don't use distance table
} // lookup_distance()

#if USE_EDGE_TABLE
edge_t* lookup_edge(int p1, int p2)
{
	if (p1<p2)
	{
		return edgeTable[(p2*(p2-1)/2)+p1];
	}
	else if (p1>p2)
	{
		return edgeTable[(p1*(p1-1)/2)+p2];
	}
	else
	{
		ERROR_TEXT;
		DPRINTF("WARNING IN LOOKUP_EDGE -- THIS SHOULD NEVER HAPPEN (p1[%i]==p2[%i]); terminating...\n", p1, p2);
		NORMAL_TEXT;
		
		terminate_program(787);
		return 0;
	}
} // lookup_edge()
#endif

/**
 * DESC: Given a tour and the number of cities, determine its fitness by
 *	computing the distance required to traverse the route.
 */
void set_tour_fitness(tour_t* tour, int num_cities)
{
	int i;
	tour->fitness=0.0;
#if DEBUG_SET_TOUR_FITNESS
	DPRINTF("Debugging fitness for tour: ");
	dprint_tour(tour);
#endif
	for (i=0; i < num_cities-1; i++) 
	{
#if DEBUG_SET_TOUR_FITNESS
		float lookup = lookup_distance(tour->city[i]->id,
			tour->city[i+1]->id);
		float calc = dist(tour->city[i]->x, tour->city[i]->y,
			tour->city[i+1]->x, tour->city[i+1]->y);
		tour->fitness+=lookup;
		DPRINTF("[%i]->[%i] : lookup: %f  calc: %f   total: %f\n",
			tour->city[i]->id, tour->city[i+1]->id, lookup, calc,
			tour->fitness);
#else
		tour->fitness+=lookup_distance(tour->city[i]->id,tour->city[i+1]->id);
#endif
	}
	// and, we count arr[n] --> arr[0]
#if DEBUG_SET_TOUR_FITNESS
	float lookup = lookup_distance(tour->city[num_cities-1]->id,
		tour->city[0]->id);
	float calc = dist(tour->city[num_cities-1]->x, tour->city[num_cities-1]->y,
		tour->city[0]->x, tour->city[0]->y);
	tour->fitness+=lookup;
	DPRINTF("[%i]->[%i] : lookup: %f  calc: %f   total: %f\n",
		tour->city[num_cities-1]->id, tour->city[0]->id, lookup, calc,
		tour->fitness);
#else
	tour->fitness+=lookup_distance(tour->city[num_cities-1]->id,
		tour->city[0]->id);
#endif
}

/**
 * DESC: Given a city, find its nearest neighbor. The array cities_visited
 *	denotes the id of cities which are available (0) and unavaiable/already
 *	visited (1).
 */
city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities,
 char* cities_visited)
{
	city_t* short_city;
	short_city=malloc( sizeof(city_t) );
	float temp_dist,short_dist;
	short_dist=10000000.0; // some big value //TODO: replace with float max?
	int i;

	for (i=0;i<num_cities;i++)
	{
		if (cities_visited[i] || cities->city[i]->id == city->id)
			continue;
		temp_dist = lookup_distance(cities->city[i]->id,city->id);
		if (  temp_dist < short_dist)
		{
			// If your distance was shorter than the shortest,
			// use this instead.
			short_city = cities->city[i];
			short_dist = temp_dist;
		}
	}
	return short_city;
}

/**
 * DESC: Given an array of yours and the number of tours in the array, randomly
 *	choose one of the tours. The choice is weighted based on the fitness
 *	of the function, inversely. In other words, for fitness F1 for tour T1,
 *	your probability of receiving tour T1 is (1/F1) / sum( 1/Fi ).
 *
 * tours : the array of tours to choose from
 * num_tours : the number of tours to choose from
 * ignore_tour [optional] : a tour to ignore for choosing, set to null (0)
 *	to not ignore any tours
 */
tour_t* roulette_select(tour_t** tours, int num_tours, tour_t* ignore_tour)
{
	int i;
	float rand,rand_fit,sum_fitness,temp;
	sum_fitness=0.0;

#if CAP_ROULETTE_WHEEL
	float fitTotal = 0;
	float fitAvg = -1;
	float fitMin = -1;
	float fitMax = -1;
	for (i = 0; i < num_tours; i++)
	{
		if (tours[i] == ignore_tour)
			continue;
		temp = 1.0 / tours[i]->fitness;
		fitTotal += temp;
	}
	fitAvg = fitTotal / (num_tours - (ignore_tour?1:0));
	fitMin = RW_CAP_MIN * fitAvg;
	fitMax = RW_CAP_MAX * fitAvg;
#endif

	// sum up the inverted total fitnesses
	for (i=0;i<num_tours;i++)
	{
		if (tours[i] == ignore_tour)
			continue; // don't count ignore_tour in the fitness sum
		temp = tours[i]->fitness;
#if ENFORCE_NONZERO_FITNESS
		if (temp==0) {
			printf("tour %i has fitness zero. hex: %x\n",i, tours[i]);
		}
#endif
		temp = 1.0 / temp;
#if CAP_ROULETTE_WHEEL
		if (temp < fitMin)
			temp = fitMin;
		if (temp > fitMax)
			temp = fitMax;
#endif
		sum_fitness+= temp;
	}

	// random float from 0 to 1
	rand=frand();

	// some random point between 0 and top fitness
	rand_fit = sum_fitness * rand;

	for (i=0;i<num_tours;i++)
	{
		if (tours[i] == ignore_tour)
			continue; // don't check the ignore tour, it wasn't counted in the fitness sum
		temp = 1.0 / tours[i]->fitness;
#if CAP_ROULETTE_WHEEL
		if (temp < fitMin)
			temp = fitMin;
		if (temp > fitMax)
			temp = fitMax;
#endif
		if (rand_fit < temp) {
			// If your fitness is in this tour, return it.
			return tours[i];
		} 
		// Otherwise, subtract this tour's fitness from sum_fitness and try again.
		else rand_fit-=temp;
	}
	
	// never executes.
	ERROR_TEXT;
	DPRINTF("Reached 'unreachable' code in roullette_select.\n");
	NORMAL_TEXT;
	DPRINTF("rand: %f, rand_fit: %f, sum_fitness: %f, temp: %f, ignore_tour: m(\033[31m%i\033[0m)\n", rand, rand_fit, sum_fitness, temp, ignore_tour);
	terminate_program(543);
	return 0;
}

