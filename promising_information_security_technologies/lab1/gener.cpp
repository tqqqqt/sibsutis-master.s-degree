#include <stdlib.h>
#include <time.h>

#include <iostream>

#include "../../gmp/include/gmpxx.h"
using namespace std;

int main() {
    // multipliq
    mpz_t p, q, g;
    mpz_inits(p, q, g, 0);

    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(0));

    do {
        mpz_urandomb(q, rs, 1023);
        mpz_nextprime(q, q);
        mpz_mul_ui(p, q, 2);
        mpz_add_ui(p, p, 1);
    } while (mpz_probab_prime_p(p, 15) == 0);

    mpz_t r;
    mpz_init(r);
    do {
        mpz_urandomm(g, rs, p);
        mpz_powm(r, g, q, p);
    } while (mpz_cmp_ui(r, 1) <= 0);

    FILE *file1 = fopen("mulpipliq.txt", "w");
    gmp_fprintf(file1, "%#0Zx\n", p);
    gmp_fprintf(file1, "%#0Zx\n", q);
    gmp_fprintf(file1, "%#0Zx\n", g);
    fclose(file1);

    // zikl-podg
    mpz_t t;
    mpz_inits(p, q, g, t, 0);

    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(0));

    mpz_urandomb(q, rs, 256);
    mpz_nextprime(q, q);

    do {
        mpz_urandomb(t, rs, 768);
        mpz_mul(p, t, q);
        mpz_add_ui(p, p, 1);
    } while (mpz_probab_prime_p(p, 15) == 0);

    mpz_init(r);
    do {
        mpz_urandomm(r, rs, p);
        if (mpz_cmp_ui(r, 2) < 0) mpz_add_ui(r, r, 2);
        mpz_powm(g, r, t, p);
    } while (mpz_cmp_ui(g, 1) <= 0);

    FILE *file2 = fopen("zikl-podg.txt", "w");
    gmp_fprintf(file2, "%#0Zx\n", p);
    gmp_fprintf(file2, "%#0Zx\n", q);
    gmp_fprintf(file2, "%#0Zx\n", g);
    fclose(file2);

    return 0;
}