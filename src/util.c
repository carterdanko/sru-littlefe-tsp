#include "include/tsp.h"

float frand() {
	return ((float)rand())/((float)RAND_MAX);
}

void print_tour(tour_t* tour) {
	int i;
	printf("Tour: [%i]", tour->city[0]->id);
	for (i=1; i < tour->size; i++)
		printf(", [%i]", tour->city[i]->id);
	printf("\n");
}

void dprint_tour(tour_t* tour) {
	int i;
	DPRINTF("Tour: [%i]", tour->city[0]->id);
	for (i=1; i < tour->size; i++)
		DPRINTF(", [%i]", tour->city[i]->id);
	DPRINTF("\n");
}

int doIntersect(LineSegment a, LineSegment b){
//receives 2 line segments & tells if they intersect.
//param a & b represent the two arcs a0-a1 and b0-b1, where the
//   values for a & b are the point numbers in the points array.
//   Eg. (2,9) would be arc from point#2 to point#9

        //Var Dictionary
        int x1,y1,x2,y2,x3,y3,x4,y4;    //Bourke's names for stuff
        float pbDenom;                                  //denominator of Bourke's stuff
        float ua,ub;                                    //Bourke's names for test results
        int doCross = 0;                   //flag to tell outcome: guess 'NO'

        //convert param's to Bourke's variable names
        x1= a.x1;               y1=a.y1;
        x2= a.x2;               y2=a.y2;

        x3= b.x1;               y3=b.y1;
        x4= b.x2;               y4=b.y2;

        //from Paul Bourke   http://paulbourke.net/geometry/lineline2d/
        pbDenom = (float)(y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
        if (pbDenom != 0.0) {//testing float for zero (but is converted int)
                ua = ((x4-x3)*(y1-y3)-(y4-y3)*(x1-x3))/pbDenom;
                ub = ((x2-x1)*(y1-y3)-(y2-y1)*(x1-x3))/pbDenom;
                //if 0<ua<1 and 0<ub<1 then line segments intersect
                doCross = ( (0.0<ua) && (ua<1.0) && (0.0<ub) && (ub<1.0));
        }

        return doCross;
}//doIntersect


