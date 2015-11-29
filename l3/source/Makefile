CC = gcc
CFLAGS = -ggdb -Wall -Werror 

IDIR = /home/karol/crypto/crypto2/include
SDIR = /home/karol/crypto/crypto2/source

LIBS = lcrypto -lpthread

ENCDEC: main.o cipher_file.o cipher_helper.o aes_cipher.o keystore.o
	$(CC) $(CFLAGS) main.o cipher_file.o cipher_helper.o aes_cipher.o keystore.o -$(LIBS) -I$(IDIR) -o ENCDEC 

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -I$(IDIR)

cipher_file.o: cipher_file.c
	$(CC) $(CFLAGS) -c cipher_file.c -I$(IDIR)

cipher_helper.o: cipher_helper.c
	$(CC) $(CFLAGS) -c cipher_helper.c -I$(IDIR)

aes_cipher.o: aes_cipher.c
	$(CC) $(CFLAGS) -c aes_cipher.c -I$(IDIR)

keystore.o: keystore.c
	$(CC) $(CFLAGS) -c keystore.c -I$(IDIR)

clean:
	rm ENCDEC *.o
