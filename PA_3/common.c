/*Author:Sayan Barman
Description: Common functions required by server and client
*/

//Header Files
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <openssl/md5.h>
#include "common.h"


//This function appends to create chunk name
int getChunkName(char * fileName, int index)
{
       if(!(fileName!=NULL)) return -1;
       sprintf(fileName,"%s.%d", fileName, index);
       return 0; 
}

//This function confirms whether the file exists
int existConfirm(const char * filename) {
  DIR * dirparent;
  struct dirent * dp;
  long len = strlen(filename);
  dirparent = opendir(".");
  if ((dirparent = opendir(".")) != NULL) {
    while ((dp = readdir(dirparent)) != NULL) {
      if (strcmp(dp->d_name, filename) == 0) {
        (void) closedir(dirparent);
        return 1;
      }
    }
    (void) closedir(dirparent);
    printf("the file name :%s does not exist in current directory\n", filename);
  } else {
    printf("Can not open the current directory\n");
  }
  return 0;
}



//This function lists all the files in the directory
char* listFileInDir(char * path, int * fileExists){
  DIR * dirparent;
  struct dirent * dp;
  fflush(stdout);
  char *listFiles = (char*)malloc(MAX);
  memset(listFiles,'\0',sizeof(listFiles));
  dirparent = opendir(".");
  if ((dirparent = opendir(path)) != NULL) {
    while ((dp = readdir(dirparent)) != NULL) {
      if(dp-> d_type != DT_DIR) 
	{
      strcat(listFiles,dp->d_name);
      strcat(listFiles,FILE_DELIMITER);
      *fileExists =1;
	}
    }
    (void) closedir(dirparent);
  } else {
    strcat(listFiles,"Cannot open the current directory\n");
  }
  return listFiles;  
}

//This function is used to encrypt and decrypt data
void encryptDecrypt(char * data, unsigned int len, char * password, unsigned int passlen)
{
    unsigned int l;
    unsigned int m=0;
    for(l=0;l<len;l++)
    {
	if(m==passlen)
		m=0;
        data[l] = data[l] ^ password[m];
	m++;
    }
}


//This function gets the filename from the entire string
char* getFileFromDirString(char* token)
{
    char* temp;
    char* tokenCpy;
    tokenCpy = (char*)malloc(strlen(token));
    strcpy(tokenCpy, token);

    
    temp = strtok(token, "/");
 
    while (temp != NULL) {
        if (temp != NULL) {
            memset(tokenCpy, '\0', strlen(tokenCpy));
            strcpy(tokenCpy, temp);
            
        }
        temp = strtok(NULL, "/");
    }
    return tokenCpy;
}

//This functions show the directory contents
void showDir(char* path, char* fileName, int* fileExists, char * fileNameArray[])
{
    DIR* dirparent = opendir(path); // open path
    if (dirparent == NULL)
        return; 
    struct dirent* dir; // for directory entries
    while ((dir = readdir(dirparent)) != NULL) // check to read something from the directory
    {
        if (dir->d_type != DT_DIR) // check if not directory
        {
            if (strstr(dir->d_name, fileName)!=NULL) 
	    {
                sprintf(fileNameArray[ *fileExists ], "%s/%s", path, dir->d_name);
                *fileExists += 1;
            }
        }
        else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) // if it is a directory
        {
            char d_path[255]; 
            sprintf(d_path, "%s/%s", path, dir->d_name);
            showDir(d_path, fileName, fileExists, fileNameArray); 
        }
    }
    closedir(dirparent); //close the directory
}



//This function convers string to Integer
int stringToInt(char * str) {
  char * end;
  errno = 0;
  long convert = strtol(str, & end, 0);
  if (errno == ERANGE || * end != '\0' || str == end) {
    printf("Invalid  Number\n");
    exit(-1);
  }
  // Only needed if sizeof(int) < sizeof(long)
  if (convert < INT_MIN || convert > INT_MAX) {
    printf("Invalid  Number\n");
    exit(-1);
  }
  return (int) convert;
}


//This function calculates md5sum and returns mod 4
int md5Mod(char * fileName)
{
  unsigned char md5res[MD5_DIGEST_LENGTH];
    int i;
    FILE *file = fopen (fileName, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[MAX];
    unsigned char bufFInal[MAX];
    memset(data, '\0', MAX);
    memset(bufFInal, '\0', MAX);
    if (file == NULL) {
        printf ("%s can't be opened.\n", fileName);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, MAX, file)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (md5res,&mdContext);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
        char buf[MAX];
	sprintf(buf,"%02x", md5res[i]);
	strcat(bufFInal,buf); 
    }
    printf (" %s\n", fileName);
    printf("The md5 in buffer is : %s\n", bufFInal);
    fclose (file);
    
    unsigned long long x = *(unsigned long long*)bufFInal;
    unsigned long long y = *(unsigned long long*)(bufFInal + 8);
    printf("The Mod Value: %llu\n",((x ^ y)%4));
    return ((int)(x ^ y)%4);
}

//This function is used to extract filename from the Directory string
char * filenameFromDirString(char * fullPath)
{
	if(!(fullPath!=NULL))
	    return NULL;
	char * filename = (char *) malloc(sizeof(strlen(fullPath)));
	filename = strrchr(fullPath, '/');
        if(filename!=NULL)
            return ++filename;	
  	return NULL;
}


//This function left shifts the number
unsigned int shift( unsigned int x )
{
  unsigned int res = 0 ;
  while( x>>=1 ) res++;
  return 1<<res ;
}
