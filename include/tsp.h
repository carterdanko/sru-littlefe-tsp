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
#if MPIFLAG
#include <mpi.h>
#endif

#include "include/printcolors.h"     // for coloring the console output
#include "include/outputcontrol.h"   // controls debug output

#define DEBUG 1     // set to zero to remove a lot of debugging output and speed up the code 
#define DPRINTF if (DEBUG) printf
#define MAX_CITIES 1000
#define MAX_TOUR 1001     // this should basically be the same as MAX_CITIES
#define MAX_POPULATION 100
#define TABLE_SIZE (MAX_CITIES*(MAX_CITIES-1))/2 // size based on a counting argument
#define MAX_ITERATIONS 100 // sets the maximum number of generations to iterate through in the GA
#define DELTA 0.50 // A float threshold for the difference in the population's best fitness.
		// When the difference is within this threshold, begin counting how frequently it occurs.
#define MAX_DELTA 20 // set the maximum number of generations to iterate through when the difference in fitness was repetitively within DELTA.
#define MPI_TAG 2
#define MAX_PAIR_TOURS MAX_POPULATION

#define ENFORCE_LOOKUP_TABLE_CORRECTNESS 0    // extra checks in the lookup table for debugging purposes
#define DEBUG_SET_TOUR_FITNESS 0              // inserts extra lines while calculating a tour's fitness for debugging

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
	int tour[MAX_TOUR]; // keeps track of which tour this "edge" was from. An "edge"[n] is defined as: tour.city[n-1] -> tour.city[n], then tour.tour[n] == which tour that edge was from
					    // NOTE: this field is only really relevant when dealing with cycles
	int size; // size of the tour
	// ~~!
	float fitness; // the fitness of the entire tour.
} tour_t;

// main.c
extern int randSeed; // random seed to use
extern char* citiesFile; // cities file name
extern tour_t** Tours; // Tours array
#if BEST_TOUR_TRACKING
	extern tour_t** BestTours;          // array containing the best tours
	extern tour_t* lastBestTour;        // the previous best tour, last iteration
	void dumpBestTours();      // a function in main.c that dumps the best tours
#endif // best tour tracking
///////////////////////////////////////////////////////////////////////////////


// util.c
void terminate_program(int ecode); // terminates the program and outputs some information
void city_tToInt(tour_t* C, int nCities, int* I);
void intToCity_t(int* I, int nCities, tour_t* C);
void tour_tToInt(tour_t** tours, int nTours, int* I);
void intToTour_t(tour_t* Cities, int* I, int nTours, tour_t** tours);
float frand(); // Returns a random float between 0.0 and 1.0.
void print_tour(tour_t* tour); // Simple print procedure for a tour.
void mergeToursToPop(tour_t** tours, int num_tours, tour_t** toursToMerge, int numToursToMerge); // merges tours into the master array of tours
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

void terminate_program(int ecode);

void populate_tours(int N, int mpi_rank, tour_t** arr_tours, tour_t* arr_cities);

void MPI_init(char *mpi_flag, int *mpi_rank, int *mpi_procs, int *argc, char ***argv);

void load_cities(int mpi_rank, char *citiesFile, tour_t *arr_cities);

void master_listener(int *iter, int *delta_iter, char *lcv, tour_t** arr_tours, int mpi_procs);

void serial_listener(int *iter,int *delta_iter,char *lcv,tour_t** arr_tours, int N);

void run_genalg(int N, char* lcv, tour_t** arr_tours, int mpi_flag);

void sortTours(tour_t** tours, int numTours);

void merge_sort(tour_t** tours, int a, int b);

void merge_recursive(tour_t** tours, int a, int mid, int b);

void merge_swap(tour_t** elem1, tour_t** elem2);

void getBestTours(int max, tour_t** tours, tour_t** bestTours);

#endif // header guard
