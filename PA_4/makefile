#Makefile


CC=gcc
INCLUDE= /usr/lib

ALL: proxy

proxy:clean
	$(CC) -g -Wall -w -o proxy proxy_server.c -lcrypto

clean:
	rm -rf proxy
