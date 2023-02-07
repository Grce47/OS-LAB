bash: pwd delep a.out
pwd: pwd.cpp
	g++ pwd.cpp -o pwd
delep: delep.cpp
	g++ delep.cpp -o delep
a.out: Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp
	g++ Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp -std=c++17 -lstdc++fs -lreadline -o a.out
run:
	./a.out
lock: 
	g++ lock.c -o lock
	touch lock_test.txt
	./lock
install:
	sudo apt-get install libreadline6-dev
clean: 
	rm a.out pwd delep
