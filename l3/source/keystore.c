# include <stdio.h>
# include <stdlib.h>
# include <openssl/evp.h>
# include <openssl/rand.h>
# include <openssl/aes.h>
# include <openssl/sha.h>
# include <unistd.h>
# include <error.h>
# include <sys/types.h>
# include <fcntl.h>
# include <sys/stat.h>
# include "keystore.h"

# define itr 1000
# define SIZE 32

int creat_keystore(unsigned char *pwd,unsigned int pwd_len,unsigned char *k,unsigned char *v)
{
	int ks=0,i=0,j=0,wr=0,outlen=0;
	unsigned char salt[32]={0},pkey[32]={0},outbuf[SIZE+AES_BLOCK_SIZE]={0},md[SHA256_DIGEST_LENGTH]={0};
	/* for pass with hash */
  unsigned char psalt[64]={0},salt_phash[32]={0};
	unsigned char path[256] = "/home/karol/crypto/crypto2/keystore" ;
	EVP_CIPHER_CTX p;
	/* for pass hash */
  SHA256_CTX sh;

  /* -1 == error for open func */
  ks=open((const char *)path,O_CREAT|O_RDWR|O_TRUNC,S_IREAD|S_IWRITE);

	RAND_bytes(salt,32);
  /* salt for session key */
	PKCS5_PBKDF2_HMAC_SHA1((const char *)pwd,pwd_len,salt,32,itr,32,pkey);
  /* salt for pass hash */
  RAND_bytes(salt_phash,32);

	i=0,j=0;
	while(i<64)	{
		if(i<pwd_len)	{
			psalt[i] = pwd[i];
		}
		if(i>=pwd_len && i<64) {				/* Specify max pwd len in pwd input MAX_PWD_SIZE 31 */
			if(j<32) {
				psalt[i] = salt_phash[j];
				++j;
			}
			else { break;}
		}
		++i;
	}
	if(i <64)	{
		psalt[i] = '\0';
	}
	else { return 1;}

  /* gen pass hash: pwd = input pwd + salt */
	SHA256_Init(&sh);
	SHA256_Update(&sh,psalt,i);
	SHA256_Final(md,&sh);

	/* 1 item in ks: write pass hash 32 bytes */
  wr=0;
	if((wr=write(ks,md,SHA256_DIGEST_LENGTH)) != SHA256_DIGEST_LENGTH) {
		return 1;
	}

  /* 2 item in ks: salt for pass hash 32 bytes */
	wr=0;
	if((wr=write(ks,salt_phash,32)) != 32) {
		if(wr ==0) { }
    else
    if(wr<0) {
      perror("\n ERROR,write_PSALT_ITEM 2::");
    }
    return 1;
	}

	/* 3 item in ks: salt to gen session key 'S' 32 bytes */
  wr=0;
	if((wr=write(ks,salt,32)) != 32) {
		if(wr ==0) { }					/* Will be needed to gen S to decrypt k */
		else
		if(wr<0) { perror("\n ERROR,write::");}
		return 1;
	}

	/* 4 item in ks: iv 'v' 32 bytes
   * will be needed for decrypt data */
  wr=0;
  if((wr=write(ks,v,32)) != 32) {
      if(wr ==0) { }
      else
      if(wr<0) {
        perror("ERROR, write error");
      }
      return 1;
  }

	EVP_CIPHER_CTX_init(&p);
	EVP_EncryptInit_ex(&p, EVP_aes_256_cbc(), NULL, pkey, salt);
	EVP_EncryptUpdate(&p,outbuf,&outlen,k,SIZE);

  if(write(ks,outbuf,outlen) != outlen)	{
		return 1;
	}

	EVP_EncryptFinal_ex(&p,outbuf,&outlen);

  /* 5 item in ks: key final 48 bytes */
	if(write(ks,outbuf,outlen) != outlen)	{
		return 1;
	}

	close(ks);
	EVP_CIPHER_CTX_cleanup(&p);
	return 0;
}

/***************************************************** read_keystore() *****************************************/

int read_keystore(unsigned char *pwd,unsigned int pwd_len,unsigned char* k, unsigned char *v)
{
	int ks=0,i=0,j=0,rd=0,outlen=0;
	unsigned char s_key[32]={0},md[SHA256_DIGEST_LENGTH]={0},r_phash[SHA256_DIGEST_LENGTH]={0},salt_s[32]={0},salt_h[32]={0},psalt[64]={0};
	unsigned char key_e[48]={0},outbuf[SIZE+AES_BLOCK_SIZE]={0};
	char path[256]= "/home/karol/crypto/crypto2/keystore";

	EVP_CIPHER_CTX dp;
	SHA256_CTX sh;

	ks=open(path,O_RDONLY);

  /* read stored pwd hash for compare - 32 bytes */
	rd = 0;
	if((rd=read(ks,r_phash,SHA256_DIGEST_LENGTH)) != SHA256_DIGEST_LENGTH) {
		perror("\n ERROR,Reading_PHASH_RKS::");
		return 1;
	}

	rd=0;
	if((rd=read(ks,salt_h,32)) != 32)	{
		perror("\n ERROR,READ_SALT_H_READKS::");
		return 1;
	}

	i=0,j=0;
  while(i<64) {
    if(i<pwd_len) {
      psalt[i] = pwd[i];
    }
    /* specify max pwd len in pwd input MAX_PWD_SIZE 31 */
    if(i>=pwd_len && i<64) {
      if(j<32) {
        psalt[i] = salt_h[j];
        ++j;
      }
      else { break; }
    }
    ++i;
  }

	if(i <64) {
    psalt[i] = '\0';
  }
  else { return 1;}

	SHA256_Init(&sh);
	SHA256_Update(&sh,psalt,i);
	SHA256_Final(md,&sh);

	/* Compare two pwd hashes r_phash & md */
	i=0,j=0;
	for(i=0;i<32;i++)	{
		if(md[i] == r_phash[i])	{
			++j;
		}
	}
  /* if not match */
	if(j!=32)	{
		return 1;
	}

	/* reading salt to generate session key */
  rd = 0;
	if((rd=read(ks,salt_s,32)) != 32)	{
		perror("\n ERROR,Read_SALT_S_RKS::");
		return 1;
	}

	PKCS5_PBKDF2_HMAC_SHA1((const char *)pwd,pwd_len,salt_s,32,itr,32,s_key);

	rd=0;
	if((rd=read(ks,v,32)) !=32)	{
		perror("\n ERROR,READ_V_RKS::");
		return 1;
	}

	rd=0;
	if((rd=read(ks,key_e,48)) != 48)	{
		perror("\n ERROR,Read_A_KEY_RKS::");
		return 1;
	}

	EVP_CIPHER_CTX_init(&dp);
	EVP_DecryptInit_ex(&dp,EVP_aes_256_cbc(), NULL,s_key,salt_s);
	EVP_DecryptUpdate(&dp,outbuf,&outlen,key_e,48);

	i=0;
	while(i<outlen && i<32)	{
		k[i] =outbuf[i];
		++i;
	}

	EVP_DecryptFinal_ex(&dp,outbuf,&outlen);


	close(ks);
  EVP_CIPHER_CTX_cleanup(&dp);
	return 0;
}
