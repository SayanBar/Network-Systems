#Makefile

OBJS = 
HEADERS= .
INCLUDE = /usr/lib
LIBS = -lpthread -lrt -lssl -lcrypto 
all:  dfc dfs

dfc:
	gcc -w -g -o dfc  common.c DFC.c -I$(HEADERS) $(LIBS) 

dfs:
	gcc -w -o dfs common.c DFS.c -I$(HEADERS) $(LIBS)
clean:
	rm -f  dfc dfs 

