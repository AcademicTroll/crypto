#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <error.h>

#include "aes_cipher.h"
#include "cipher_file.h"


int key_flag;

int main(){
  unsigned char in_path[4096];
  unsigned char pwd[256];
  int ci_flag = 0; /* 1 - ENC, 2 - DEC */
  unsigned int i, npwd = 0, upwd = 0, lenpwd = 0;
  struct termios old, new;
  struct stat in;
  char ch;

  EVP_CIPHER_CTX en,de;

  /* ASCII ART */
  printf("\n'||''''| '||\\   ||` .|'''',                           ||");
  printf("\n ||   .   ||\\\\  ||  ||                                ||");
  printf("\n ||'''|   || \\\\ ||  ||      '||''| '||  ||` '||''|, ''||''  .|''|, '||''|");
  printf("\n ||       ||  \\\\||  ||       ||     `|..||   ||  ||   ||    ||  ||  ||");
  printf("\n.||....| .||   \\||. `|....' .||.        ||   ||..|'   `|..' `|..|' .||.");
  printf("\n                                     ,  |'   ||");
  printf("\n                                      ''    .||");
  printf("\n'||'''|. '||''''| .|'''',                           ||");
  printf("\n ||   ||  ||   .  ||                                ||");
  printf("\n ||   ||  ||'''|  ||      '||''| '||  ||` '||''|, ''||''  .|''|, '||''|");
  printf("\n ||   ||  ||      ||       ||     `|..||   ||  ||   ||    ||  ||  ||");
  printf("\n.||...|' .||....| `|....' .||.        ||   ||..|'   `|..' `|..|' .||.");
  printf("\n                                   ,  |'   ||");
  printf("\n                                    ''    .||");



  printf("\n\nEnter the path of the file (e.g. /home/user/plaintext.txt):\n");
  if((fgets((char *)in_path, 4096, stdin)) == NULL) {
    perror("\n ERROR, main.c fgets: in_path ");
    return 1;
  }

  i = 0;
  while(in_path[i] != '\n' && in_path[i] != '\0') i++;
  if(in_path[i] == '\n')
  {
    in_path[i] = '\0';
    i = 0;
  }
  if(stat((char *) in_path, &in) != 0){
    perror("\nERROR, main.c stat: in_path ");
    return 1;
  }

  printf("\nEnter the number: \n 1 - for Encryption\n 2 - for Decryption \n");
  scanf("%d", &ci_flag);
  if(ci_flag != 1 && ci_flag != 2) printf("\nWrong number entered \n");
  if(ci_flag == 1){
    printf("\nEnter the number:\n 1 - for create new keystore \n 2 - for use existing keystore \n");
    scanf("%d", &key_flag);
  }

  /*
   * termio.h
   * PASSWORD INPUT SECTION
   *
   */

  /* truncate newlines in input */
  while((ch = getchar()) != '\n');

  if(tcgetattr(fileno(stdin), &old) != 0)
  {
    perror("\nERROR, main.c tcget ");
    return 1;
  }
  /* set ECHO off */
  new = old;
  new.c_lflag &= ~ECHO;

  if(tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0)
  {
    perror("\nERROR, main.c tcset - 1 ");
    return 1;
  }

  /* password input */
  printf("\nPassword rules: length min 10, length max 30, min 2 numbers, min 1 uppercase\n\nEnter the password: ");
  if((fgets((char *)pwd, 256, stdin)) == NULL)
  {
    perror("\n ERROR, main.c fgets pwd ");
    return 1;
  }

  /* restoring stdin to old settings */
  if(tcsetattr(fileno(stdin), TCSAFLUSH, &old) != 0)
  {
    perror("\nERROR, main.c tcset - 2 ");
    return 1;
  }

  /*
   * end (termio.h)
   * end of PASSWORD INPUT SECTION
   *
   */

  i = 0;
  while(pwd[i] != '\n' && pwd[i] != '\0'){
    if(pwd[i] >= 48 && pwd[i] <= 57) ++npwd;
    if(pwd[i] >= 65 && pwd[i] <= 90) ++upwd;
    ++i;
  }

  /* trunc new line if present */
  if(pwd[i] == '\n')
  {
    pwd[i] = '\0';
  }
  lenpwd = i;

  /* check password rules */
  if( (npwd < 2) || (!upwd) || (lenpwd<10) || (lenpwd>30) )
  {
    printf("\nError on checking password rules: \n");
    return 1;
  }

  switch(ci_flag){
    /* encryption */
    case 1:
      aes_init(pwd,lenpwd,&en,(EVP_CIPHER_CTX *) NULL, 1);
      cipher_file(in_path, &en, ci_flag);
    break;

    /* decryption */
    case 2:
      aes_init(pwd, lenpwd, (EVP_CIPHER_CTX *) NULL, &de, 2);
      cipher_file(in_path, &de, ci_flag);
    break;
  }

  if(ci_flag == 1){
    EVP_CIPHER_CTX_cleanup(&en);
  }
  if(ci_flag == 2){
    EVP_CIPHER_CTX_cleanup(&de);
  }


  return 0;
}
