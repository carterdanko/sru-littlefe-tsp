////////////////////////////////////////////////////////////////////////////////
//	DESC:	Contains functions for I/O and memory management for the program.
////////////////////////////////////////////////////////////////////////////////

 #include "include/tsp.h"
 
/**
 * DESC: Loads cities from a file.
 *
 * mpi_rank : the rank of the mpi process
 * citiesFile : the string that is the file name
 * arr_cities : a pointer, passed by reference (so whatever pointer was passed in will now point to the cities loaded)
 */
void load_cities(int mpi_rank, char *citiesFile, tour_t **arr_cities)
{
	// if master...
	if (mpi_rank==0)
	{
		// load the cities specified by the file
		DPRINTF("Loading cities...");
		*arr_cities = loadCities(citiesFile);
		if (!(*arr_cities))
		{
			printf("Error while loading cities. refer to error log? halting.\n");
			terminate_program(5); // ERROR: error while loading cities
		}
		DPRINTF("done! (loaded %i cities from the file)\n",
			(*arr_cities)->size);
	}
	DPRINTF("done! (loaded %i cities from the file)\n",
		(*arr_cities)->size);
	// process the cities
	int N = (*arr_cities)->size;

	// output the city information to the console
#if PRINT_CITIES
	DPRINTF("\nNum Cities: %04i\n", (*arr_cities)->size);
	DPRINTF("---------------------------\n");
	int i;
	for (i=0; i < (*arr_cities)->size; i++)
	{
		DPRINTF("City[%04i] at %04i, %04i   [id: %04i]\n", i,
			(*arr_cities)->city[i]->x, (*arr_cities)->city[i]->y,
			(*arr_cities)->city[i]->id);
	}
	DPRINTF("\n");
#endif
}

/**
 * DESC: Loads all of the cities found in fileName.
 *
 * fileName : the name of the file to load. The format:
 *      line 0 : <int : # of cities>
 *      line 1..# : <int : city X> <int : city Y>
 * returns : a "tour" object, but really the tour object is just used as a
 *	master array to hold all of the cities in order. Be sure to free this
 *	memory when finished.
 */
tour_t* loadCities(const char* const fileName)
{
	FILE* in;
	int numCities;
	
	// open the file
	in = fopen(fileName, "r");
	
	// verification
	if (!in)
	{
		fprintf(stderr, "Error opening %s for input. Check filename", fileName);
		return 0;
	}
	
	// get number of cities from first line
	fscanf(in, "%i", &numCities); 
	
	// create the tour
	tour_t* cities = (tour_t*)malloc(sizeof(tour_t));
	cities->size = numCities;
	
	// load information about each city
	int i;
	for (i=0; i < numCities; i++)
	{
		cities->city[i] = (city_t*)malloc(sizeof(city_t));
		fscanf(in, "%i %i", &cities->city[i]->x, &cities->city[i]->y);
		cities->city[i]->id = i;
	}
	
	// all of the city information has been loaded, return the result
	return cities;
}
 
/**
 * DESC: Frees all of the memory used by the tour structure, including every
 *	city pointed to by the tour.
 *
 * this is pretty much only useful fore freeing the master cities structure in
 *	main
 * cities : the tour structure containing all of the cities
 * returns : void
 * side-effects : cities will point to garbage memory, you should make sure to
 *	set it to null
 */
void freeCities(tour_t* cities)
{
	int i;
	for (i=0; i < cities->size; i++)
		free(cities->city[i]);
	free(cities);
}
 
/**
 * DESC: Generates the nearest neighbor tour based on a random city.
 *
 * city : city to start with
 * num_cities : number of cities there are
 * cities : the master array of city objects
 */
tour_t* create_tour_nn(city_t* city, int num_cities, tour_t* cities) {
	// Set up the cities_visited array; 0 for not visited, 1 for visited.
	char cities_visited[MAX_CITIES]; /*	keeps track of which cities we've
									visited so far */
	tour_t* tour; // The tour to be returned.
	city_t* next_city; // The next city to place in the tour.
	int i; // loop control
	
	
	memset(cities_visited, 0, MAX_CITIES * sizeof(char)); // set all to false
	tour = malloc( sizeof(tour_t) ); // instantiate tour
	
	// Init to be the city passed into the function
	next_city = city;
	// The first city is city passed.
	tour->city[0] = city;
	cities_visited[city->id] = 1;

	// Iterate through the cities, adding new ones and marking them off.
	for (i=1;i<num_cities;i++)
	{
		next_city = find_nearest_neighbor(next_city, num_cities, cities, cities_visited);
		tour->city[i] = next_city;
		cities_visited[next_city->id] = 1;
	}

	// Before returning, set the tour's size.
	tour->size = num_cities;
	return tour;
}

/**
 * DESC: Creates a tour by randomly iteration over the cities.
 *
 * Cities : the master cities structure
 * returns : pointer to a tour created on the heap (will have to manually free
 *	it to avoid memory leaks).
 */
tour_t* create_tour_rand(tour_t* cities)
{
	tour_t* tour = malloc(sizeof(tour_t)); // return value
	memcpy(tour, cities, sizeof(tour_t));
	city_t* swap; // temp for swapping
	
	// iterate over tour, swapping a random city with the last city in the array
	int a, b;
	for (a=tour->size; a > 1; a--)
	{
		b = rand() % a;
		swap = tour->city[a-1];
		tour->city[a-1] = tour->city[b];
		tour->city[b] = swap;
	}
	
	return tour;
}
