test: ./mergesort.out 
	./mergesort.out
mergesort.out: mergesort.cpp libmgoodmalloc.a
	g++ mergesort.cpp -omergesort.out -L. -lmgoodmalloc 
libmgoodmalloc.a: goodmalloc.o
	ar rcs libmgoodmalloc.a goodmalloc.o
goodmalloc.o: goodmalloc.cpp
	g++ -c goodmalloc.cpp -o goodmalloc.o
clean:
	rm *.out *.a *.o 