#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>

#include "../../gmp/include/gmpxx.h"
#include "../gmp/rdtsc.h"
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
    // --------------------------------------------------------
    cout << "\n----------DF multi group----------------\n";
    mpz_t p, q, g;
    LoadParams("mulpipliq.txt", p, q, g, 0);

    mpz_t secret_X, open_X, secret_Y, open_Y, Z_X, Z_Y;
    DFEncrypt(p, g, p, secret_X, open_X);
    DFEncrypt(p, g, p, secret_Y, open_Y);
    DFDecrypt(p, secret_X, open_Y, Z_X);
    DFDecrypt(p, secret_Y, open_X, Z_Y);

    gmp_printf("Open = %#0Zx\n\n", open_X);

    gmp_fprintf(stdout, "Z_X = %#0Zx\n", Z_X);
    gmp_fprintf(stdout, "Z_Y = %#0Zx\n", Z_Y);
    cout << "Z_X == Z_Y is " << (bool)(mpz_cmp(Z_X, Z_Y) == 0) << '\n';

    // --------------------------------------------------------
    cout << "\n \n-------------DF podgroup-----------------\n";
    LoadParams("zikl-podg.txt", p, q, g, 0);

    DFEncrypt(p, g, p, secret_X, open_X);
    DFEncrypt(p, g, p, secret_Y, open_Y);
    DFDecrypt(p, secret_X, open_Y, Z_X);
    DFDecrypt(p, secret_Y, open_X, Z_Y);

    gmp_fprintf(stdout, "Z_X = %#0Zx\n", Z_X);
    gmp_fprintf(stdout, "Z_Y = %#0Zx\n", Z_Y);
    cout << "Z_X == Z_Y is " << (bool)(mpz_cmp(Z_X, Z_Y) == 0) << '\n';

    // --------------------------------------------------------
    cout << "\n \n-----------------MQV---------------------\n";
    LoadParams("zikl-podg.txt", p, q, g, 0);
    mpz_t longterm_open_key_X, longterm_open_key_Y, longterm_secret_key_X,
        longterm_secret_key_Y;

    MQVEncrypt(p, q, g, longterm_secret_key_X, longterm_open_key_X);
    MQVEncrypt(p, q, g, longterm_secret_key_Y, longterm_open_key_Y);
    MQVEncrypt(p, q, g, secret_X, open_X);
    MQVEncrypt(p, q, g, secret_Y, open_Y);
    MQVDecrypt(p, q, secret_X, open_X, open_Y, longterm_secret_key_X,
               longterm_open_key_Y, Z_X);
    MQVDecrypt(p, q, secret_Y, open_Y, open_X, longterm_secret_key_Y,
               longterm_open_key_X, Z_Y);

    gmp_fprintf(stdout, "Z_X = %#0Zx\n", Z_X);
    gmp_fprintf(stdout, "Z_Y = %#0Zx\n", Z_Y);
    cout << "Z_X == Z_Y is " << (bool)(mpz_cmp(Z_X, Z_Y) == 0) << '\n';

    return 0;
}