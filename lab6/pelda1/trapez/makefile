all: trap trap1 trap2a trap2b

trap: trap.c
	gcc -o trap trap.c -Wall
	
trap1: omp_trap1.c
	gcc -o trap1 omp_trap1.c -fopenmp -Wall

trap2a: omp_trap2a.c
	gcc -o trap2a omp_trap2a.c -fopenmp -Wall
	
trap2b: omp_trap2b.c
	gcc -o trap2b omp_trap2b.c -fopenmp -Wall
        			