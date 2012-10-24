tsp: makefile main.c tsp.c eax.c fitness.c util.c tsp.h eax.h fitness.h
	cc -lm -g main.c tsp.c eax.c fitness.c util.c -o tsp

clean:
	rm tsp
