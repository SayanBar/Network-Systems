/*Author:Sayan Barman
****reference:http://www.binarytides.com/server-client-example-c-sockets-linux/
Description: Client that connects to multiple servers
*/

//Header files
#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include "common.h"



//global variables
aliveConn * serverConnStatus;
char* _global_ServerIPAndPort[MIN][3];
int globalCounter = 0;
char globalUserName[MIN];
char globalPassword[MIN];


//Function prototypes
void connect_Client( );
int putFileServer(char * filePathInput, char * serverFolderPath, int fileSize);
int getFileServer(char * filePathInput, char * serverFolderPath);
void* handle(void*); 
char* getFileFromDirString(char* token);
int clientConfig();
int mergeFile(char * filePathInput);
void listFile(char * serverFolderPath);



//This function gets all the chunks from the servers
int getFileServer(char * filePathInput, char * serverFolderPath)
{
    connect_Client();
    frameInfo * packetInfo = (frameInfo *)malloc(sizeof(frameInfo));
    int length = sizeof(frameInfo);
    int read_size =0, errFlag=0;
    FILE* file = NULL;
    for(int i=0; i<globalCounter; i++)
    {
        memset(packetInfo, '\0', length);
        sprintf(packetInfo->command,"%s","GET"); 
	    sprintf(packetInfo->fileName,"%s",filePathInput);
		sprintf(packetInfo->username , "%s",globalUserName);
		sprintf(packetInfo->pasword , "%s",globalPassword);
		sprintf(packetInfo->folderName , "%s",serverFolderPath);
		printf("packetInfo->username : %s packetInfo->pasword :%s\n",packetInfo->username, packetInfo->pasword);
		if(serverConnStatus->connstatus[i])
		{	
			if(send(serverConnStatus->socketID[i], packetInfo, length, MSG_NOSIGNAL)<0)
			 { 

				serverConnStatus->connstatus[i] = 0;
				connect_Client();
			 }
			else {
				memset(packetInfo, '\0', length);
				if ((read_size = recv(serverConnStatus->socketID[i], packetInfo, length, 0)) > 0) {
					errFlag = packetInfo->errFlag;
					if(errFlag)printf("Error Message From Server: %s\n", packetInfo->errMessage);
					if((!packetInfo->dataFlag)&&(!(errFlag!=0))){
							int fileCounter =0, fileMaxNum = packetInfo->readBytes; 
						
						while(fileCounter < fileMaxNum){
							memset(packetInfo, '\0', length);
							if ((read_size = recv(serverConnStatus->socketID[i], packetInfo, length, 0)) > 0) {
								int fileSizeCounter = 0, readBytes = 0, writtenBytes = 0;
								printf("The file name is :%s Size:%d\n",packetInfo->fileName, packetInfo->fileSize );				    
								file = fopen(packetInfo->fileName, "wb+"); //501error
								while (fileSizeCounter < packetInfo->fileSize) { 
									memset(packetInfo,'\0', length);
									readBytes = read( serverConnStatus->socketID[i] , packetInfo, length);
									
									encryptDecrypt(packetInfo->data,packetInfo->readBytes, globalPassword, strlen(globalPassword));
									writtenBytes = fwrite(packetInfo->data, 1, packetInfo->readBytes, file); printf("ReadBytes : %d writtenBytes:%d\n", packetInfo->readBytes, writtenBytes);
									fflush(file);
									fileSizeCounter += writtenBytes;
								}
								
									fclose(file);
							}
							fileCounter++;
						}	
					}
				}	
			}
		}
	}
	free(packetInfo);
        if(mergeFile(filePathInput)!=0)
	 {
            printf("The file %s is incomplete. All the chunks were not available\n", filePathInput);
	 }
	else
	{
		printf("The file %s was succesfully received\n",filePathInput);
	}
    return 0;
}

//This function connects clients to servers on four different sockets
void connect_Client()
{
    for (int i = 0; i < globalCounter; i++) {
       if(!serverConnStatus->connstatus[i]){
        struct sockaddr_in servaddr;
        if ((serverConnStatus->socketID[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("The socket could not be created\n");
            exit(-1);
        }
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(stringToInt(_global_ServerIPAndPort[i][2]));
        if (inet_pton(AF_INET, _global_ServerIPAndPort[i][1], &servaddr.sin_addr) != 1) {
            printf("Invalid IP Address\n");
            exit(-1);
        }

        
        if (connect(serverConnStatus->socketID[i], (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            printf("Could not connect to the Server IP:%s Port:%s\n", _global_ServerIPAndPort[i][1], _global_ServerIPAndPort[i][2]);
             	
	    time_t start, end;
	    double elapsed;  // seconds
	    start = time(NULL);
	    int terminate = 1;
	    while (terminate) {
	     end = time(NULL);
	     elapsed = difftime(end, start);
	     if (elapsed >= 1.0 /* seconds */)
	       terminate = 0;
	         if(connect(serverConnStatus->socketID[i], (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		     serverConnStatus->connstatus[i] = 0; 
		 }
		 else {
	 		serverConnStatus->connstatus[i] = 1; 
    		 }
		}
        }else serverConnStatus->connstatus[i] = 1;
    } 
   }
}

//This function lists out the contents of all the servers
void listFile(char * serverFolderPath)
{
     connect_Client();
     int counter1=0, inComplete=0;
     frameInfo * packetInfo = (frameInfo *) malloc(sizeof(frameInfo));
     char buf[MAX];
     char * ptrArray[200];
     int dst = 0, i,j=0;
     int counter =0;
     char * pch;
     int read_size =0, errFlag=0; 
     memset(buf, '\0', MAX);
     size_t length = sizeof(frameInfo);
     for(int i=0; i<globalCounter; i++)
     {
        memset(packetInfo, '\0', length);
        sprintf(packetInfo->command,"%s","LIST"); 
	sprintf(packetInfo->username , "%s",globalUserName);
	sprintf(packetInfo->pasword , "%s",globalPassword);
	sprintf(packetInfo->folderName , "%s",serverFolderPath);
printf("packetInfo->username : %s packetInfo->pasword :%s\n",packetInfo->username, packetInfo->pasword);
		if(serverConnStatus->connstatus[i])
		{	
			if(send(serverConnStatus->socketID[i], packetInfo, length, MSG_NOSIGNAL)<0){ 
				serverConnStatus->connstatus[i] = 0;
				connect_Client();
			 }else{
			memset(packetInfo, '\0', length);
			if ((read_size = recv(serverConnStatus->socketID[i], packetInfo, length, 0)) > 0) {
				errFlag = packetInfo->errFlag;
				if(errFlag){
					printf("Error Message From Server: %s\n", packetInfo->errMessage);
					return;
				}
				if((!packetInfo->dataFlag)&&(!(errFlag!=0))){
					printf("packetInfo->data : %s\n",packetInfo->data);
					sprintf(buf,"%s%s%s",buf,FILE_DELIMITER,packetInfo->data);
				}
			}
			}
		}
      }	
      printf("All the files :%s\n",buf);
      
	  pch = strtok (buf,FILE_DELIMITER);
	  while (pch != NULL)
	  {
	    ptrArray[counter] = (char *)malloc(200); 
	    printf ("%s\n",pch);
	    strcpy(ptrArray[counter],pch);
	    pch = strtok (NULL, FILE_DELIMITER);
	    counter++;
	  }

	
	char temp[200]; memset(temp,'\0',200);
	 for (int i = 0; i < counter; i++) {
	      for (int j = 0; j < counter - 1; j++) {
		 if (strcmp(ptrArray[j], ptrArray[j + 1]) > 0) {
		    strcpy(temp, ptrArray[j]);
		    strcpy(ptrArray[j], ptrArray[j + 1]);
		    strcpy(ptrArray[j + 1], temp);
		 }
	      }
	   }


	  
	   for (i = 1; i < counter; ++i) {
	       if (strcmp (ptrArray[dst], ptrArray[i]) != 0){
		   ptrArray[++dst] = ptrArray[i];
		   j++;
		   }
	   }

	for(int i =0; i<=j; i++)
	{
	printf("ptrArray[%d]: %s\n",i,ptrArray[i]);
	
	} 

	
for(int i =0; i<=j; i++)
{
	if(!(i^j))
	{	
		inComplete= j;
		break;
	}
	if(!(strncmp(ptrArray[i],ptrArray[i+1], (strlen(ptrArray[i])-1))!=0))
		counter1=counter1+1;
		
	else
	{

		if(counter1==3)
		{
		 	int check = strlen(ptrArray[i])-2;
			printf("Iteration: %d\n",check );
		 	ptrArray[i][check]='\0';
		 	printf("%s\n",ptrArray[i]);
		}
		else
		{
		 	int check = strlen(ptrArray[i])-2;
			printf("Iteration : %d\n",check );
			ptrArray[i][check]='\0';
		 	printf("%s - Incomplete\n",ptrArray[i]);
		}
		counter1 =0;
	}
	

}
        
if(counter1<3)
{
		 	int check = strlen(ptrArray[inComplete])-2;
			printf("Iteration : %d\n",check );
			ptrArray[inComplete][check]='\0';
		 	printf("%s - Incomplete\n",ptrArray[inComplete]);
}else
{
int check = strlen(ptrArray[inComplete])-2;
			printf("Iteration : %d\n",check );
		 	ptrArray[inComplete][check]='\0';
		 	printf("%s\n",ptrArray[inComplete]);
}
}


//This function splits and sends the file to different servers
int putFileServer(char * filePathInput,char * serverFolderPath, int fileSize)
{
	if(fileSize<=0) 
	{
		printf("File Size is 0\n");return 0;
	}
     connect_Client();
     char buf[MAX];
    memset(buf, '\0', MAX);
    int fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
    FILE* file = NULL;
    file = fopen(filePathInput, "rb+"); 
    int totalCounter = 0;
    int read_size =0, errorServer1=0, errorServer2=0;
    size_t length = sizeof(frameInfo);
    int serverVisit =0;
    int modVal = md5Mod(filePathInput);
     int fileChunkSize = shift((unsigned int)(fileSize/TOTAL_SERVERS));
     frameInfo * packetInfo = (frameInfo *) malloc(sizeof(frameInfo));
    for(int i=modVal; serverVisit<globalCounter; i++)
    {  	
	if(i==globalCounter)
		i=0; 
	
	int j =((i-1)<0?3:i-1);
        memset(packetInfo, '\0', length);
        sprintf(packetInfo->command,"%s","PUT"); 
	sprintf(packetInfo->fileName,"%s.%d",filePathInput,i);
	if(serverConnStatus->connstatus[i]||serverConnStatus->connstatus[j])
	{	
                if(serverVisit==(globalCounter-1)) fileChunkSize = fileSize-totalCounter;
                packetInfo->fileSize = fileChunkSize;
		sprintf(packetInfo->folderName , "%s",serverFolderPath);
		sprintf(packetInfo->username , "%s",globalUserName);
		sprintf(packetInfo->pasword , "%s",globalPassword);
		
		if(serverConnStatus->connstatus[i]) 
		{
			if(send(serverConnStatus->socketID[i], packetInfo, length, MSG_NOSIGNAL)<0)
			{ 
				serverConnStatus->connstatus[i] = 0;
				connect_Client();
			 }
		}
		
         	
		if(serverConnStatus->connstatus[j]) 
		{
			if(send(serverConnStatus->socketID[j], packetInfo, length, MSG_NOSIGNAL)<0)
				{ 
				serverConnStatus->connstatus[i] = 0;
				connect_Client();
			 }else{
			memset(packetInfo, '\0', length); 
		 	if ((read_size = recv(serverConnStatus->socketID[j], packetInfo, length, 0)) > 0) { 
				errorServer2 = packetInfo->errFlag;
				if(errorServer2)printf("Error From Server %d : %s\n", j, packetInfo->errMessage);
			}
			}
		}
		if(serverConnStatus->connstatus[i]) 
		{
			memset(packetInfo, '\0', length); 
			if ((read_size = recv(serverConnStatus->socketID[i], packetInfo, length, 0)) > 0) { 
				errorServer1 = packetInfo->errFlag;
				if(errorServer1)printf("Error From Server %d: %s\n",i, packetInfo->errMessage);
			}
		}

		fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
		while ((fileSizeCounter < fileChunkSize)&&(!(errorServer1||errorServer2))) {
			
			memset(packetInfo, '\0', length);
		        sprintf(packetInfo->command,"%s","PUT"); 
			sprintf(packetInfo->fileName,"%s.%d",filePathInput,serverVisit);
			sprintf(packetInfo->folderName , "%s",serverFolderPath);
			sprintf(packetInfo->username , "%s",globalUserName);
			sprintf(packetInfo->pasword , "%s",globalPassword);
			printf("THe file name :%s\n",packetInfo->fileName);
			packetInfo->fileSize = fileChunkSize;
		        int readLimit;  if(fileChunkSize<MAX)readLimit = fileChunkSize;else readLimit=MAX;
			packetInfo->dataFlag = 1;
	 		readBytes = fread(packetInfo->data, 1, readLimit, file); 
			encryptDecrypt(packetInfo->data,readBytes, globalPassword, strlen(globalPassword));
		        packetInfo->readBytes= readBytes;
			
			if((!(errorServer1!=0))&&(serverConnStatus->connstatus[i]))
			{
				sendBytes = send(serverConnStatus->socketID[i], packetInfo, length, MSG_NOSIGNAL);
				if(sendBytes<0){ 

				serverConnStatus->connstatus[i] = 0;
				connect_Client();
			 }
			}
			if((!(errorServer2!=0))&&(serverConnStatus->connstatus[j]))
			{
				sendBytes = send(serverConnStatus->socketID[j], packetInfo, length, MSG_NOSIGNAL);
				if(sendBytes<0){ 
				serverConnStatus->connstatus[i] = 0;
				connect_Client();
			 }
			}
			fileSizeCounter += readBytes;printf("fileSizeCounter:%d fileChunkSize :%d\n", fileSizeCounter, fileChunkSize);
	        } 
		totalCounter+= fileSizeCounter;
	        printf("done with index : %d\n", i);
	} 
	serverVisit++;
    }
    fclose(file);
	free(packetInfo);
   
     return 0;
}


//This function is used to parse information from user conf file
int clientConfig(char* confFileName)
{
    FILE* webConfig;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char temp[MIN] = { 0 };
    int counter = 0;
    char* tempBuf = (char*)malloc(MIN);
    memset(tempBuf, '\0', MIN);
    sprintf(tempBuf, "%s%s", CLIENT_CONFIG, confFileName);
    webConfig = fopen(tempBuf, "rb");
    if (webConfig == NULL) {
        perror("WebConfig File error:");
        exit(-1);
    }
    free(tempBuf);
    
    while ((read = getline(&line, &len, webConfig)) != -1) {
        if (strstr(line, USERNAME) != NULL) //Listen
        {
            sscanf(line, "%*s %s", globalUserName);
            
            continue;
        }
        if (strstr(line, PASSWORD) != NULL) //Listen
        {
            sscanf(line, "%*s %s", globalPassword);
            continue;
        }
        if (strstr(line, SERVER) != NULL) {
            sscanf(line, "%*s %s %s %s", (_global_ServerIPAndPort[counter][0] = (char*)malloc(strlen(line))), (_global_ServerIPAndPort[counter][1] = (char*)malloc(strlen(line))), (_global_ServerIPAndPort[counter][2] = (char*)malloc(sizeof(int))));
            counter++;
        }
    }
    globalCounter = counter;
}



//The main function sets up the client and calls all functions for all functionalities
int main(int argc, char* argv[])
{
    int sockfd;
    int clientfd;
    char buf[MAX];
    struct sockaddr_in servaddr;
    char commandInput[MAX];
    char filePathInput[MAX];
    char serverFolderPath[MAX];
    
    if (argc != 2) {
        printf("Incorrect usage\nExample: ./client AdamDFC.conf\n");
        exit(0);
    }

    if (!clientConfig(argv[1])) 
    {
        printf("Error in conf file. Please restart\n");
        exit(0);
    }
	serverConnStatus = ( aliveConn *) malloc(sizeof(aliveConn));
    frameInfo * packetInfo = ( frameInfo *) malloc(sizeof(frameInfo));
    memset(serverConnStatus, '\0', sizeof(aliveConn));
    memset(packetInfo, '\0', sizeof(frameInfo));
	serverConnStatus->socketID = (int *)malloc(globalCounter * sizeof(int));
	serverConnStatus->connstatus = (int *)malloc(globalCounter * sizeof(int));
    connect_Client();
    printf("Commands Available:\n   GET [path/fileName]\n   PUT [path/fileName]\n   LIST\n\nExample: PUT foo4.pdf\n");
    
    while (1) {

        printf("\n@Waiting for Input\n\n");
        memset(buf, 0, MAX);
        memset(commandInput, 0, MAX);
        memset(filePathInput, 0, MAX);
        memset(serverFolderPath, 0, MAX);

        if (fgets(buf, MAX - 1, stdin) == NULL) {
            printf("Invalid Entry\n");
            exit(-1);
        }
        if (sscanf(buf, "%s %s %s", commandInput, filePathInput, serverFolderPath) < 1) // this itself takes care of trailing and leading zeros
        {
            printf("Invalid Entry\n");
            continue;
        }
        else {
	printf("%s %s %s", commandInput, filePathInput ,serverFolderPath);
            if (strcmp(commandInput, "PUT") == 0){
                
                if (existConfirm(filePathInput)) {
		 FILE* file = NULL;
	         file = fopen(filePathInput, "r"); //501error
       	         if (file == NULL) 
		 {
	  	    printf("Error in FIle\n");
		 }
	         fseek(file, 0, SEEK_END);
	         int fileSize = ftell(file);
	         rewind(file);
	         fclose(file);

                    
                    if(putFileServer(filePathInput, serverFolderPath ,fileSize)==0) 
		    {
                            printf("The Md5sum of the file %s is:",filePathInput);
                            sprintf(buf,"md5sum %s",filePathInput);
                            system(buf);
                        }
                }
                printf("put command \n");
            }
            else if (strcmp(commandInput, "GET") == 0) 
	    {
                printf("get command \n");
                if(getFileServer(filePathInput,serverFolderPath)==0)
                {
                }
            }
            else if (strcmp(commandInput, "LIST") == 0) 
	    {
                printf("ls command \n");
                listFile(filePathInput);
            }
            else 
                printf("other command \n");
            
        }
    }

    return 0;
}


//This function merges all the chunks after retrieving them from the servers
int mergeFile(char * filePathInput)
{
  FILE * file =NULL;
  file = fopen(filePathInput, "wb+");
  char buf[MAX];
  int readBytes=0,writtenBytes=0;
  for(int i=0; i<globalCounter; i++)
  {
	FILE * chunk =NULL;
	memset(buf, '\0', MAX);
	sprintf(buf,"%s",filePathInput);
        if(getChunkName(buf,i)!=0) printf("THe file pointer requested by user is NULL\n");
        printf("chunk name: %s\n",buf);
	if(!(existConfirm(buf)!=0))
	{ 
		return -1;
		continue;
	}
	chunk = fopen(buf, "rb+");
	
	
	
        while(!feof(chunk) && !ferror(chunk))
	{
		memset(buf, '\0', MAX);
		readBytes = fread(buf, 1, MAX, chunk);
		writtenBytes = fwrite(buf, 1,readBytes, file);
		
	}       
	fclose(chunk);
  }
  fclose(file);
  return 0;
}

