 #include "tsp.h"
 
 /**
  * loads all of the cities found in fileName
  * fileName : the name of the file to load. The format:
  *      line 0 : <int : # of cities>
  *      line 1..# : <int : city X> <int : city Y>
  * returns : a "tour" object, but really the tour object is just used as a master array to hold all of the cities in order. Be sure to free this memory when finished.
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
	tour_t* cities = (tour_t*)malloc(sizeof(tour_t)); //TODO: FREE_MEM: remember to free this memory when finished  (memTourMaster)
	cities->size = numCities;
	
	// load information about each city
	int i;
	for (i=0; i < numCities; i++)
	{
		cities->city[i] = (city_t*)malloc(sizeof(city_t)); // TODO: FREE_MEM: remember to free this memory when finished  (memTourMasterCity)
		fscanf(in, "%i %i", &cities->city[i]->x, &cities->city[i]->y);
		cities->city[i]->id = i;
	}
	
	// all of the city information has been loaded, return the result
	return cities;
 }
 
 /**
  * frees all of the memory used by the tour structure, including every city pointed to by the tour
  * this is pretty much only useful fore freeing the master cities structure in main
  * cities : the tour structure containing all of the cities
  * returns : void
  * side-effects : cities will point to garbage memory, you should make sure to set it to null
  */
 void freeCities(tour_t* cities)
 {
	int i;
	for (i=0; i < cities->size; i++)
		free(cities->city[i]);
		
	free(cities);
 }


