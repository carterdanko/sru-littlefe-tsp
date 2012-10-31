/** Author: Mike Tasota
 *  Date:   18 September 2012
 *  Descr:  Pretending like this was never accidentally lost the first time
 *          and that I didn't have to rewrite this code again. OK? So, this
 *          is the original file I wrote, because I only wrote this once.
 */
#include <stdio.h>
#include <math.h>
#include "include/fitness.h"
#include "include/tsp.h"
#include "util.c"
//~~!
#include "tsp.c"

#define DEBUG 1

/** Stores distances from every point to another. */
//float distTable[TABLE_SIZE];


/**
 * Using The Pythagorean's Theorem, calculate the distance from p1 to p2. (CUDA)
 */
__device__ float dev_get_distance_between(city_t* citylist) {
	float x,y;
	x = citylist[threadIdx.x].x - citylist[blockIdx.x].x;
	y = citylist[threadIdx.x].y - citylist[blockIdx.x].y;
	return hypotf(x,y); // CUDA function.
}

/**
 * CUDA function for finding table distances.
 */
__global__ void compute_distances(float *table, city_t *cities) {
	if (threadIdx.x < blockIdx.x) {
		int index = blockIdx.x * (blockIdx.x - 1) / 2;
		index+= threadIdx.x;
		table[index] = dev_get_distance_between(cities);
	}
}



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
 *  Constructs the distTable. Implemented with CUDA.
 */
void construct_distTable(tour_t* cities, int num_cities) {
	int num_bytes = TABLE_SIZE * sizeof(float);
	int i;
	float *dev_table;
	city_t *dev_cities;//,*host_cities;
	city_t host_cities[MAX_CITIES];

	for (i=0;i<num_cities;i++) {
		host_cities[i]=*cities->city[i];
	}
	memset((void*)distTable, 0, sizeof(distTable));

	// allocate memory for cuda device variable
	cudaMalloc((void**)&dev_table,num_bytes);
	cudaMalloc((void**)&dev_cities,sizeof(host_cities));
	cudaMemcpy(dev_cities,host_cities,sizeof(host_cities),cudaMemcpyHostToDevice);
	cudaMemcpy(dev_table,distTable,sizeof(distTable),cudaMemcpyHostToDevice);

	// now, pass the table through to the GPU.
	compute_distances<<<num_cities,num_cities>>>(dev_table,dev_cities);

	// Now, read memory back to host
	cudaMemcpy(distTable,dev_table,num_bytes,cudaMemcpyDeviceToHost);

	// deallocate memory
	cudaFree(dev_table);
	cudaFree(dev_cities);
}

/**
 * Returns the distance traveled from a point p1 to another point p2.
 *   This is retrieved from the distTable hashtable.
 */
float lookup_distance(int p1, int p2) {
        if (p1<p2) {
                return distTable[(p2*(p2-1)/2)+p1];
        } else if (p1>p2) {
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
	tour = (tour_t*)malloc( sizeof(tour_t) );
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
	short_city=(city_t*)malloc( sizeof(city_t) );
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

int main() {
	// init the cities
	int num_cities = 5;
	tour_t* myCities = loadCities("input/cities1.in");

	// construct the distance table.
	construct_distTable(myCities,num_cities);
}
