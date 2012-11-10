/**
 * contains types and what not that will be used throughout the entirety of the TSP algorithm
 */
#ifndef TSP_H // header guard
#define TSP_H
//#define MPIFLAG 1 // this decides whether or not we are using MPI (for compiling purposes)
	//now implemented with the makefiles

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
#define CAP_ROULETTE_WHEEL 0         // if true, puts limits on roullette_wheel
#if CAP_ROULETTE_WHEEL
	#define RW_CAP_MIN 1.0 // this is a minimum weight for each child in the roullette wheel selection (times average)
	#define RW_CAP_MAX 1.0 // then this is a maximum weight for each child in the roullette wheel selection (times average)
#endif
////////////////////////////////////////////////////////////

///////////////////// TOUR GENERATION MODIFICATIONS ////////
#define USE_NEAREST_NEIGHBOR 1       // if true, uses nearest neighbor instead of purely random initial pop
#define USE_HYBRID_PROBABILITY 50    // if 0, only use NN or RAND, if non-zero, this/100 = probability for NN
////////////////////////////////////////////////////////////

////////////////////// OTHER MODIFICATIONS /////////////////
#define USE_DISTANCE_TABLE 1         // if true, uses a distance lookup table
#define USE_BIG_TABLE 0              // if true, uses an n^2 table (NxN) instead of the reduced triangle-table
#define LEAVE_SQUARED 0              //TODO: DOESN'T SEEM TO WORK if true, leaves the distance squared
#define USE_EDGE_TABLE 0             //TODO: THIS ISN'T FINISHED AND WON'T COMPILE if true, uses a huge table of edges instead of INIT_EDGEing all the time
#define USE_NAIVE_DISTANCE 0         // if true, distance calculation is simply: (x2-x1)+(y2-y1)
////////////////////////////////////////////////////////////

#include "include/printcolors.h"     // for coloring the console output
#include "include/outputcontrol.h"   // controls debug output

#define DEBUG 1     // set to zero to remove a lot of debugging output and speed up the code 

#if PRINT_MPI_INFO
	#define DPRINTF if (DEBUG) printf("r%io%i::", mpi_rank, outputCounter++); if (DEBUG) printf
#else
	#define DPRINTF if (DEBUG) printf
#endif
#define MAX_CITIES 20000

#define MAX_TOUR MAX_CITIES+1     // maximum length of a tour, MAX_CITIES+1 because sometimes tours loop back around
#define MAX_POPULATION 100
#define TABLE_SIZE (MAX_CITIES*(MAX_CITIES-1))/2 // size based on a counting argument
#define MAX_ITERATIONS 100 // sets the maximum number of generations to iterate through in the GA
#define DELTA 0.50 // A float threshold for the difference in the population's best fitness.
		// When the difference is within this threshold, begin counting how frequently it occurs.
#define MAX_DELTA 20 // set the maximum number of generations to iterate through when the difference in fitness was repetitively within DELTA.
#define MAX_PAIR_TOURS MAX_POPULATION   // how many of pairs of tours (parent pairs) to make when generating children
#define NUM_TOP_TOURS 3       // how many tours master sends to each slave
#define SEND_TO_MASTER 1      // how many tours each slave sends to master

#define ENFORCE_LOOKUP_TABLE_CORRECTNESS 0    // extra checks in the lookup table for debugging purposes
#define ENFORCE_NONZERO_FITNESS 0             // extra checks when using a fitness value (roullette_select, etc.)
#define DEBUG_SET_TOUR_FITNESS 0              // inserts extra lines while calculating a tour's fitness for debugging

/**
 * represents a city that must be visited to create a complete tour
 */
struct edge_struct;
typedef struct {
	int x,y; // x and y position of the city
	int id; // a unique number for each city in the map. It should be equal to the city's index in the cities array.
	int tour; // keeps track of which tour this "edge" was from. An "edge"[n] is defined as: tour.city[n-1] -> tour.city[n], then tour.tour[n] == which tour that edge was from
			  // NOTE: this field is only really relevant when dealing with cycles 
	struct edge_struct* edge; // like tour, this is the actual "edge" that connected this to the city before it in the tour. These are changed
	              // all the time by every iteration of the eax, so they're really only valid for the brief period during fixIntermediate
				  // pretty much
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
extern tour_t *CitiesA, *CitiesB;
extern time_t startTime; // time the program started running
#if BEST_TOUR_TRACKING
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
void load_cities(int mpi_rank, char *citiesFile, tour_t **arr_cities);
///////////////////////////////////////////////////////////////////////////////

// fitness.c
tour_t* roulette_select(tour_t** tours, int num_tours, tour_t* ignore_tour);  // selects a random tour weighted by its fitness.
city_t* find_nearest_neighbor(city_t* city, int num_cities, tour_t* cities, char* cities_visited); // find nearest neighbor of a given city.
void set_tour_fitness(tour_t* tour, int num_cities); // compute and set a tour's fitness.
float lookup_distance(int p1, int p2); // distance between two points; retrieved in constant time.
void construct_distTable(tour_t* cities, int num_cities); // constructs distTable variable.
///////////////////////////////////////////////////////////////////////////////

#endif // header guard
