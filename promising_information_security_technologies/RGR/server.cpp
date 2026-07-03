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
    cout << "H(Z1):\n"
         << str1 << "\nH(Z2):\n"
         << str2 << "\nCheck = " << (str1 == str2) << "\n \n";

    int flg = 0;
    if (str1 == str2) {
        str1 = "Connect";
        str1 += '\0';
        flg = 1;
    } else {
        str1 = "ErrorConnect";
        str1 += '\0';
        flg = -1;
    }
    send(arguments[0].socket, str1.c_str(), str1.length(), 0);
    send(arguments[1].socket, str1.c_str(), str1.length(), 0);

    if (flg == -1) close(socketDescrip);

    str1 = "1Send";
    str1 += '\0';
    str2 = "2Recv";
    str2 += '\0';
    send(arguments[0].socket, str1.c_str(), str1.length(), 0);
    send(arguments[1].socket, str2.c_str(), str2.length(), 0);

    char buf[256];
    int fl = 0;
    cout << "Start resend file...";
    while (1) {
        memset(buf, 0, 256);
        recv(arguments[0].socket, buf, 256, 0);
        if (strcmp(buf, "EndSend") == 0) fl = 1;
        send(arguments[1].socket, buf, 256, 0);
        recv(arguments[1].socket, buffer, 2048, 0);
        send(arguments[0].socket, str1.c_str(), str1.length(), 0);
        if (fl == 1) break;
    }
    cout << "complete\nWait hash from client_1...";

    str1 = "H";
    str1 += '\0';
    send(arguments[0].socket, str1.c_str(), str1.length(), 0);
    recv(arguments[0].socket, buffer, 2048, 0);
    str1 = buffer;
    send(arguments[1].socket, str1.c_str(), str1.length(), 0);
    cout << "complete\nSend hash to client_2...";
    recv(arguments[1].socket, buffer, 2048, 0);
    cout << "complete\n";

    close(socketDescrip);
    return 0;
}