 for(i = 0; i <= Cities->size-4; i+=2)
        {
                LineSegment outsideSeg;
                outsideSeg.x1 = tourA.city[i]->x;
                outsideSeg.y1 = tourA.city[i]->y;
                outsideSeg.x2 = tourA.city[i+1]->x;
                outsideSeg.y2 = tourA.city[i+1]->y;
                int j;
                //This might only work for an even amount of cities?
                for(j = i+2;j <= Cities->size-2; j+=2)
                {
                        LineSegment insideSeg;
                        insideSeg.x1 = tourA.city[j]->x;
                        insideSeg.y1 = tourA.city[j]->y;
                        insideSeg.x2 = tourA.city[j+1]->x;
                        insideSeg.y2 = tourA.city[j+1]->y;
                        if(doIntersect(outsideSeg, insideSeg) == 1)
                        {
                                printf("Intersecting segments i:%d j:%d\n", i,j);

                                city_t *temp = tourA.city[i+1];
                                tourA.city[i+1] = tourA.city[j];
                                tourA.city[j] = temp;

                        }//End of if the segments intersect
                }//End of the inside j loop

        }//End of outside loop

        printf("\nPrinting tour after 2-opt\n");
        print_tour(&tourA);

