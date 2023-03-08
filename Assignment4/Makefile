run: sns.out
	./sns.out

debug: 
	g++ -std=c++11 sns.cpp -o sns_debug.out -lpthread -g
	gdb ./sns_debug.out

sns.out: sns.cpp
	g++ -std=c++11 sns.cpp -o sns.out -lpthread

clean: 
	rm sns.out