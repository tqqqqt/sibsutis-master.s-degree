#include <memory.h>
#include <stdio.h>

#include <iostream>

#include "../gmp/rdtsc.h"
#include "../salsa/salsa20.c"
using namespace std;

ECRYPT_ctx ctx;

int main(int argc, char* argv[]) {
    u8 K[16], IV[16], in[100], out[100];
    memset(K, 0, 16);
    memset(IV, 0, 16);
    memset(in, 0, 100);

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

    // ----------------------------------------- decrypt
    t = rdtsc();
    ECRYPT_init();
    ECRYPT_keysetup(&ctx, K, 128, 128);
    ECRYPT_ivsetup(&ctx, IV);
    t = rdtsc() - t;
    cout << "Init time = " << t << '\n';

    t = rdtsc();
    ECRYPT_encrypt_bytes(&ctx, out, in, 100);
    t = rdtsc() - t;
    cout << "Decrypt time = " << t << '\n';

    return 0;
}