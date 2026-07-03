#include <memory.h>
#include <stdio.h>

#include <fstream>
#include <iostream>

#include "../gmp/rdtsc.h"
#include "../hc-128/hc-128.c"
using namespace std;

ECRYPT_ctx ctx;

int main(int argc, char** argv) {
    u8 K[16], IV[16], in[100], out[100];
    memset(K, 0, 16);
    memset(IV, 0, 16);
    memset(in, 0, 100);

    string str;
    for (int i = 0; i < 100; ++i) str += ('A' + (i % 26));
    cout << "Str:\n" << str << "\n \n";
    for (int i = 0; i < 100; ++i) in[i] = (u8)str[i];

    if (argc > 1)
        for (int i = 0; argv[1][i]; ++i) K[i] = argv[1][i];
    if (argc > 2)
        for (int i = 0; argv[2][i]; ++i) IV[i] = argv[2][i];

    // --------------------------------------- encrypt
    unsigned long long t = rdtsc();
    ECRYPT_init();
    ECRYPT_keysetup(&ctx, K, 128, 128);
    ECRYPT_ivsetup(&ctx, IV);
    t = rdtsc() - t;
    cout << "Init time = " << t << '\n';

    t = rdtsc();
    ECRYPT_encrypt_bytes(&ctx, in, out, 100);
    t = rdtsc() - t;
    cout << "Encrypt time = " << t << '\n';

    ofstream fil1("encrypt.txt");
    for (int i = 0; i < 100; ++i) fil1 << (int)out[i] << ' ';
    fil1.close();

    // ----------------------------------------- decrypt
    memset(in, 0, 100);
    memset(out, 0, 100);

    t = rdtsc();
    ECRYPT_init();
    ECRYPT_keysetup(&ctx, K, 128, 128);
    ECRYPT_ivsetup(&ctx, IV);
    t = rdtsc() - t;
    cout << "Init time = " << t << '\n';

    ifstream fil2("encrypt.txt");
    int tt;
    for (int i = 0; i < 100; ++i) {
        fil2 >> tt;
        out[i] = (u8)tt;
    }
    fil2.close();

    t = rdtsc();
    ECRYPT_encrypt_bytes(&ctx, out, in, 100);
    t = rdtsc() - t;
    cout << "Decrypt time = " << t << "\n \n";

    for (int i = 0; i < 100; ++i) str[i] = (char)in[i];
    cout << "Decrypt str:\n" << str << '\n';

    return 0;
}
