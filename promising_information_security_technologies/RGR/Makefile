GMP = -I /home/user/gmp/include -L /home/user/gmp/lib -lgmpxx -lgmp

all: client.cpp server.cpp sha256.cpp
	g++ -g client.cpp sha256.cpp $(GMP) -o client
	g++ -g server.cpp sha256.cpp $(GMP) -o server

clear:
	rm -f recv.bmp
	rm -f port.txt
	rm -f client
	rm -f server
