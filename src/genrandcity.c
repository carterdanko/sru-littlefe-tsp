/**
 * this program creates random city files
 */
 
#include <stdio.h>

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
				//TODO: num cities
				numCities = atoi(argv[++i]);
			}
			else if (strcmp(p, "-d") == 0)
			{
				//TODO: max dist
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
	
	// generate the file
	printf("saving %i cities maximally %i distance away to '%s'.\n", numCities, maxDist, fn); // TODO: DEBUG remove
	FILE* F = fopen(fn, "w");
	printf("writing");
	fprintf(F, "%i\n", numCities);
	for (i=0; i< numCities; i++)
	{
		printf(".");
		fprintf(F, "%i %i\n", rand() % maxDist, rand() % maxDist);
	}
	printf("done!\n");
	fclose(F);
}// int main()