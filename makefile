tsp: makefile main.c tsp.c eax.c util.c fitness.c parentselection.c tsp.h eax.h fitness.h
	cc -lm -g main.c tsp.c eax.c util.c fitness.c parentselection.c -o tsp

clean:
	rm tsp
