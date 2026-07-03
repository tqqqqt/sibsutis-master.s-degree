GMP = -I /home/user/gmp/include -L /home/user/gmp/lib -lgmpxx -lgmp

all: 2 3 4
	# all compiles complete

2: gener.cpp
	# compile generator
	g++ gener.cpp $(GMP) -o gener

3: offline.cpp
	# compile ofline alg
	g++ offline.cpp $(GMP) -o offline

4: client.cpp server.cpp
	# compile online alg
	g++ -g client.cpp $(GMP) -o client
	g++ -g server.cpp $(GMP) -o server

clear:
	rm -f gener
	rm -f offline
	rm -f client
	rm -f server
