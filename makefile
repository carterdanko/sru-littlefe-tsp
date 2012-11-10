# files
sources=src/eax.c src/fitness.c src/main.c src/tsp.c src/util.c
includes=include/eax.h include/tsp.h include/outputcontrol.h include/printcolors.h

tsp: makefile $(sources) $(includes)
	cc -g $(sources) -o tsp -Iinclude/* -I. -lm -pg -DMPIFLAG=0
	
genrandcity: src/genrandcity.c
	cc -g src/genrandcity.c -o genrandcity

cuda: src/fitnesscuda.cu
	nvcc -g src/fitnesscuda.cu -o cuda -Iinclude/* -I. -lm

mpi: makefile $(sources) $(includes)
	mpicc -g $(sources) -o mpitsp -Iinclude/* -I. -lm -DMPIFLAG=1

nn: makefile src/fitness.c src/nnmain.c src/tsp.c src/util.c $(includes)
	mpicc -g src/nnmain.c src/fitness.c src/tsp.c src/util.c -o nntsp -Iinclude/* -I. -lm -DMPIFLAG=1

clean:
	rm tsp genrandcity cuda mpitsp graphBefore.txt graphAfter.txt bestTours.txt nntsp *.out output/*
