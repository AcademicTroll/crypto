#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <time.h>
#include <pthread.h>
#include <bsd/stdlib.h>

#define foo4random() (arc4random() % ((unsigned)RAND_MAX + 1))
#define COMPOSITE        0
#define PROBABLE_PRIME   1

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
        result=PROBABLE_PRIME; goto exit;
    }
    for(i=0; i < s-1; i++) {
        if (mpz_cmp(a_to_power, n_minus_one) == 0) {
            result=PROBABLE_PRIME; goto exit;
        }
        mpz_powm_ui (a_to_power, a_to_power, 2, n);
    }
    if (mpz_cmp(a_to_power, n_minus_one) == 0) {
        result=PROBABLE_PRIME; goto exit;
    }
    result = COMPOSITE;

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
        if (miller_rabin_pass(a, n) == COMPOSITE) {
            return COMPOSITE;
        }
    }
    return PROBABLE_PRIME;
}

void *GenPrime(void *threadid)
{
  long tid;
  tid = (long)threadid;

  mpz_t n, max, two, p;
  gmp_randstate_t rand_state;
  gmp_randinit_default(rand_state);
  gmp_randseed_ui(rand_state, arc4random());

  mpz_init(max);
  mpz_init_set_ui(two, 2);

  mpz_mul_2exp(max, two, prime_bits);

  mpz_init(p);
  do{
    mpz_urandomm(p, rand_state, max);
    if(mpz_even_p(p)) ;
    if(mpz_fdiv_ui(p, 3) == 0) ;
    if(mpz_fdiv_ui(p, 5) == 0) ;
    if(mpz_fdiv_ui(p, 7) == 0) ;
  }while(miller_rabin(p, rand_state) == COMPOSITE);

  printf("\nHello World! It's me, thread #%ld!\n", tid);
  mpz_out_str(stdout, 10, p);


  /* GO */

  pthread_exit(NULL);

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
    /*if (strcmp(argv[1], "test") == 0) {
        mpz_init_set_str(n, argv[2], 10);
        puts(miller_rabin(n, rand_state) == PROBABLE_PRIME ? "PRIME" : "COMPOSITE");
     } */

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

    pthread_t threads[k];
    int rc;
    long t;
  for(t=0; t<k; t++){
    printf("In main: creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, GenPrime, (void *)t);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

    /* Last thing that main() should do */
  pthread_exit(NULL);

  return 0;
}
