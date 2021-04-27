all: affinity-test

affinity-test: main.cpp
	mpicxx -g -fopenmp -pthread $^ -o $@

clean:
	rm -f affinity-test