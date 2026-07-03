all: main hc rabbit salsa sosemanuk

hc: main-hc.cpp
	g++ main-hc.cpp -o hc

rabbit: main-rabbit.cpp
	g++ main-rabbit.cpp -o rabbit

salsa: main-salsa.cpp
	g++ main-salsa.cpp -o salsa

sosemanuk: main-sosemanuk.cpp
	g++ main-sosemanuk.cpp -o sosemanuk
	clear

main: main.cpp
	g++ main.cpp -o main

clear:
	rm -f main
	rm -f hc
	rm -f rabbit
	rm -f salsa
	rm -f sosemanuk
