#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>

#include "../../gmp/include/gmp.h"
#include "../gmp/rdtsc.h"
using namespace std;

mpz_t p, a, b, q, Gx, Gy;

void init_curve_p256() {
    mpz_init_set_str(p,
                     "115792089210356248762697446949407573530086143415290314195"
                     "533631308867097853951",
                     10);

    mpz_init(a);
    mpz_sub_ui(a, p, 3);

    mpz_init_set_str(
        b, "5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
        16);

    mpz_init_set_str(q,
                     "115792089210356248762697446949407573529996955224135760342"
                     "422259061068512044369",
                     10);

    mpz_init_set_str(
        Gx, "6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
        16);
    mpz_init_set_str(
        Gy, "4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",
        16);
}

void clear_curve() {
    mpz_clear(p);
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(q);
    mpz_clear(Gx);
    mpz_clear(Gy);
}

// ----------------------------------------------------------------------
// Аффинные координаты
struct PointAffine {
    int inf;  // 1 – бесконечность
    mpz_t x, y;
};

void point_affine_init(PointAffine &P) {
    P.inf = 0;
    mpz_init(P.x);
    mpz_init(P.y);
}

void point_affine_clear(PointAffine &P) {
    mpz_clear(P.x);
    mpz_clear(P.y);
}

void point_affine_set_inf(PointAffine &P) {
    P.inf = 1;
    mpz_set_ui(P.x, 0);
    mpz_set_ui(P.y, 0);
}

void point_affine_set(PointAffine &P, const mpz_t x, const mpz_t y) {
    P.inf = 0;
    mpz_set(P.x, x);
    mpz_set(P.y, y);
}

void point_affine_copy(PointAffine &dest, const PointAffine &src) {
    dest.inf = src.inf;
    mpz_set(dest.x, src.x);
    mpz_set(dest.y, src.y);
}

int point_affine_cmp(const PointAffine &P, const PointAffine &Q) {
    if (P.inf && Q.inf) return 0;
    if (P.inf || Q.inf) return 1;
    return (mpz_cmp(P.x, Q.x) == 0 && mpz_cmp(P.y, Q.y) == 0) ? 0 : 1;
}

int mpz_invert_mod(mpz_t rop, const mpz_t op, const mpz_t mod) {
    if (mpz_sgn(op) == 0) return 0;  // обратного не существует
    return mpz_invert(rop, op, mod);
}

void point_affine_double(const PointAffine &P, PointAffine &R, const mpz_t p,
                         const mpz_t a) {
    if (P.inf) {
        point_affine_set_inf(R);
        return;
    }
    mpz_t lambda, num, den, tmp;
    mpz_inits(lambda, num, den, tmp, NULL);

    // числитель: 3*x^2 + a
    mpz_mul(num, P.x, P.x);
    mpz_mul_ui(num, num, 3);
    mpz_add(num, num, a);
    mpz_mod(num, num, p);

    // знаменатель: 2*y
    mpz_mul_ui(den, P.y, 2);
    mpz_mod(den, den, p);

    // Если знаменатель равен 0, то точка имеет порядок 2 – в нашей группе не
    // должно быть, но на всякий случай возвращаем бесконечность.
    if (mpz_sgn(den) == 0) {
        point_affine_set_inf(R);
        mpz_clears(lambda, num, den, tmp, NULL);
        return;
    }

    if (!mpz_invert_mod(den, den, p)) {
        // Инверсия не удалась (маловероятно для простого p)
        point_affine_set_inf(R);
        mpz_clears(lambda, num, den, tmp, NULL);
        return;
    }

    mpz_mul(lambda, num, den);
    mpz_mod(lambda, lambda, p);

    // x3 = λ^2 - 2*x
    mpz_mul(tmp, lambda, lambda);
    mpz_sub(tmp, tmp, P.x);
    mpz_sub(tmp, tmp, P.x);
    mpz_mod(R.x, tmp, p);

    // y3 = λ*(x - x3) - y
    mpz_sub(tmp, P.x, R.x);
    mpz_mul(tmp, lambda, tmp);
    mpz_sub(tmp, tmp, P.y);
    mpz_mod(R.y, tmp, p);
    R.inf = 0;

    mpz_clears(lambda, num, den, tmp, NULL);
}

// Сложение двух аффинных точек: R = P + Q
void point_affine_add(const PointAffine &P, const PointAffine &Q,
                      PointAffine &R, const mpz_t p, const mpz_t a) {
    if (P.inf) {
        point_affine_copy(R, Q);
        return;
    }
    if (Q.inf) {
        point_affine_copy(R, P);
        return;
    }

    // Проверка на равенство x-координат
    if (mpz_cmp(P.x, Q.x) == 0) {
        mpz_t neg_y;
        mpz_init(neg_y);
        mpz_sub(neg_y, p, Q.y);  // -Q.y mod p
        if (mpz_cmp(P.y, neg_y) == 0) {
            // Противоположные точки -> бесконечность
            point_affine_set_inf(R);
        } else {
            // Точки равны -> удвоение
            point_affine_double(P, R, p, a);
        }
        mpz_clear(neg_y);
        return;
    }

    // Общий случай
    mpz_t lambda, num, den, tmp;
    mpz_inits(lambda, num, den, tmp, NULL);

    mpz_sub(num, Q.y, P.y);
    mpz_mod(num, num, p);

    mpz_sub(den, Q.x, P.x);
    mpz_mod(den, den, p);

    if (!mpz_invert_mod(den, den, p)) {
        // Не должно происходить, так как x различны и знаменатель не 0
        point_affine_set_inf(R);
        mpz_clears(lambda, num, den, tmp, NULL);
        return;
    }

    mpz_mul(lambda, num, den);
    mpz_mod(lambda, lambda, p);

    mpz_mul(tmp, lambda, lambda);
    mpz_sub(tmp, tmp, P.x);
    mpz_sub(tmp, tmp, Q.x);
    mpz_mod(R.x, tmp, p);

    mpz_sub(tmp, P.x, R.x);
    mpz_mul(tmp, lambda, tmp);
    mpz_sub(tmp, tmp, P.y);
    mpz_mod(R.y, tmp, p);
    R.inf = 0;

    mpz_clears(lambda, num, den, tmp, NULL);
}

// Умножение точки на скаляр (бинарный метод) в аффинных координатах
void point_affine_mul(const PointAffine &P, const mpz_t k, PointAffine &R,
                      const mpz_t p, const mpz_t a) {
    PointAffine Q, tmp;
    point_affine_init(Q);
    point_affine_init(tmp);
    point_affine_copy(Q, P);
    point_affine_set_inf(R);

    mpz_t exp;
    mpz_init_set(exp, k);

    while (mpz_cmp_ui(exp, 0) > 0) {
        if (mpz_odd_p(exp)) {
            point_affine_add(R, Q, tmp, p, a);
            point_affine_copy(R, tmp);
        }
        point_affine_double(Q, tmp, p, a);
        point_affine_copy(Q, tmp);
        mpz_fdiv_q_2exp(exp, exp, 1);
    }

    point_affine_clear(Q);
    point_affine_clear(tmp);
    mpz_clear(exp);
}

// ----------------------------------------------------------------------
// Проективные координаты (стандартные: x = X/Z, y = Y/Z)
struct PointProj {
    int inf;
    mpz_t X, Y, Z;
};

void point_proj_init(PointProj &P) {
    P.inf = 0;
    mpz_init(P.X);
    mpz_init(P.Y);
    mpz_init(P.Z);
}

void point_proj_clear(PointProj &P) {
    mpz_clear(P.X);
    mpz_clear(P.Y);
    mpz_clear(P.Z);
}

void point_proj_set_inf(PointProj &P) {
    P.inf = 1;
    mpz_set_ui(P.X, 1);
    mpz_set_ui(P.Y, 1);
    mpz_set_ui(P.Z, 0);
}

void point_proj_set(PointProj &P, const mpz_t X, const mpz_t Y, const mpz_t Z) {
    P.inf = 0;
    mpz_set(P.X, X);
    mpz_set(P.Y, Y);
    mpz_set(P.Z, Z);
}

void point_proj_copy(PointProj &dest, const PointProj &src) {
    dest.inf = src.inf;
    mpz_set(dest.X, src.X);
    mpz_set(dest.Y, src.Y);
    mpz_set(dest.Z, src.Z);
}

// Преобразование аффинной в проективную (Z=1)
void point_affine_to_proj(const PointAffine &A, PointProj &P) {
    if (A.inf) {
        point_proj_set_inf(P);
    } else {
        P.inf = 0;
        mpz_set(P.X, A.x);
        mpz_set(P.Y, A.y);
        mpz_set_ui(P.Z, 1);
    }
}

// Преобразование проективной в аффинную (выполняется инверсия Z)
void point_proj_to_affine(const PointProj &P, PointAffine &A, const mpz_t p) {
    if (P.inf || mpz_cmp_ui(P.Z, 0) == 0) {
        point_affine_set_inf(A);
        return;
    }
    mpz_t invZ;
    mpz_init(invZ);
    mpz_invert(invZ, P.Z, p);
    mpz_mul(A.x, P.X, invZ);
    mpz_mod(A.x, A.x, p);
    mpz_mul(A.y, P.Y, invZ);
    mpz_mod(A.y, A.y, p);
    A.inf = 0;
    mpz_clear(invZ);
}

// Удвоение в проективных координатах
void point_proj_double(const PointProj &P, PointProj &R, const mpz_t p,
                       const mpz_t a) {
    if (P.inf || mpz_cmp_ui(P.Z, 0) == 0) {
        point_proj_set_inf(R);
        return;
    }
    mpz_t YY, YYYY, S, M, T, X3, Y3, Z3;
    mpz_inits(YY, YYYY, S, M, T, X3, Y3, Z3, NULL);

    // YY = Y^2
    mpz_mul(YY, P.Y, P.Y);
    mpz_mod(YY, YY, p);
    // YYYY = YY^2
    mpz_mul(YYYY, YY, YY);
    mpz_mod(YYYY, YYYY, p);
    // S = 4 * X * YY
    mpz_mul(S, P.X, YY);
    mpz_mul_ui(S, S, 4);
    mpz_mod(S, S, p);
    // M = 3*X^2 + a*Z^4
    mpz_mul(T, P.X, P.X);   // X^2
    mpz_mul_ui(M, T, 3);    // 3X^2
    mpz_pow_ui(T, P.Z, 4);  // Z^4
    mpz_mul(T, T, a);
    mpz_add(M, M, T);
    mpz_mod(M, M, p);
    // X3 = M^2 - 2*S
    mpz_mul(X3, M, M);
    mpz_sub(X3, X3, S);
    mpz_sub(X3, X3, S);
    mpz_mod(X3, X3, p);
    // Y3 = M*(S - X3) - 8*YYYY
    mpz_sub(T, S, X3);
    mpz_mul(T, M, T);
    mpz_mul_ui(Y3, YYYY, 8);
    mpz_sub(Y3, T, Y3);
    mpz_mod(Y3, Y3, p);
    // Z3 = 2*Y*Z
    mpz_mul(Z3, P.Y, P.Z);
    mpz_mul_ui(Z3, Z3, 2);
    mpz_mod(Z3, Z3, p);

    point_proj_set(R, X3, Y3, Z3);
    mpz_clears(YY, YYYY, S, M, T, X3, Y3, Z3, NULL);
}

// Сложение двух проективных точек (различных)
void point_proj_add(const PointProj &P, const PointProj &Q, PointProj &R,
                    const mpz_t p, const mpz_t a) {
    if (P.inf || mpz_cmp_ui(P.Z, 0) == 0) {
        point_proj_copy(R, Q);
        return;
    }
    if (Q.inf || mpz_cmp_ui(Q.Z, 0) == 0) {
        point_proj_copy(R, P);
        return;
    }

    mpz_t Z1Z1, Z2Z2, U1, U2, S1, S2, H, HH, HHH, Rz, V, X3, Y3, Z3;
    mpz_inits(Z1Z1, Z2Z2, U1, U2, S1, S2, H, HH, HHH, Rz, V, X3, Y3, Z3, NULL);

    // Z1Z1 = Z1^2
    mpz_mul(Z1Z1, P.Z, P.Z);
    mpz_mod(Z1Z1, Z1Z1, p);
    // Z2Z2 = Z2^2
    mpz_mul(Z2Z2, Q.Z, Q.Z);
    mpz_mod(Z2Z2, Z2Z2, p);
    // U1 = X1 * Z2Z2
    mpz_mul(U1, P.X, Z2Z2);
    mpz_mod(U1, U1, p);
    // U2 = X2 * Z1Z1
    mpz_mul(U2, Q.X, Z1Z1);
    mpz_mod(U2, U2, p);
    // S1 = Y1 * Z2^3 = Y1 * Z2Z2 * Z2
    mpz_mul(S1, P.Y, Z2Z2);
    mpz_mod(S1, S1, p);
    mpz_mul(S1, S1, Q.Z);
    mpz_mod(S1, S1, p);
    // S2 = Y2 * Z1^3 = Y2 * Z1Z1 * Z1
    mpz_mul(S2, Q.Y, Z1Z1);
    mpz_mod(S2, S2, p);
    mpz_mul(S2, S2, P.Z);
    mpz_mod(S2, S2, p);

    // H = U2 - U1
    mpz_sub(H, U2, U1);
    mpz_mod(H, H, p);
    // Rz = S2 - S1
    mpz_sub(Rz, S2, S1);
    mpz_mod(Rz, Rz, p);

    if (mpz_cmp_ui(H, 0) == 0) {
        if (mpz_cmp_ui(Rz, 0) == 0) {
            // P == Q, удвоение
            point_proj_double(P, R, p, a);
        } else {
            // противоположные точки
            point_proj_set_inf(R);
        }
        mpz_clears(Z1Z1, Z2Z2, U1, U2, S1, S2, H, HH, HHH, Rz, V, X3, Y3, Z3,
                   NULL);
        return;
    }

    // HH = H^2
    mpz_mul(HH, H, H);
    mpz_mod(HH, HH, p);
    // HHH = H^3
    mpz_mul(HHH, HH, H);
    mpz_mod(HHH, HHH, p);
    // V = U1 * HH
    mpz_mul(V, U1, HH);
    mpz_mod(V, V, p);

    // X3 = Rz^2 - HHH - 2*V
    mpz_mul(X3, Rz, Rz);
    mpz_mod(X3, X3, p);
    mpz_sub(X3, X3, HHH);
    mpz_mod(X3, X3, p);
    mpz_sub(X3, X3, V);
    mpz_sub(X3, X3, V);
    mpz_mod(X3, X3, p);
    // Y3 = Rz*(V - X3) - S1*HHH
    mpz_sub(Y3, V, X3);
    mpz_mul(Y3, Rz, Y3);
    mpz_mod(Y3, Y3, p);
    mpz_mul(Z3, S1, HHH);
    mpz_mod(Z3, Z3, p);
    mpz_sub(Y3, Y3, Z3);
    mpz_mod(Y3, Y3, p);
    // Z3 = H * Z1 * Z2
    mpz_mul(Z3, H, P.Z);
    mpz_mod(Z3, Z3, p);
    mpz_mul(Z3, Z3, Q.Z);
    mpz_mod(Z3, Z3, p);

    point_proj_set(R, X3, Y3, Z3);
    mpz_clears(Z1Z1, Z2Z2, U1, U2, S1, S2, H, HH, HHH, Rz, V, X3, Y3, Z3, NULL);
}

// Умножение на скаляр в проективных координатах
void point_proj_mul(const PointProj &P, const mpz_t k, PointProj &R,
                    const mpz_t p, const mpz_t a) {
    PointProj Q, tmp;
    point_proj_init(Q);
    point_proj_init(tmp);
    point_proj_copy(Q, P);
    point_proj_set_inf(R);

    mpz_t exp;
    mpz_init_set(exp, k);

    while (mpz_cmp_ui(exp, 0) > 0) {
        if (mpz_odd_p(exp)) {
            point_proj_add(R, Q, tmp, p, a);
            point_proj_copy(R, tmp);
        }
        point_proj_double(Q, tmp, p, a);
        point_proj_copy(Q, tmp);
        mpz_fdiv_q_2exp(exp, exp, 1);
    }

    point_proj_clear(Q);
    point_proj_clear(tmp);
    mpz_clear(exp);
}

// ----------------------------------------------------------------------
// Вспомогательная функция для MQV: преобразование x-координаты в число L (или
// M)
void mqv_transform(const mpz_t x, mpz_t L, const mpz_t q) {
    // Длина q в битах (округляем вверх до чётного, хотя для P-256 чётное)
    size_t bits = mpz_sizeinbase(q, 2);
    size_t f = (bits + 1) / 2;  // половинная длина с округлением вверх
    mpz_fdiv_r_2exp(L, x, f);
    mpz_setbit(L, f);
}

// ----------------------------------------------------------------------
// Протокол Диффи–Хеллмана (аффинный)
void dh_encrypt_affine(const mpz_t q, const PointAffine &G, mpz_t &secret,
                       PointAffine &pub) {
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(NULL));

    mpz_init(secret);
    mpz_urandomm(secret, rs, q);
    if (mpz_cmp_ui(secret, 0) == 0)
        mpz_set_ui(secret, 1);  // секретный ключ ∈ [1, q-1]

    point_affine_mul(G, secret, pub, p, a);
    gmp_randclear(rs);
}

void dh_decrypt_affine(const mpz_t secret, const PointAffine &pub_other,
                       PointAffine &shared, const mpz_t p, const mpz_t a) {
    point_affine_mul(pub_other, secret, shared, p, a);
}

// Протокол Диффи–Хеллмана (проективный)
void dh_encrypt_proj(const mpz_t q, const PointProj &G, mpz_t &secret,
                     PointProj &pub) {
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(NULL));

    mpz_init(secret);
    mpz_urandomm(secret, rs, q);
    if (mpz_cmp_ui(secret, 0) == 0) mpz_set_ui(secret, 1);

    point_proj_mul(G, secret, pub, p, a);
    gmp_randclear(rs);
}

void dh_decrypt_proj(const mpz_t secret, const PointProj &pub_other,
                     PointProj &shared, const mpz_t p, const mpz_t a) {
    point_proj_mul(pub_other, secret, shared, p, a);
}

// ----------------------------------------------------------------------
// Протокол MQV (аффинный)
void mqv_encrypt_affine(const mpz_t q, const PointAffine &G, mpz_t &secret,
                        PointAffine &pub) {
    // генерирует либо долговременную, либо эфемерную пару
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(NULL));

    mpz_init(secret);
    mpz_urandomm(secret, rs, q);
    if (mpz_cmp_ui(secret, 0) == 0) mpz_set_ui(secret, 1);

    point_affine_mul(G, secret, pub, p, a);
    gmp_randclear(rs);
}

void mqv_decrypt_affine(
    const mpz_t q,
    const mpz_t secret_ephemeral,  // e (свой эфемерный секрет)
    const PointAffine &pub_ephemeral,  // R (свой эфемерный открытый)
    const PointAffine &pub_ephemeral_other,  // S (чужой эфемерный открытый)
    const mpz_t longterm_secret,  // d (свой долговременный секрет)
    const PointAffine
        &longterm_pub_other,  // Q_B (чужой долговременный открытый)
    PointAffine &shared) {
    unsigned int tt = CC();

    mpz_t L, M, t;
    mpz_inits(L, M, t, NULL);

    // Вычисляем преобразованные значения
    mqv_transform(pub_ephemeral.x, L, q);
    mqv_transform(pub_ephemeral_other.x, M, q);

    // t = (L * d + e) mod q
    mpz_mul(t, L, longterm_secret);
    mpz_add(t, t, secret_ephemeral);
    mpz_mod(t, t, q);

    // Вычисляем T = S + M * Q_B
    PointAffine MQ, T;
    point_affine_init(MQ);
    point_affine_init(T);

    // Приводим M по модулю q для эффективности (но это не обязательно для
    // корректности)
    mpz_t M_mod;
    mpz_init(M_mod);
    mpz_mod(M_mod, M, q);
    point_affine_mul(longterm_pub_other, M_mod, MQ, p, a);
    mpz_clear(M_mod);

    point_affine_add(pub_ephemeral_other, MQ, T, p, a);

    // Общий секрет K = t * T
    point_affine_mul(T, t, shared, p, a);

    point_affine_clear(MQ);
    point_affine_clear(T);
    mpz_clears(L, M, t, NULL);

    tt = CC() - tt;
    cout << "\n MQVDecrypt (affine) time = " << tt << '\n';
}

// Аналогично для проективных координат
void mqv_encrypt_proj(const mpz_t q, const PointProj &G, mpz_t &secret,
                      PointProj &pub) {
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(NULL));

    mpz_init(secret);
    mpz_urandomm(secret, rs, q);
    if (mpz_cmp_ui(secret, 0) == 0) mpz_set_ui(secret, 1);

    point_proj_mul(G, secret, pub, p, a);
    gmp_randclear(rs);
}

void mqv_decrypt_proj(const mpz_t q, const mpz_t secret_ephemeral,
                      const PointProj &pub_ephemeral,
                      const PointProj &pub_ephemeral_other,
                      const mpz_t longterm_secret,
                      const PointProj &longterm_pub_other, PointProj &shared) {
    unsigned int tt = CC();

    // Для преобразования нужна x-координата в аффинном виде, поэтому временно
    // переведём
    PointAffine tmp_aff;
    point_affine_init(tmp_aff);

    mpz_t L, M, t;
    mpz_inits(L, M, t, NULL);

    point_proj_to_affine(pub_ephemeral, tmp_aff, p);
    mqv_transform(tmp_aff.x, L, q);
    point_proj_to_affine(pub_ephemeral_other, tmp_aff, p);
    mqv_transform(tmp_aff.x, M, q);

    mpz_mul(t, L, longterm_secret);
    mpz_add(t, t, secret_ephemeral);
    mpz_mod(t, t, q);

    // T = S + M * Q_B (в проективных)
    PointProj MQ, T;
    point_proj_init(MQ);
    point_proj_init(T);
    point_proj_mul(longterm_pub_other, M, MQ, p, a);
    point_proj_add(pub_ephemeral_other, MQ, T, p, a);

    // shared = t * T
    point_proj_mul(T, t, shared, p, a);

    point_affine_clear(tmp_aff);
    point_proj_clear(MQ);
    point_proj_clear(T);
    mpz_clears(L, M, t, NULL);

    tt = CC() - tt;
    cout << "\n MQVDecrypt (projective) time = " << tt << '\n';
}

// ----------------------------------------------------------------------
int main() {
    init_curve_p256();
    gmp_randstate_t rs;
    gmp_randinit_default(rs);
    gmp_randseed_ui(rs, time(NULL));

    cout << "\n---------- Диффи–Хеллман на эллиптической кривой (аффинные) "
            "----------\n";
    PointAffine G_aff, pubA_aff, pubB_aff, sharedA_aff, sharedB_aff;
    point_affine_init(G_aff);
    point_affine_set(G_aff, Gx, Gy);
    point_affine_init(pubA_aff);
    point_affine_init(pubB_aff);
    point_affine_init(sharedA_aff);
    point_affine_init(sharedB_aff);

    mpz_t secA, secB;
    dh_encrypt_affine(q, G_aff, secA, pubA_aff);
    dh_encrypt_affine(q, G_aff, secB, pubB_aff);
    dh_decrypt_affine(secA, pubB_aff, sharedA_aff, p, a);
    dh_decrypt_affine(secB, pubA_aff, sharedB_aff, p, a);

    cout << "Общий секрет (A): ";
    gmp_printf("(%Zd, %Zd)\n", sharedA_aff.x, sharedA_aff.y);
    cout << "Общий секрет (B): ";
    gmp_printf("(%Zd, %Zd)\n", sharedB_aff.x, sharedB_aff.y);
    cout << "Совпадают: " << (point_affine_cmp(sharedA_aff, sharedB_aff) == 0)
         << endl;

    // Очистка
    mpz_clear(secA);
    mpz_clear(secB);
    point_affine_clear(G_aff);
    point_affine_clear(pubA_aff);
    point_affine_clear(pubB_aff);
    point_affine_clear(sharedA_aff);
    point_affine_clear(sharedB_aff);

    cout << "\n---------- Диффи–Хеллман на эллиптической кривой (проективные) "
            "----------\n";
    PointProj G_proj, pubA_proj, pubB_proj, sharedA_proj, sharedB_proj;
    point_proj_init(G_proj);
    point_affine_to_proj(G_aff, G_proj);
    point_proj_init(pubA_proj);
    point_proj_init(pubB_proj);
    point_proj_init(sharedA_proj);
    point_proj_init(sharedB_proj);

    dh_encrypt_proj(q, G_proj, secA, pubA_proj);
    dh_encrypt_proj(q, G_proj, secB, pubB_proj);
    dh_decrypt_proj(secA, pubB_proj, sharedA_proj, p, a);
    dh_decrypt_proj(secB, pubA_proj, sharedB_proj, p, a);

    PointAffine tmp_aff;
    point_affine_init(tmp_aff);
    point_proj_to_affine(sharedA_proj, tmp_aff, p);
    cout << "Общий секрет (A): ";
    gmp_printf("(%Zd, %Zd)\n", tmp_aff.x, tmp_aff.y);
    point_proj_to_affine(sharedB_proj, tmp_aff, p);
    cout << "Общий секрет (B): ";
    gmp_printf("(%Zd, %Zd)\n", tmp_aff.x, tmp_aff.y);
    // Сравнение проективных точек не прямое, но можно сравнить аффинные
    point_proj_to_affine(sharedA_proj, tmp_aff, p);
    PointAffine tmp_aff2;
    point_affine_init(tmp_aff2);
    point_proj_to_affine(sharedB_proj, tmp_aff2, p);
    cout << "Совпадают: " << (point_affine_cmp(tmp_aff, tmp_aff2) == 0) << endl;
    point_affine_clear(tmp_aff);
    point_affine_clear(tmp_aff2);

    mpz_clear(secA);
    mpz_clear(secB);
    point_proj_clear(G_proj);
    point_proj_clear(pubA_proj);
    point_proj_clear(pubB_proj);
    point_proj_clear(sharedA_proj);
    point_proj_clear(sharedB_proj);

    // ------------------------------------------------------------------
    cout << "\n---------- MQV на эллиптической кривой (аффинные) ----------\n";
    // Долговременные ключи A и B
    mpz_t longterm_secA, longterm_secB;
    PointAffine longterm_pubA_aff, longterm_pubB_aff;
    point_affine_init(longterm_pubA_aff);
    point_affine_init(longterm_pubB_aff);
    mqv_encrypt_affine(q, G_aff, longterm_secA, longterm_pubA_aff);
    mqv_encrypt_affine(q, G_aff, longterm_secB, longterm_pubB_aff);

    // Эфемерные ключи A и B
    mpz_t eph_secA, eph_secB;
    PointAffine eph_pubA_aff, eph_pubB_aff;
    point_affine_init(eph_pubA_aff);
    point_affine_init(eph_pubB_aff);
    mqv_encrypt_affine(q, G_aff, eph_secA, eph_pubA_aff);
    mqv_encrypt_affine(q, G_aff, eph_secB, eph_pubB_aff);

    // Вычисление общих секретов
    PointAffine sharedA_mqv_aff, sharedB_mqv_aff;
    point_affine_init(sharedA_mqv_aff);
    point_affine_init(sharedB_mqv_aff);

    mqv_decrypt_affine(q, eph_secA, eph_pubA_aff, eph_pubB_aff, longterm_secA,
                       longterm_pubB_aff, sharedA_mqv_aff);
    mqv_decrypt_affine(q, eph_secB, eph_pubB_aff, eph_pubA_aff, longterm_secB,
                       longterm_pubA_aff, sharedB_mqv_aff);

    cout << "Общий секрет MQV (A): ";
    gmp_printf("(%Zd, %Zd)\n", sharedA_mqv_aff.x, sharedA_mqv_aff.y);
    cout << "Общий секрет MQV (B): ";
    gmp_printf("(%Zd, %Zd)\n", sharedB_mqv_aff.x, sharedB_mqv_aff.y);
    cout << "Совпадают: "
         << (point_affine_cmp(sharedA_mqv_aff, sharedB_mqv_aff) == 0) << endl;

    // Очистка аффинных MQV
    mpz_clear(longterm_secA);
    mpz_clear(longterm_secB);
    mpz_clear(eph_secA);
    mpz_clear(eph_secB);
    point_affine_clear(longterm_pubA_aff);
    point_affine_clear(longterm_pubB_aff);
    point_affine_clear(eph_pubA_aff);
    point_affine_clear(eph_pubB_aff);
    point_affine_clear(sharedA_mqv_aff);
    point_affine_clear(sharedB_mqv_aff);

    cout << "\n---------- MQV на эллиптической кривой (проективные) "
            "----------\n";
    // Долговременные ключи в проективных
    PointProj G_proj2, longterm_pubA_proj, longterm_pubB_proj;
    point_proj_init(G_proj2);
    point_affine_to_proj(G_aff, G_proj2);
    point_proj_init(longterm_pubA_proj);
    point_proj_init(longterm_pubB_proj);
    mqv_encrypt_proj(q, G_proj2, longterm_secA, longterm_pubA_proj);
    mqv_encrypt_proj(q, G_proj2, longterm_secB, longterm_pubB_proj);

    // Эфемерные ключи
    PointProj eph_pubA_proj, eph_pubB_proj;
    point_proj_init(eph_pubA_proj);
    point_proj_init(eph_pubB_proj);
    mqv_encrypt_proj(q, G_proj2, eph_secA, eph_pubA_proj);
    mqv_encrypt_proj(q, G_proj2, eph_secB, eph_pubB_proj);

    PointProj sharedA_mqv_proj, sharedB_mqv_proj;
    point_proj_init(sharedA_mqv_proj);
    point_proj_init(sharedB_mqv_proj);

    mqv_decrypt_proj(q, eph_secA, eph_pubA_proj, eph_pubB_proj, longterm_secA,
                     longterm_pubB_proj, sharedA_mqv_proj);
    mqv_decrypt_proj(q, eph_secB, eph_pubB_proj, eph_pubA_proj, longterm_secB,
                     longterm_pubA_proj, sharedB_mqv_proj);

    point_affine_init(tmp_aff);
    point_affine_init(tmp_aff2);
    point_proj_to_affine(sharedA_mqv_proj, tmp_aff, p);
    cout << "Общий секрет MQV (A): ";
    gmp_printf("(%Zd, %Zd)\n", tmp_aff.x, tmp_aff.y);
    point_proj_to_affine(sharedB_mqv_proj, tmp_aff2, p);
    cout << "Общий секрет MQV (B): ";
    gmp_printf("(%Zd, %Zd)\n", tmp_aff2.x, tmp_aff2.y);
    cout << "Совпадают: " << (point_affine_cmp(tmp_aff, tmp_aff2) == 0) << endl;

    // Очистка
    mpz_clear(longterm_secA);
    mpz_clear(longterm_secB);
    mpz_clear(eph_secA);
    mpz_clear(eph_secB);
    point_proj_clear(G_proj2);
    point_proj_clear(longterm_pubA_proj);
    point_proj_clear(longterm_pubB_proj);
    point_proj_clear(eph_pubA_proj);
    point_proj_clear(eph_pubB_proj);
    point_proj_clear(sharedA_mqv_proj);
    point_proj_clear(sharedB_mqv_proj);
    point_affine_clear(tmp_aff);
    point_affine_clear(tmp_aff2);

    clear_curve();
    gmp_randclear(rs);
    return 0;
}