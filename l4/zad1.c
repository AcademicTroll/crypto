#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <time.h>
#include <pthread.h>
#include <bsd/stdlib.h>

#define foo4random() (arc4random() % ((unsigned)RAND_MAX + 1))
#define COMP 0
#define PRIME 1

int prime_bits;

int miller_rabin_pass(mpz_t a, mpz_t n) {
    int i, s, result;
    mpz_t a_to_power, d, n_minus_one;

    mpz_init(n_minus_one);
    mpz_sub_ui(n_minus_one, n, 1);

    s = 0;
    mpz_init_set(d, n_minus_one);
    while (mpz_even_p(d)) {
      mpz_fdiv_q_2exp(d, d, 1);
      s++;
    }

    mpz_init(a_to_power);
    mpz_powm(a_to_power, a, d, n);
    if (mpz_cmp_ui(a_to_power, 1) == 0)  {
        result=PRIME; goto exit;
    }
    for(i=0; i < s-1; i++) {
        if (mpz_cmp(a_to_power, n_minus_one) == 0) {
            result=PRIME; goto exit;
        }
        mpz_powm_ui (a_to_power, a_to_power, 2, n);
    }
    if (mpz_cmp(a_to_power, n_minus_one) == 0) {
        result=PRIME; goto exit;
    }
    result = COMP;

exit:
    mpz_clear(a_to_power);
    mpz_clear(d);
    mpz_clear(n_minus_one);
    return result;
}

int miller_rabin(mpz_t n, gmp_randstate_t rand_state) {
    mpz_t a;
    int repeat;
    mpz_init(a);
    for(repeat=0; repeat<20; repeat++) {
        do {
            mpz_urandomm(a, rand_state, n);
        } while (mpz_sgn(a) == 0);
        if (miller_rabin_pass(a, n) == COMP) {
            return COMP;
        }
    }
    return PRIME;
}


int main(int argc, char* argv[]) {

    int k, d;

    k = atoi(argv[1]);
    if(k > 0 && k < 9) {
      /* test */
    }
    else {
      printf("Wrong input, k = {1...8}\n");
      return 1;
    }

    d = atoi(argv[2]);
    switch (d) {
      case 256: break;
      case 512: break;
      case 1024: break;
      case 2048: break;
      case 3072: break;
      case 7680: break;
      default:
        printf("Wrong input, d = {256, 512, 1024, 2048, 3072, 7680}\n");
        return 1;
    }
    prime_bits = atoi(argv[2])+1;
    /* mpz_mul_2exp(max, two, atoi(argv[2])+1); */

    mpz_t n, max, one, two, p, q;
    mpz_t p_minus, q_minus, helper, enka;
    gmp_randstate_t rand_state;
    gmp_randinit_default(rand_state);
    gmp_randseed_ui(rand_state, arc4random());

    mpz_init(max);
    mpz_init_set_ui(one, 1);
    mpz_init_set_ui(two, 2);

    mpz_mul_2exp(max, two, prime_bits);

    mpz_init(p);
    do{
      mpz_urandomm(p, rand_state, max);
      if(mpz_even_p(p)) continue;
      if(mpz_fdiv_ui(p, 3) == 0) continue;
      if(mpz_fdiv_ui(p, 5) == 0) continue;
      if(mpz_fdiv_ui(p, 7) == 0) continue;
    }while(miller_rabin(p, rand_state) == COMP);

    /* gmp_randseed_ui(rand_state, arc4random()); */
    mpz_init(q);
    do{
       mpz_urandomm(q, rand_state, max);
       if(mpz_even_p(q)) continue;
       if(mpz_fdiv_ui(q, 3) == 0) continue;
       if(mpz_fdiv_ui(q, 5) == 0) continue;
       if(mpz_fdiv_ui(q, 7) == 0) continue;
     }while(miller_rabin(q, rand_state) == COMP);

    /* mpz_out_str(stdout, 10, p);
    printf("\n");
    mpz_out_str(stdout, 10, q); */

/******************************* STAGE 2 ************************************/

    /* n = p * q */
    mpz_init(enka);
    mpz_mul(enka, p, q);
    /* printf("\n");
    mpz_out_str(stdout, 10, enka); */

    /* helper = (p-1) * (q-1) */
    mpz_init(p_minus);
    mpz_init(q_minus);
    mpz_init(helper);
    mpz_sub(p_minus, p, one);
    mpz_sub(q_minus, q, one);
    mpz_mul(helper, p_minus, q_minus);


/******************************* STAGE 3 ***********************************/

    /* random e: 1 < e < helper, gcd(e, helper) = 1 */
    /* mpz_t e;
    mpz_t gcd;
    mpz_init(e);
    mpz_init(gcd);
    do {
      gmp_randseed_ui(rand_state, arc4random());
      mpz_urandomm(e, rand_state, helper);
      mpz_gcd(gcd, e, helper);
    } while(mpz_cmp_ui(gcd, 1) == 0);

    printf("\nOk! e: \n");
    mpz_out_str(stdout, 10, e); */

/******************************* STAGE 4 ***********************************/

    /* Extended Euclides Algorith */
    /* d: 1 < d < helper, ed = 1 (mod helper) */
    mpz_t gcd, e;
    mpz_t eea_x, eea_y;
    mpz_init(eea_x);
    mpz_init(eea_y);
    mpz_init(e);

    /* nie dziala na petli do - while ??? */
    while(1) {
      /*gmp_randseed_ui(rand_state, arc4random()); */
      mpz_urandomm(e, rand_state, helper);
      mpz_gcdext(gcd, eea_x, eea_y, helper, e);
      if(mpz_cmp_ui(gcd, 1) == 0) {
        break;
      }
      mpz_clears(e, gcd, eea_x, eea_y);
    }

/******************************* GOT ALL ***********************************/

    /* PUBLIC KEY: (enka,e)
     * PRIVATE KEY: eea_y */

    int choice;
    printf("\nEncrypt - 1, Decrypt - 2: ");
    scanf("%i", &choice);

    /* TEST VALUES */
    mpz_t msg;
    mpz_init_set_ui(msg, 12345678);
    /* GOT THIS */

    mpz_t enc_c, dec_m;
    mpz_init(enc_c);
    mpz_init(dec_m);

    switch(choice) {
      case 1:
        /* ENCRYPT */
        mpz_powm(enc_c, msg, e, enka);
        printf("\nEncrypted msg: ");
        mpz_out_str(stdout, 10, enc_c);
        break;

      case 2:
        /* DECRYPT */
        mpz_powm(enc_c, msg, e, enka);
        printf("\nEncrypted msg: ");
        mpz_out_str(stdout, 10, enc_c);
        printf("\nDecrypted msg: ");
        mpz_powm(dec_m, enc_c, eea_y, enka);
        mpz_out_str(stdout, 10, dec_m);
        break;

      default:
        printf("\nWrong choice!\n");
        return 1;
    }


    return 0;
}
