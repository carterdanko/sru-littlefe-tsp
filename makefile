# files
sources=src/eax.c src/fitness.c src/main.c src/tsp.c src/util.c
includes=include/eax.h include/tsp.h include/outputcontrol.h include/printcolors.h

tsp: makefile $(sources) $(includes)
	cc -g $(sources) -o tsp -Iinclude/* -I. -lm -pg
	
genrandcity: src/genrandcity.c
	cc -g src/genrandcity.c -o genrandcity

cuda: src/fitnesscuda.cu
	nvcc -g src/fitnesscuda.cu -o cuda -Iinclude/* -I. -lm

mpi: makefile $(sources) $(includes)
	mpicc -g $(sources) -o mpitsp -Iinclude/* -I. -lm

clean:
	rm tsp genrandcity cuda mpitsp graphBefore.txt graphAfter.txt bestTours.txt *.out output/*
