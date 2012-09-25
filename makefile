tsp: main.o eax.o tsp.o
	cc -o tsp main.o eax.o tsp.o
	  
main.o: main.c eax.h tsp.h
	cc -c main.c
	
tsp.o: tsp.c
	cc -c tsp.c

eax.o: eax.c
	cc -c eax.c

clean: 
	rm tsp main.o eax.o
