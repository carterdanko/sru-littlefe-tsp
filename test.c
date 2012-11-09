#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *toursFile;

void loadTours(const char* const fileName) {
	FILE* in;
	int numCities,numTours,i,j;
	
	// open the file
	in = fopen(fileName, "r");
	
	// verification
	if (!in)
	{
		fprintf(stderr, "Error opening %s for input. Check filename", fileName);
		exit(0);
	}

	// get number of tours from first line
	fscanf(in, "%i", &numTours); 
	
	// get number of cities from first line
	fscanf(in, "%i", &numCities); 
	
	printf("received %i for tours, %i for cities.\n",numTours,numCities);

	// set up the tours
	for (i=0;i<numCities;i++) {
		for (j=0;j<numTours;j++) {
			fscanf(in, "%i", &numCities);
		}
	}
}

int main(int argc, char** argv) {
	printf("test.\n");
	toursFile = argv[1];
	loadTours(toursFile);
}
