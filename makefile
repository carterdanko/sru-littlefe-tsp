tsp: main.o eax.o
      cc -o tsp main.o eax.o
	  
main.o: main.c eax.h tsp.h
        cc -c main.c

eax.o: eax.c
       cc -c eax.c

clean: 
       rm tsp main.o eax.o