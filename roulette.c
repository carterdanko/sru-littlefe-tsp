/*
######################################################################
-Take your list of routes and add up the fitness from every route.
-Mark off values that indicate the end of each route's fitness.
-Pick a random number from 0 to max_fitness, then choose the route
 which is within the bounds of this random number.
######################################################################
*/
#include <stdio.h>

//void roulette_wheel(int fitness [population], char tour[population][tourLength],
//char parent[population][tourLength])
//TODO: clean up function header
void roulette_wheel(int *fitness, int population, char *tour, int tourLength, char *parent)
{
/*  int i;
  int j;
  int k;
  int max_fitness;*/
  int i,j,k,max_fitness,random;
  max_fitness=0;
//  cout<<endl;
//  int random;
  int total[population];
    for (i=0; i<population; i++)
     {
       max_fitness = fitness[i] + max_fitness;
       total[i] = max_fitness;
//       cout<<"Running Total ["<<i<<"]  "<<total[i]<<endl;
       printf("Runing Total %i\n",total[i]);
     }
//         cout<<endl;
//       cout<<"Max Fitness = "<<max_fitness<<endl;
//       cout<<endl;
       printf("Max Fitness = %i\n",max_fitness);
       //TODO: rewrite from here below for readability
       for (i=0; i<population; i++)
          {
//           cout<<endl;
           random = My_rand_no(max_fitness);
//           cout<<"Random Number = "<<random<<endl;
           printf("Random Number = %i\n",random);
             for (j=0; j < population; j++)
                {
                  if (random < total[j]/*~~!*/)
                {
//                      cout<<"tour   = "<<total[j]<<endl;
                      printf("tour   = %i\n",total[j]);
                        for (k=0; k<tourLength; k++)
                      {
                            parent[i][k]= tour[j][k];
//                            cout<<"Parent ["<<i<<"]        = "<<parent[i][k]<<endl;
                            printf("Parent [%i]\n",parent[i][k]);
                      }
                          j = population + 1;
                  }
                }
            }
}

int main() {

}
