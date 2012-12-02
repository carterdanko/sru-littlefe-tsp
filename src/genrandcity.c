////////////////////////////////////////////////////////////////////////////////
//	DESC:	Creates random city files to be used by the GA program.
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	// check number of parameters
	char* fn = 0;
	int numCities = 50;
	int maxDist = 100;
	int i;
	
	if (argc < 2)
	{
		printf("Usage: %s [flags] <cities filename to save>\n", argv[0]);
		printf("Try -h or --help for more information.\n");
		exit(1); // ERROR: must supply a filename for the cities
	}
	else // process params
	{
		
		for (i=1; i < argc; i++)
		{
			char* p = argv[i];
			if (strcmp(p, "-h") == 0 || strcmp(p, "-H") == 0 || strcmp(p, "--help") == 0 || strcmp(p, "--HELP") == 0)
			{
				printf("Usage: %s [flags] <cities filename to save>\n", argv[0]);
				printf("-h, --help : this screen.\n");
				printf("-n <num cities> : how many cities to create.\n");
				printf("-d <maximum distance> : max x or y.\n");
			}
			else if (strcmp(p, "-n") == 0)
			{
				numCities = atoi(argv[++i]);
			}
			else if (strcmp(p, "-d") == 0)
			{
				maxDist = atoi(argv[++i]);
			}
			else
			{
				fn = argv[i];
			}// else filename
		}// for each argument
		if (!fn)
		{
			printf("please specify a file to save as. Try '%s --help' for more information.\n", argv[0]);
			exit(-1);
		}
	}// else process the arguments
	
	// track locations of cities into an array
	char* tracking = malloc(sizeof(char) * maxDist * maxDist);
	memset(tracking, 0, sizeof(*tracking));
	
	// generate the file
	printf("saving %i cities maximally %i distance away to '%s'.\n", numCities, maxDist, fn);
	FILE* F = fopen(fn, "w");
	printf("writing");
	fprintf(F, "%i\n", numCities);
	int x,y;
	for (i=0; i< numCities; i++)
	{
		printf(".");
		do
		{
			x = rand() % maxDist;
			y = rand() % maxDist;
		} while (tracking[x*maxDist+y]);
		tracking[x*maxDist+y] = 1;
		fprintf(F, "%i %i\n", x, y);
	}
	printf("done!\n");
	fclose(F);
}
