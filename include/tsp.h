/**
 * contains types and what not that will be used throughout the entirety of the TSP algorithm
 */
#ifndef TSP_H // header guard
#define TSP_H
#define MPIFLAG 0 // this decides whether or not we are using MPI (for compiling purposes)

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//////////////////////// MPI STUFF /////////////////////////
#if MPIFLAG
#include <mpi.h>
#define MPI_TAG 2
#endif
////////////////////////////////////////////////////////////

////////////////////// GA MODIFICATIONS ////////////////////
#define CHOOSE_BEST_FROM_THREE 1     // if true, performEAX returns only the best tour from {parentA, parentB, child} instead of always the child
#define PERFORM_2OPT_ON_CHILD 0      // if true, performs 2-opt optimization on child tour
#define USE_HEURISTIC_ESET 0         // if true, instead of randomly choosing AB cycles for E-SETs, uses a heuristic
////////////////////////////////////////////////////////////

////////////////////// Other Modifications /////////////////
#define USE_DISTANCE_TABLE 1         // if true, uses a distance table to look up distances between cities, otherwise calculates it on the fly
////////////////////////////////////////////////////////////

#include "include/printcolors.h"     // for coloring the console output
#include "include/outputcontrol.h"   // controls debug output

#define DEBUG 1     // set to zero to remove a lot of debugging output and speed up the code 

#define DPRINTF if (DEBUG) printf("r%io%i::", mpi_rank, outputCounter++); if (DEBUG) printf
#define MAX_CITIES 15000

#define MAX_TOUR MAX_CITIES+1     // this should basically be the same as MAX_CITIES
#define MAX_POPULATION 100
#define TABLE_SIZE (MAX_CITIES*(MAX_CITIES-1))/2 // size based on a counting argument
#define MAX_ITERATIONS 100 // sets the maximum number of generations to iterate through in the GA
#define DELTA 0.50 // A float threshold for the difference in the population's best fitness.
		// When the difference is within this threshold, begin counting how frequently it occurs.
#define MAX_DELTA 20 // set the maximum number of generations to iterate through when the difference in fitness was repetitively within DELTA.
#define MAX_PAIR_TOURS MAX_POPULATION
#define NUM_TOP_TOURS 5

#define ENFORCE_LOOKUP_TABLE_CORRECTNESS 0    // extra checks in the lookup table for debugging purposes
#define DEBUG_SET_TOUR_FITNESS 0              // inserts extra lines while calculating a tour's fitness for debugging

/**
 * represents a city that must be visited to create a complete tour
 */
typedef struct {
	int x,y; // x and y position of the city
	int id; // a unique number for each city in the map. It should be equal to the city's index in the cities array.
	int tour; // keeps track of which tour this "edge" was from. An "edge"[n] is defined as: tour.city[n-1] -> tour.city[n], then tour.tour[n] == which tour that edge was from
			  // NOTE: this field is only really relevant when dealing with cycles 
} city_t;

/**
 * represents a possible tour of the cities
 */
typedef struct {
	int size; // size of the tour
	// ~~!
	float fitness; // the fitness of the entire tour.
	city_t* city[MAX_TOUR]; // a pointer to each city in the tour
} tour_t;

// main.c
extern int mpi_rank;
extern int randSeed; // random seed to use
extern char* citiesFile; // cities file name
extern tour_t** Tours; // Tours array
extern tour_t *CitiesA;
extern tour_t *CitiesB;
extern time_t startTime; // time that the program started running
#if BEST_TOUR_TRACKING
	extern tour_t** BestTours;          // array containing the best tours
	extern tour_t* lastBestTour;        // the previous best tour, last iteration
	void dumpBestTours();      // a function in main.c that dumps the best tours
#endif // best tour tracking
///////////////////////////////////////////////////////////////////////////////


// util.c
extern int outputCounter;
void terminate_program(int ecode); // terminates the program and outputs some information
void city_tToInt(tour_t* C, int nCities, int* I);
void intToCity_t(int* I, int nCities, tour_t* C);
void tour_tToInt(tour_t** tours, int nTours, int* I);
void intToTour_t(tour_t* Cities, int* I, int nTours, tour_t** tours);
float frand(); // Returns a random float between 0.0 and 1.0.
void print_tour(tour_t* tour); // Simple print procedure for a tour.
void mergeToursToPop(tour_t** tours, int num_tours, tour_t** toursToMerge, int numToursToMerge); // merges tours into the master array of tours
void sortTours(tour_t** tours, int numTours); // sorts an array of tours by ascending fitness values (lower fitness is better tour)
void merge_swap(tour_t** elem1, tour_t** elem2); // swaps two tours by reference (that is, after this executes, elem1 == (previous value of elem2) and elem2 == (previous elem1)
void terminate_program(int ecode); // terminates the program. ecode 0 is normal successful termination, anything else indicates an error (abnormal termination)
void getBestTours(int max, tour_t** tours, tour_t** bestTours);
///////////////////////////////////////////////////////////////////////////////

// tsp.c
tour_t* loadCities(const char* const fileName); // loads cities from file
void freeCities(tour_t* cities); // frees the memory used by the structure
tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities); // create nearest neighbor tour based on an initial city.
tour_t* create_tour_rand(tour_t* cities); // creates tours by randomly choosing cities
///////////////////////////////////////////////////////////////////////////////

// fitness.c
tour_t* roulette_select(tour_t** tours, int num_tours, tour_t* ignore_tour);  // selects a random tour weighted by its fitness.
city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited); // find nearest neighbor of a given city.
void set_tour_fitness(tour_t* tour, int num_cities); // compute and set a tour's fitness.
float lookup_distance(int p1, int p2); // distance between two points; retrieved in constant time.
void construct_distTable(tour_t* cities, int num_cities); // constructs distTable variable.
///////////////////////////////////////////////////////////////////////////////

#endif // header guard
