#include "tsp.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
typedef struct coord
{
	int nodeNum;
	int xCord;
	int yCord;
} coord;
*/

int findNearestNeighbor(int city) {
	//TODO: return integer value;
}

void printTour(city_t  *ary, int size)
{
	city_t city;
	int i = 0;
	for(i = 0; i < size; i++)
	{
		city = *(ary + i);
		//printf("Node:%i\t", *(ary+i));
		printf("Node:%i\t", city.id);
		printf("xCord:%i\t", city.x);
		printf("yCord:%i\n", city.y);
	}
}
void initialTour(int *tour, city_t *cities, int size)
{
	int startCity = random() % size;
	tour[0] = startCity;	
	int i = 0;
	for(i = 1; i < size; i++)
	{
		tour[i] = findNearestNeighbor(tour[i-1]);
	}	

	
}


int main ()
{
	FILE *fp;	//File pointer
	char *lp;	//line pointer
	char line[100];	//Holds each new line of data
	char *  token;
	city_t cities[281];	


	fp = fopen("data", "rt");
	if(fp == NULL) 
	{
		printf("Could not open file");
		exit(1);	//IDK what exit(0) is
	}
	int i;
	for(i = 0; i < 6; i++){
		fgets(line, 200, fp);
	//	printf(line);
	}	
	char delims[] = " ";
	int count = 0,innerCount = 0;;
	city_t city;

	while ( fgets(line,20,fp) != NULL)
	{
		token = strtok(line, " " );
		count = atoi(token);
		city.id = count;
		innerCount = 0;
		token = strtok(NULL, delims);
		while (token != NULL)
		{	
			if(innerCount ==0)
				city.x = atoi(token);	
			else
				city.y = atoi(token);

			innerCount++;
			token = strtok(NULL, delims);		
					
		}
		cities[count] = city;
	}
	city_t *ptr;
	ptr = &cities[0];
	int numOfCities = sizeof(cities)/sizeof(city_t);
	
	int tour[numOfCities];
	initialTour(&tour[0], ptr, numOfCities);
		
	
}
