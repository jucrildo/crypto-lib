CC=gcc
CFLAGS=-Wall -Wextra

crypto: DES.c
	$(CC) $(CFLAGS) -o des DES.c 
