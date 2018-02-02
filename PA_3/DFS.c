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
#include <sys/stat.h>
#include "common.h"




//Global Variables 
char* globalUserNameAndPassword[MIN][2];
char* globalPath;
unsigned int globalCounter=0;


//Declaration of Functions
int getFileServer(int sock, char * filePathInput, int fileSize);
void* handle(void*); 
char* getFileFromDirString(char* token);
int serverConfig();
int putFileServer( int sock , frameInfo * packetInfo);


//This file is for sending the file
int getFileServer(int sock, char * filePathInput, int fileSize)
{
    printf("The file name is :%s",filePathInput);
     frameInfo * packetInfo = (frameInfo *) malloc(sizeof(frameInfo));
    int fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
    char * fileName = (char *)malloc(sizeof(strlen(filePathInput)));
    memset(fileName,'\0', strlen(filePathInput));
    FILE* file = NULL;
    file = fopen(filePathInput, "rb+"); 
    size_t length = sizeof(frameInfo);	
        memset(packetInfo, '\0', length);
        sprintf(packetInfo->command,"%s","GET"); 
        if((fileName = filenameFromDirString(filePathInput))==NULL) 
	{ 
          printf("The file Name doesn't exist in Directory String\n");
	  fileName = filePathInput;
	}
	sprintf(packetInfo->fileName,"%s",fileName);
                packetInfo->fileSize = fileSize;
		send(sock, packetInfo, length, MSG_NOSIGNAL);  
	       
		        int readLimit;  if(fileSize<MAX)readLimit = fileSize;else readLimit=MAX;
		fileSizeCounter = 0, readBytes = 0, sendBytes = 0;
		while (fileSizeCounter < fileSize) {
			
			memset(packetInfo, '\0', length);
		        sprintf(packetInfo->command,"%s","GET"); 
			sprintf(packetInfo->fileName,"%s",fileName);
			packetInfo->fileSize = fileSize;
			packetInfo->dataFlag = 1;
	 		readBytes = fread(packetInfo->data, 1, readLimit, file);
		        packetInfo->readBytes= readBytes;
			sendBytes = send(sock, packetInfo, length, MSG_NOSIGNAL);
			fileSizeCounter += readBytes;printf("fileSizeCounter:%d fileChunkSize :%d\n", fileSizeCounter, fileSize);
	        } 
    fclose(file);
	free(packetInfo);
}

//Config function is used to set up server by parsing DFC.conf
int serverConfig()
{
    FILE* webConfig;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char temp[MIN] = { 0 };
    globalCounter = 0;

    webConfig = fopen(SERVER_CONFIG, "rb");
    if (webConfig == NULL) {
        perror("WebConfig File error:");
	exit(-1);
    }
    
    while ((read = getline(&line, &len, webConfig)) != -1) {
        //DEBUG(printf("Retrieved line of length %zu :\n", read));
        //DEBUG(printf("%s", line));
        sscanf(line, "%s %s", (globalUserNameAndPassword[globalCounter][0] = (char*)malloc(strlen(line))), (globalUserNameAndPassword[globalCounter][1] = (char*)malloc(strlen(line))));
        printf("globalUserNameAndPassword:%s %s\n", globalUserNameAndPassword[globalCounter][0], globalUserNameAndPassword[globalCounter][1]);
        globalCounter++;
    }
}


//Main function is used to setup sockets and spawn threads for each client
int main(int argc, char* argv[])
{
    int socket_desc, client_sock, c, *new_sock, optval=1;
    struct sockaddr_in server, client;
    if(argc != 3)
    {
        printf("Usage incorrect\nExample: ./dfs DF1 10001\n");
        exit(0);
    }
    if (!serverConfig()) // using NOt since it is faster that comparator operation
    {
        printf("Error in conf file.Please restart\n");
        exit(0);
    }

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");
    
    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    globalPath = (char *)malloc(sizeof(20));
    sprintf(globalPath,"./%s", argv[1]);
    struct stat st = {0};

    if (stat(globalPath, &st) == -1) {
	    mkdir(globalPath, 0777);
    }


    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(stringToInt(argv[2]));

    //Bind
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind successful");

    //Listen
    listen(socket_desc, 10);

    //Accept and incoming connection
    puts("Waiting for clients...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c))) {
        puts("Connection accepted");

        pthread_t clientThread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&clientThread, NULL, handle, (void*)new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }

        
        puts("Handler thread spawned");
	
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}


//This function is to verify the username and password
int verifyUserNamePassword(char * userName, char* password)
{
int count = 0;
while(count <globalCounter )
{
	if((strcmp(globalUserNameAndPassword[count][0], userName)==0)&&(strcmp(globalUserNameAndPassword[count][1], password)==0))
	return 0 ;
	count ++;
}
return -1;
}


//This function is used to receive the file
int putFileServer( int sock , frameInfo * packetInfo)
{
	char totalPath[MIN]; 
	int errFlag=0;
	FILE* file = NULL;
	int length = sizeof(frameInfo);
	memset(totalPath, '\0', MIN);
	int fileSize = packetInfo->fileSize;
	int fileSizeCounter = 0, readBytes = 0, writtenBytes = 0;
	sprintf(totalPath,"%s/%s/%s",globalPath,packetInfo->username, packetInfo->folderName);
	
	if(!(verifyUserNamePassword(packetInfo->username, packetInfo->pasword)!=0))
        {
	   struct stat st = {0};
	
 	   if (stat(totalPath, &st) == -1) {
	       if(mkdir(totalPath, 0777)!=0)
		{
			packetInfo->errFlag = errFlag= 1;
			sprintf(packetInfo->errMessage, "The folder could not be created successfully at Server: %s\n",totalPath);
			printf("%s",packetInfo->errMessage);
		}
   	 }
	sprintf(totalPath,"%s/%s",totalPath,packetInfo->fileName);
 	file = fopen(totalPath, "wb+"); //501error
	}else
	{
		packetInfo->errFlag = errFlag= 1;
		sprintf(packetInfo->errMessage, "Invalid UserName and/Or Password \n");
		printf("%s",packetInfo->errMessage);
	}
	send(sock, packetInfo, length, MSG_NOSIGNAL); 
	printf("packetInfo->fileName : %s  totalPath: %s",packetInfo->fileName,totalPath);
	
	if(!(errFlag!=0)){
		while (fileSizeCounter < fileSize) {
			memset(packetInfo,'\0', sizeof(packetInfo));
			readBytes = read( sock , packetInfo, length);
			writtenBytes = fwrite(packetInfo->data, 1, packetInfo->readBytes, file);
			printf(" fileSizeCounter :%d   fileSize :%d\n",fileSizeCounter, fileSize);
			fflush(file);
			fileSizeCounter += writtenBytes;
		}
        fclose(file);
	}
return 0;
}

//This function is the handler thread for each client every time a new client connects
void* handle(void* socket_desc)
{
   
    int sock = *(int*)socket_desc;
    pthread_detach(pthread_self());
    int read_size;
    char *message, buf[MAX];
    memset(buf, '\0', MAX);
    
    frameInfo * packetInfo = (frameInfo *)malloc(sizeof(frameInfo));
    int packetLength = sizeof(frameInfo);
    char * filePath = (char * )malloc(MIN);
    while(1){
            //Receive a message from client
	    memset(filePath,'\0',MIN);
            if ((read_size = recv(sock, packetInfo, packetLength, 0)) > 0) {
		 if(!packetInfo->dataFlag){
                           if (strcmp(packetInfo->command, "PUT") == 0){
				  putFileServer(sock , packetInfo);
				printf("put command completed\n");
			    }
			    else if (strcmp(packetInfo->command, "GET") == 0) {
				sprintf(filePath,"%s/%s/%s",globalPath,packetInfo->username, packetInfo->folderName);
				printf("get command selected filePath: %s fileNAme: %s\n",filePath,packetInfo->fileName);
				int fileExists =0;
				char * fileArry[2];
				char buffer[MIN];
				fileArry[0] = (char*)malloc(MIN);memset(fileArry[0],'\0',(MIN));
				fileArry[1] = (char*)malloc(MIN);memset(fileArry[1],'\0',(MIN));
				showDir(filePath, packetInfo->fileName,  &fileExists, fileArry);
				printf("get command selected filePath: %s fileNAme: %s\n",filePath,packetInfo->fileName);
				int counter=0, errFlag=0;
				if(verifyUserNamePassword(packetInfo->username, packetInfo->pasword)!=0)
        			{
					packetInfo->errFlag = errFlag= 1;
					sprintf(packetInfo->errMessage, "Invalid UserName and/Or Password \n");
					printf("%s",packetInfo->errMessage);

				}

			        memset(packetInfo, '\0', packetLength);		
				packetInfo->readBytes= fileExists;
				sprintf(packetInfo->data, "%s *** %s",  fileArry[0], fileArry[1] );
				printf("packetInfo->username : %s packetInfo->pasword :%s\n",packetInfo->username, packetInfo->pasword);
				
				send(sock, packetInfo, packetLength, MSG_NOSIGNAL); 

  				while((fileExists!=0)&&(counter<fileExists)&&(!(errFlag!=0)))
				{						
					printf("The file was found:%s\n",fileArry[counter]);
					FILE* file = NULL;
					memset(buffer,'\0',MIN);
					sprintf(buffer, "%s", fileArry[counter]);
					printf("The buffer file Name is :%s", buffer);
					 file = fopen(buffer, "rb+"); //501error
			       	       
					  if (file == NULL) {
				  	    printf("Error in FIle\n");
					 }
					 fseek(file, 0, SEEK_END);
					 int fileSize = ftell(file);
					 rewind(file);
					 fclose(file);
					if(getFileServer(sock,fileArry[counter], fileSize)==0)
					    {
					    }counter++;
				}
			    }
			    else if (strcmp(packetInfo->command, "LIST") == 0) {
				printf("ls command selected\n");
				sprintf(filePath,"%s/%s/%s",globalPath,packetInfo->username, packetInfo->folderName);
				printf("get command selected filePath: %s fileNAme: %s\n",filePath,packetInfo->fileName);
				int fileExists =0;
				int counter=0, errFlag=0;
				if(verifyUserNamePassword(packetInfo->username, packetInfo->pasword)!=0)
        			{
					packetInfo->errFlag = errFlag= 1;
					sprintf(packetInfo->errMessage, "Invalid UserName and/Or Password \n");
					printf("%s",packetInfo->errMessage);

				}else
				{
					sprintf(packetInfo->data, "%s", listFileInDir(filePath,&fileExists));
					printf("packetInfo->data :%s\n",packetInfo->data);
					if(!(fileExists!=0))
					{
						packetInfo->errFlag = errFlag= 1;
						sprintf(packetInfo->errMessage, "No Files Found%s\n",filePath);
						printf("%s",packetInfo->errMessage);
					}
				}
		
				send(sock, packetInfo, packetLength, MSG_NOSIGNAL);  
			    }
			    else {
				printf("other command selected\n");
				
			    }			
		}	
            }
    }		
    
    free(socket_desc);
    close(sock);
    pthread_exit(0);
    return NULL;
}

