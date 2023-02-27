run: main.out
	./main.out
main.out: main.cpp producer.out consumer.out
	g++ main.cpp -o main.out
producer.out: producer.cpp
	g++ producer.cpp -o producer.out
consumer.out: consumer.cpp
	g++ consumer.cpp -o consumer.out
clean:
	rm *.out