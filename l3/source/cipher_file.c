/* cipher_file() encryptes a file
*  can be called from main()
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include "aes_cipher.h"
#include "cipher_helper.h"
#include "cipher_file.h"

int cipher_file(unsigned char *in_path, EVP_CIPHER_CTX *tx, int ci_flag){

  char enc_path[] = "/tmp/do_encr";
  char dec_path[] = "/tmp/do_dec";
  struct stat a; /* stores original file/owner perm */
  int in,out;

  in = open((char *)in_path, O_RDONLY);

  switch (ci_flag)
  {
    case 1:
      out = open(enc_path, O_CREAT|O_RDWR, 0400|0200); /* tmp for writing */
      fstat(in, &a); /* fetching file perm */
      aes_encrypt(tx, in, out); /*ENCRYPTING */

      close(in);
      close(out);

      /* replacing original file */
      replace(enc_path, (char *)in_path);

      if((unlink(enc_path)) < 0){
        perror("\nERROR, Cant unlink enc file_C_FILE::");
        return 1;
      }

      file_perres((char *)in_path, &a); /* restoring file perm */

    break;

    case 2:
      in = open((char *)in_path, O_RDONLY);
      out = open(dec_path, O_CREAT|O_RDWR, 0400|0200);

      aes_decrypt(tx, in, out);

      close(in);
      close(out);

      replace(dec_path, (char *)in_path);
      unlink(dec_path);

    break;

    default:
      printf("\nIncorrect choice entered \n");
      return 1;
    break;
  }


   return 0;
}

