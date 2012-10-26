tsp: makefile main.c tsp.c eax.c util.c fitness.c tsp.h eax.h fitness.h
	cc -lm -g main.c tsp.c eax.c util.c fitness.c -o tsp

clean:
	rm tsp
