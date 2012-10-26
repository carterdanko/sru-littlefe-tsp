tsp: main.o eax.o tsp.o util.o fitnessfunction.o parentselection.o
	cc -lm -o tsp main.o eax.o tsp.o util.o fitnessfunction.o parentselection.o
	  
main.o: main.c eax.h tsp.h
	cc -c main.c
	
tsp.o: tsp.c
	cc -c tsp.c

eax.o: eax.c
	cc -c eax.c

util.o: util.c
	cc -c util.c

fitnessfunction.o: fitnessfunction.c
	cc -c fitnessfunction.c

parentselection.o: parentselection.c
	cc -c parentselection.c

clean: 
	rm tsp *.o 
