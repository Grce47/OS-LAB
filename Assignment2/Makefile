bash: pwd delep lock sb malware.out a.out
malware.out: malware.c
	gcc malware.c -o malware.out
pwd: pwd.cpp
	g++ pwd.cpp -o pwd
	sudo cp pwd /usr/local/bin
delep: delep.cpp
	g++ delep.cpp -o delep
	sudo cp delep /usr/local/bin
sb: sb.cpp
	g++ sb.cpp -o sb
	sudo cp sb /usr/local/bin
a.out: Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp
	g++ Assignment2_8_20CS30034_20CS30051_20CS10068_20CS30022.cpp -std=c++17 -lstdc++fs -lreadline -o a.out
lock: lock.c
	g++ lock.c -o lock
	touch lock_test.txt
install:
	sudo apt-get install libreadline6-dev
clean: 
	sudo rm malware.out a.out sb pwd delep lock /usr/local/bin/delep /usr/local/bin/pwd lock_test.txt 