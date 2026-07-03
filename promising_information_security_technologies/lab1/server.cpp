#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "../../gmp/include/gmpxx.h"
#include "../gmp/rdtsc.h"
using namespace std;

struct argum {
    struct sockaddr_in client;
    int socket;
};

int main() {
    int socketDescrip;
    socklen_t tempLength;
    struct sockaddr_in server, client;
    char messageIn[1024];

    if ((socketDescrip = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stdout, "Socket don't open - %s.[%d]\n", strerror(errno),
                errno);
        return 4;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = 0;
    if (bind(socketDescrip, (struct sockaddr*)&server, sizeof(server))) {
        fprintf(stdout, "Bind error - %s.[%d]\n", strerror(errno), errno);
        return 4;
    }
    tempLength = sizeof(server);
    if (getsockname(socketDescrip, (struct sockaddr*)&server, &tempLength)) {
        fprintf(stdout, "Getsockname error - %s.[%d]\n", strerror(errno),
                errno);
        return 4;
    }
    fprintf(stdout, "Server port - %d\n", ntohs(server.sin_port));
    ofstream portf("port.txt");
    portf << ntohs(server.sin_port);
    portf.close();
    listen(socketDescrip, 3);

    int exitStatus = 1, c = sizeof(struct sockaddr_in), newSocket,
        colClient = 0;
    struct argum arguments[2];

    while ((newSocket = accept(socketDescrip, (struct sockaddr*)&client,
                               (socklen_t*)&c))) {
        arguments[colClient].client = client;
        arguments[colClient].socket = newSocket;
        colClient += 1;
        cout << "New connect\n";
        if (colClient == 2) break;
    }

    char buffer[2048];
    string str1, str2;
    mpz_t Z1, Z2;
    mpz_inits(Z1, Z2, 0);

    // ------------------------------------ DF multi
    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    recv(arguments[1].socket, buffer, 2048, 0);
    str2 = buffer;
    str1 += '\0';
    str2 += '\0';

    send(arguments[0].socket, str2.c_str(), str2.length(), 0);
    send(arguments[1].socket, str1.c_str(), str1.length(), 0);

    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    recv(arguments[1].socket, buffer, 2048, 0);
    str2 = buffer;
    mpz_set_str(Z1, str1.c_str(), 0);
    mpz_set_str(Z2, str2.c_str(), 0);
    cout << "Z1:\n"
         << str1 << "\nZ2:\n"
         << str2 << "\nCheck = " << (mpz_cmp(Z1, Z2) == 0) << "\n \n";

    // ----------------------------------- DF pod grup
    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    recv(arguments[1].socket, buffer, 2048, 0);
    str2 = buffer;
    str1 += '\0';
    str2 += '\0';

    send(arguments[0].socket, str2.c_str(), str2.length(), 0);
    send(arguments[1].socket, str1.c_str(), str1.length(), 0);

    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    recv(arguments[1].socket, buffer, 2048, 0);
    str2 = buffer;
    mpz_set_str(Z1, str1.c_str(), 0);
    mpz_set_str(Z2, str2.c_str(), 0);
    cout << "Z1:\n"
         << str1 << "\nZ2:\n"
         << str2 << "\nCheck = " << (mpz_cmp(Z1, Z2) == 0) << "\n \n";

    // ---------------------------------- MQV
    string str3, str4;
    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    str1 += '\0';
    recv(arguments[1].socket, buffer, 2048, 0);
    str3 = buffer;
    str3 += '\0';

    send(arguments[0].socket, str3.c_str(), str3.length(), 0);
    send(arguments[1].socket, str1.c_str(), str1.length(), 0);

    recv(arguments[0].socket, buffer, 2048, 0);
    str2 = buffer;
    str2 += '\0';
    recv(arguments[1].socket, buffer, 2048, 0);
    str4 = buffer;
    str4 += '\0';

    send(arguments[0].socket, str4.c_str(), str4.length(), 0);
    send(arguments[1].socket, str2.c_str(), str2.length(), 0);

    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    recv(arguments[1].socket, buffer, 2048, 0);
    str2 = buffer;
    mpz_set_str(Z1, str1.c_str(), 0);
    mpz_set_str(Z2, str2.c_str(), 0);
    cout << "Z1:\n"
         << str1 << "\nZ2:\n"
         << str2 << "\nCheck = " << (mpz_cmp(Z1, Z2) == 0) << "\n \n";

    close(socketDescrip);
    return 0;
}