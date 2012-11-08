# files
sources=src/eax.c src/fitness.c src/main.c src/tsp.c src/util.c
includes=include/eax.h include/tsp.h include/outputcontrol.h include/printcolors.h

tsp: makefile $(sources) $(includes)
	cc -lm -g $(sources) -o tsp -Iinclude/* -I. -pg
	
genrandcity: src/genrandcity.c
	cc -g src/genrandcity.c -o genrandcity

cuda: src/fitnesscuda.cu
	nvcc -lm -g src/fitnesscuda.cu -o cuda -Iinclude/* -I.

mpi: makefile $(sources) $(includes)
	mpicc -lm -g $(sources) -o mpitsp -Iinclude/* -I.

clean:
	rm tsp genrandcity cuda mpitsp graphBefore.txt graphAfter.txt bestTours.txt *.out
