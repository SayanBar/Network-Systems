/*Author:Sayan Barman
Description: Common header required by server and client
*/


#ifndef COMMON_H
#define COMMON_H

#define MAX  1024
#define MIN 200

#define SERVER_CONFIG "/home/connoisseur/Netsys/saba7535PA#3/DFS.conf"
#define CLIENT_CONFIG "/home/connoisseur/Netsys/saba7535PA#3/"
#define TOTAL_SERVERS 4
#define FILE_DELIMITER "\n"

#define USERNAME "Username:"
#define PASSWORD "Password:"
#define SERVER   "Server"

//the declaration of the common functions in common.c
unsigned int shift( unsigned int x );
char * filenameFromDirString(char * fullPath);
int existConfirm(const char * filename);
char* listFileInDir();
int getChunkName(char * fileName, int index);
void showDir(char* path, char* fileName, int* fileExists, char * fileNameArray[]);
int stringToInt(char *str);
void encryptDecrypt(char * data, unsigned int len, char * password, unsigned int passlen);
int md5Mod(char * fileName);



typedef struct aliveConn{
    int * connstatus;
    int * socketID;
}aliveConn;

typedef struct frameInfo{ 
   char username[MIN];
   char pasword[MIN];
   char fileName[MIN];
   int dataFlag;
   int fileSize;
   char command[MIN];
   char data[MAX];
   char errMessage[MAX];
   int readBytes; 
   int errFlag; 
   char folderName[MIN];
}frameInfo;


#endif
