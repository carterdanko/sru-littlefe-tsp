void roulette_wheel(int fitness [population], char tour[population][tourLength],
char parent[population][tourLength])
{
  int i;
  int j;
  int k;
  int max_fitness;
  max_fitness=0;
  cout<<endl;
  int random;
  int total[population];
    for (i=0; i<population; i++)
     {
       max_fitness = fitness[i] + max_fitness;
       total[i] = max_fitness;
       cout<<"Running Total ["<<i<<"]  "<<total[i]<<endl;
     }
         cout<<endl;
       cout<<"Max Fitness = "<<max_fitness<<endl;
       cout<<endl;
       for (i=0; i<population; i++)
          {
           cout<<endl;
           random = My_rand_no(max_fitness);
           cout<<"Random Number = "<<random<<endl;
             for (j=0; j < population; j++)
                {
                  if (random < total[j]
                {
                      cout<<"tour   = "<<total[j]<<endl;
                        for (k=0; k<tourLength; k++)
                      {
                            parent[i][k]= tour[j][k];
                            cout<<"Parent ["<<i<<"]        = "<<parent[i][k]<<endl;
                      }
                          j = population + 1;
                  }
                }
            }
}
