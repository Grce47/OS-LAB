run: main.out inp
	./main.out < inp > out
main.out: main.cpp
	g++ main.cpp -o main.out -lpthread
clean: 
	rm *.out