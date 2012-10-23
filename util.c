#include "tsp.h"

float frand() {
	int a;
	float f;
	a=rand();
	f = ((float)a)/((float)RAND_MAX);
	return f;
}

void print_tour(tour_t* tour, int num_cities) {
	int i;

	printf("Tour: [%i]", tour->city[0]->id);
	for (i=1; i < num_cities; i++)
		printf(", [%i]", tour->city[i]->id);
	printf("\n");
}
