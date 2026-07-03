#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "../../gmp/include/gmpxx.h"
#include "../gmp/rdtsc.h"
#include "../hc-128/hc-128.c"
#include "sha256.h"
using namespace std;

void LoadParams(const char* file_name, ...) {
    FILE* fp;
    if ((fp = fopen(file_name, "rb")) == NULL) return;

    va_list args;
    va_start(args, file_name);

    mpz_ptr i;
    i = va_arg(args, mpz_ptr);

    while (i != 0) {
        mpz_init(i);
        gmp_fscanf(fp, "%Zi\n", i);
        i = va_arg(args, mpz_ptr);
    }

    va_end(args);
    fclose(fp);
}

void DFEncrypt(const mpz_t& p, const mpz_t& g, const mpz_t& range,
               mpz_t& secret_key, mpz_t& open_key) {
    unsigned int t = CC();

    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, rand() % RAND_MAX);

    mpz_inits(secret_key, open_key, 0);
    mpz_urandomm(secret_key, rs, range);
    if (mpz_cmp_ui(secret_key, 1) < 0) mpz_add_ui(secret_key, secret_key, 1);
    mpz_powm(open_key, g, secret_key, p);

    t = CC() - t;
    cout << "\n DFEcnrypt time = " << t << '\n';
}

void DFDecrypt(const mpz_t& p, const mpz_t& secret_key, const mpz_t& open_key,
               mpz_t& result) {
    unsigned int t = CC();

    mpz_init(result);
    mpz_powm(result, open_key, secret_key, p);

    t = CC() - t;
    cout << "\n DFDecrypt time = " << t << '\n';
}

void MQVEncrypt(const mpz_t& p, const mpz_t& q, const mpz_t& g,
                mpz_t& secret_key, mpz_t& open_key) {
    unsigned int t = CC();

    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, rand() % RAND_MAX);

    mpz_inits(secret_key, open_key, 0);
    mpz_urandomm(secret_key, rs, q);
    if (mpz_cmp_ui(secret_key, 1) < 0) mpz_add_ui(secret_key, secret_key, 1);
    mpz_powm(open_key, g, secret_key, p);

    t = CC() - t;
    cout << "\n MQVEncrypt time = " << t << '\n';
}

void MQVDecrypt(const mpz_t& p, const mpz_t& q, const mpz_t& secret_key,
                const mpz_t& open_key, const mpz_t& open_key_other,
                const mpz_t& longterm_secret_key,
                const mpz_t& longterm_open_key, mpz_t& result) {
    unsigned int tt = CC();

    mpz_t d, e;
    mpz_inits(d, e, 0);

    mpz_fdiv_r_2exp(d, open_key, 128);
    mpz_setbit(d, 128);

    mpz_fdiv_r_2exp(e, open_key_other, 128);
    mpz_setbit(e, 128);

    mpz_t t;
    mpz_inits(result, t, 0);

    mpz_mul(t, d, longterm_secret_key);
    mpz_add(t, secret_key, t);
    mpz_mod(t, t, q);

    mpz_powm(result, longterm_open_key, e, p);
    mpz_mul(result, open_key_other, result);
    mpz_powm(result, result, t, p);

    tt = CC() - tt;
    cout << "\n MQVDecrypt time = " << tt << '\n';
}

int main() {
    int socketClient, tempLength = 0, port = 0;
    struct sockaddr_in servAddr, clientAddr;
    struct hostent *hp, *gethostbyname();

    if ((socketClient = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stdout, "Socket error - %s. [%d]\n", strerror(errno), errno);
        return 4;
    }
    ifstream portf("port.txt");
    portf >> port;
    portf.close();

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servAddr.sin_port = htons(port);
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = 0;
    if (bind(socketClient, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) {
        fprintf(stdout, "Bind error - %s. [%d]\n", strerror(errno), errno);
        return 4;
    }
    char buffer[1024];
    int length = sizeof(servAddr), i = 0;
    if (connect(socketClient, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
        perror("connect");
        return 4;
    }
    cout << "Connect to server, try connect to client...";

    mpz_t p, q, g;
    LoadParams("mulpipliq.txt", p, q, g, 0);
    mpz_t secret, open, open_Y, Z;
    mpz_inits(Z, open_Y, 0);
    DFEncrypt(p, g, p, secret, open);

    char *tmp = mpz_get_str(NULL, 0, open), ttmp[2048];
    string str = tmp;
    str += '\0';
    send(socketClient, str.c_str(), str.length(), 0);

    recv(socketClient, ttmp, 2048, 0);
    str = ttmp;
    mpz_set_str(open_Y, str.c_str(), 0);

    DFDecrypt(p, secret, open_Y, Z);

    tmp = mpz_get_str(NULL, 0, Z);
    str = tmp;
    SHA256 sha256, sha256_2;
    str = sha256(str);
    str += '\0';
    send(socketClient, str.c_str(), str.length(), 0);
    recv(socketClient, ttmp, 2048, 0);
    if (strcmp(ttmp, "ErrorConnect") == 0) {
        cout << "Error check H(Z) key, disconnect\n";
        close(socketClient);
        return 4;
    }
    cout << "complete\n";

    recv(socketClient, ttmp, 2048, 0);
    char buff[256];
    ECRYPT_ctx ctx;
    u8 K[16], IV[16], in[256], out[256];
    memset(K, 0, 16);
    memset(IV, 0, 16);
    memset(in, 0, 256);
    memset(out, 0, 256);
    ECRYPT_init();
    ECRYPT_keysetup(&ctx, K, 128, 128);
    ECRYPT_ivsetup(&ctx, IV);

    if (strcmp(ttmp, "1Send") == 0) {
        cout << "Send file...";
        ifstream filein("orig.bmp");
        while (filein.read(buff, 256)) {
            sha256_2.add(buff, 256);
            for (int i = 0; i < 256; ++i) in[i] = (u8)buff[i];
            ECRYPT_encrypt_bytes(&ctx, in, out, 256);
            for (int i = 0; i < 256; ++i) buff[i] = (char)out[i];
            send(socketClient, buff, 256, 0);
            recv(socketClient, ttmp, 2048, 0);
        }
        filein.close();

        str = "EndSend";
        str += '\0';
        send(socketClient, str.c_str(), str.length(), 0);
        cout << "complete\nSend hash...";

        recv(socketClient, buffer, 2048, 0);
        str = sha256_2.getHash();
        str += '\0';
        cout << "compelte\nFile hash: " << str << '\n';
        send(socketClient, str.c_str(), str.length(), 0);
    } else if (strcmp(ttmp, "2Recv") == 0) {
        ofstream fileout("recv.bmp");
        string str2 = "ok";
        cout << "Get file...";
        while (1) {
            memset(buff, 0, 256);
            recv(socketClient, buff, 256, 0);
            if (strcmp(buff, "EndSend") == 0) {
                send(socketClient, str2.c_str(), str2.length(), 0);
                break;
            }
            for (int i = 0; i < 256; ++i) in[i] = (u8)buff[i];
            ECRYPT_encrypt_bytes(&ctx, in, out, 256);
            for (int i = 0; i < 256; ++i) buff[i] = (char)out[i];
            fileout.write(buff, 256);
            sha256_2.add(buff, 256);
            send(socketClient, str2.c_str(), str2.length(), 0);
        }
        fileout.close();
        cout << "complete\nGet hash...";

        str = sha256_2.getHash();
        memset(ttmp, 0, 2048);
        recv(socketClient, ttmp, 2048, 0);
        send(socketClient, buff, 256, 0);
        cout << "complete\n";
        str2 = ttmp;

        cout << "My H:\n" << str << '\n';
        cout << "Server H:\n" << str2 << '\n';
        cout << "Check H: " << (str == str2) << '\n';
    }

    close(socketClient);
    return 0;
}