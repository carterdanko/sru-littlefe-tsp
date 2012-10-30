# files
sources=src/eax.c src/fitness.c src/main.c src/tsp.c src/util.c
includes=include/eax.h include/fitness.h include/tsp.h

tsp: makefile $(sources) $(includes)
	cc -lm -g $(sources) -o tsp -Iinclude/* -I.
	
genrandcity: src/genrandcity.c
	cc -g src/genrandcity.c -o genrandcity

cuda: src/fitnesscuda.cu
	nvcc -lm -g src/fitnesscuda.cu -o cuda -Iinclude/* -I.

clean:
	rm tsp genrandcity cuda
