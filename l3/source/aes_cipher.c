/* func: aes_encrypt() : aes_decrypt : aes_init() */

#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <error.h>
#include "aes_cipher.h"
#include "keystore.h"

#define SIZE 1024


int aes_init(unsigned char* pwd, unsigned int pwd_len, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx, int flag) {
  int i, rounds = 5;
  unsigned char key[32], iv[32], salt[8], data[32];
  int data_len = 32;

  switch(flag) {
    case 1:
      if(key_flag == 1) {
        if(!(RAND_bytes(salt,8))) {
          perror("\nERROR, aes_cipher.c RAND_bytes salt ");
          return 1;
        }
        RAND_bytes(data, 32);

        i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, data, data_len, rounds, key, iv);
        if(i != 32) {
          perror("\nERROR, key != 32");
          return 1;
        }

        EVP_CIPHER_CTX_init(e_ctx);
        EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
        creat_keystore(pwd, pwd_len, key, iv);
      }
      else
      if(key_flag == 2) {
        read_keystore(pwd, pwd_len, key, iv);
        EVP_CIPHER_CTX_init(e_ctx);
        EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
      }
    break;

    case 2:
      read_keystore(pwd, pwd_len, key, iv);
      EVP_CIPHER_CTX_init(d_ctx);
      EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);
    break;
  }

  return 0;
}

int aes_encrypt(EVP_CIPHER_CTX *e, int in, int out) {

  char inbuf[SIZE] = {0};
  char outbuf[SIZE+AES_BLOCK_SIZE] = {0};
  int inlen = 0, flen = 0, outlen = 0;

  EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

  while((inlen = read(in, inbuf, SIZE)) > 0)  {
    /* printf("READ przed update: %d", inlen); */
    EVP_EncryptUpdate(e, (unsigned char*) outbuf, &outlen, (unsigned char*) inbuf, inlen);
    if(write(out, outbuf, outlen) != outlen) {
      perror("\nERROR, aes_cipher.c write - enc ");
      return 1;
    }
  }

  EVP_EncryptFinal_ex(e, (unsigned char*) outbuf, &flen);
  if(write(out, outbuf, flen) != flen) {
    perror("\nERROR, aes_cipher.c write - final enc ");
    return 1;
  }

  return 0;
}

int aes_decrypt(EVP_CIPHER_CTX *d, int in, int out) {
  int inlen = 0, flen = 0, outlen = 0;
  char inbuf[SIZE+AES_BLOCK_SIZE] = {0};
  char outbuf[SIZE+AES_BLOCK_SIZE] = {0};

  EVP_DecryptInit_ex(d, NULL, NULL, NULL, NULL);

  while((inlen = read(in, inbuf, SIZE)) > 0)  {
    EVP_DecryptUpdate(d, (unsigned char*)outbuf, &outlen, (unsigned char*)inbuf, inlen);
    if((write(out, outbuf, outlen)) != outlen) {
      perror("\nERROR, aes_cipher.c write dec ");
      return 1;
    }
  }

  EVP_DecryptFinal_ex(d, (unsigned char*)outbuf, &flen);
  if((write(out, outbuf, flen)) != flen) {
    perror("\nERROR, aes_cipher.c write - final dec ");
    return 1;
  }

  return 0;

}
