run: main.out
	./main.out
main.out: main.cpp producer.out
	g++ main.cpp -o main.out
producer.out: producer.cpp
	g++ producer.cpp -o producer.out
clean:
	rm *.out