all: affinity-test

affinity-test: main.cpp
	mpicxx -g -fopenmp $^ -o $@

clean:
	rm -f affinity-test