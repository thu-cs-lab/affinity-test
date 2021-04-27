all: affinity-test

affinity-test: main.cpp
	mpicxx -g -pthread -fopenmp $^ -o $@

clean:
	rm -f affinity-test