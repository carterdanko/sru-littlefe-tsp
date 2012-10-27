# files
sources=src/eax.c src/fitness.c src/main.c src/tsp.c src/util.c
includes=include/eax.h include/fitness.h include/tsp.h

tsp: makefile $(sources) $(includes)
	cc -lm -g $(sources) -o tsp -Iinclude/* -I.
	
genrandcity: genrandcity.c
	cc -g src/genrandcity.c -o genrandcity

clean:
	rm tsp genrandcity
