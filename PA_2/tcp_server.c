/*
AUTHOR: Sayan Barman
Description: Webserver which could handle multiple clients simultaneously

*/

//Header Files
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include<malloc.h>

//Constants
#define CONNMAX 1000
#define BYTES 1024


//Global declarations
char *buffer;
char error_code[300]="HTTP/1.1 500 Internal Server Error\n\n <html><body>500 Internal Server Error</body></html>";
char *word_req;
char ROOT[100];
int server_flag=0;
int listenfd, clients[CONNMAX];

//Declaration of prototypes
void error(char *); //function to handle server setup errors
void setup_Server(char *); //function to setup server
void request_handle(int); //forked process to handle each client
char * extract(char * firstword);  // extract the content type, port and ROOT directory



//Main function
int main(int argc, char* argv[])
{
	struct sockaddr_in clientaddr;					//Socket address declarations
	socklen_t addrlen; 						//Address length declarations
	char c,charac;  
	FILE *file;   							//File handler
	//char *port1;  //Port Number
	
	
	//char ROOT[100];
	
	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];//Port Number
	//port1=malloc(6);

									
	if((file = fopen("ws.conf","rb"))!=NULL)			//Extract port and root directory
	{
	while((charac=getc(file))!=EOF)
	{
	if(charac=='L')                                              //Looking for string "Listen"
	{
	fseek(file,ftell(file)+6,SEEK_SET);
	fgets(PORT,sizeof(PORT),file);
	}
	if(charac=='"')                                           //Looking for Document Root
	{
	fgets(ROOT,sizeof(ROOT),file);
	ROOT[strlen(ROOT)-2]='\0';
	//printf("ROOT=%s\n",ROOT);
	}	
	}
	fclose(file);
	}
	else 						//ws.conf is not available
	{
		strcpy(ROOT,"/home/connoisseur/Netsys/Prog_2");
		strcpy(PORT,"10000");
		server_flag=1;
	}	
	//port1=extract("Listen");
	//printf("%s", port1);
	

	int slot=0; 							//slots for client

	
	
	printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
	// Setting all elements to -1: signifies there is no client connected
	int i;
	for (i=0; i<CONNMAX; i++)
		clients[i]=-1;
	setup_Server(PORT);

	
	while (1)							// ACCEPT connections
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);  //connecting a client
		if(server_flag==1)                                 //if ws.conf is missing,send error 500 and close socket
		{
		write(clients[slot], error_code,strlen(error_code));         //sending error 500
		close(listenfd);
		server_flag=0;
		exit(0);		
		}
		if (clients[slot]<0)
			error ("accept() error");
		else							//Creating a child process for each client
		{
			if ( fork()==0 )
			{
				request_handle(slot);			//Child process
				
				exit(0);
			}
		}

		while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
	}

	return 0;
}


void setup_Server(char *port)				//function to setup server
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);   			//socket creation
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;  		//binding the socket
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);    						//Freeing resources

	// listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )                     	//Error Handling
	{
		perror("listen() error");
		exit(1);
	}
}


void request_handle(int n)					//forked process to handle each client
{
	FILE *file;	
	char *hello;
	char bad_request[100]="HTTP/1.0 400 Bad Request\n\n <html><body>400 Bad Request</body></html>";		//400 Bad Request
	char no_file[200]="HTTP/1.0 404 Not Found\n\n <html><body>404 File Not Found</body></html>";		//404 Not Found
	char no_support[200]="HTTP/1.0 501 Not Implemented\n\n <html><body>501 Not Implemented</body></html>";	//501 Not Implemented
	char *post;
	int length=0;
	int file_flag=0;
	char header[1000];        								//Header array
	char second_copy[99999];
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];			//Receive Message and breaking it into tokens
	char *type,*content_type;
	int rcvd, fd, bytes_read;
	int http_flag=0;
	int path_count=0;
	int count=0;
	type=malloc(20);						//Malloc allocations
	hello=malloc(20);
	post=malloc(1000);
	memset( (void*)mesg, (int)'\0', 99999 ); 			//Nulling the message array out
	
	
	rcvd=recv(clients[n], mesg, 99999, 0);
	strcpy(second_copy,mesg);
	
	
	if (rcvd<0)    							// receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    						// receive socket closed
		fprintf(stderr,"...\n");
	else    							// message received
	{
		

		reqline[0] = strtok (mesg, " \t\n");			//Delimiting the message into segments
		
		reqline[1] = strtok (NULL, " \t");
		reqline[2] = strtok (NULL, " \t\n");
		
		//reqline[3] = strtok (NULL," \r\n\r\n");
		//printf("Message:%s",reqline[3]);	
		if ( strncmp(reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 ) 			//Checking request type and version
		{
			write(clients[n], bad_request,strlen(bad_request));			//Sending error code 400
		}
		else
		{	
			//printf("%s\n", second_copy);
			//fflush(stdout);
			if(strncmp( reqline[2], "HTTP/1.0", 8)==0)
			http_flag=1;
			if(strncmp( reqline[2], "HTTP/1.1", 8)==0)
			http_flag=0;

			if(strncmp(reqline[0],"POST\0",4)==0)				//Handling POST request
			{
			
				if ( strncmp(reqline[1], "/\0", 2)==0 )			//File name not mentioned to POST
				{
					send(clients[n],"File name not mentioned",23,0);
				}
				else 				//Reading POSTDATA
				{
					printf("%s %s %s\n",reqline[0],reqline[1],reqline[2]);
					post=strtok(NULL,"\r\n\r\n");
					printf("\nMessage from Client:%s",post);

				}
			}
	
			else if ( strncmp(reqline[0], "GET\0", 4)==0 )			//Handling GET request
			{
				printf("%s\n",second_copy);	
				fflush(stdout);
				if ( strncmp(reqline[1], "/\0", 2)==0 )			//If no file name is mentioned
				{
					//file_flag=1;
					reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...
				}
				//if((strncmp(reqline[1], "/images/wine.jpg", 9)==0) || (strncmp(reqline[1], "/images/exam.gif",9)==0) || (strncmp(reqline[1], "/images/apple_ex.png",9)==0) 
				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], reqline[1]);
				//printf("file: %s\n",path);
				while(path[path_count++])				//Parsing filename to ROOT
				{
					if(path[path_count]=='.')
					{
					type=path+(path_count);
					break;
					}	
				}
				
							
				type=extract(type);					//Extracting the content type
				
				
				if((file=fopen(path,"rb"))!=NULL)			//Computing the length of the file
				{
				while(fgetc(file)!=EOF);
				length=ftell(file);
				rewind(file);
				fclose(file);
				}
				else 
				{
				//printf("File not found");
				//fflush(stdout);
				write(clients[n], no_file,strlen(no_file)); 			//FILE NOT FOUND error 404
				}
				//printf("Length:%d",length);
				//printf("Type2:%s\n",type);
				if ( (fd=open(path, O_RDONLY))!=-1 )    		//FILE FOUND, sending header and Content
				{
					if(http_flag=1)
					send(clients[n], "HTTP/1.0 200 OK ", 17, 0);    //Sending Header code according to HTTP version
					else
					send(clients[n], "HTTP/1.1 200 OK ", 17, 0);
					
					//if(file_flag==0)
					//{
					/*if(http_flag=1)
					{
					sprintf(header,"%s","HTTP/1.0 200 OK\nfile:");
					http_flag=0;
					}
					else*/
					sprintf(header,"%s","Document Follows\n");
					//sprintf(header+strlen(header),"%s","File:"); 
					//sprintf(header+strlen(header),"%s\n",path);
					sprintf(header+strlen(header),"%s","Content Length : ");						
					sprintf(header+strlen(header),"%d\n",length);					
					sprintf(header+strlen(header),"%s","Content type : ");
					sprintf(header+strlen(header),"%s\n",type);
					//sprintf(header+strlen(header),"%s\n","Document Follows:\n");
					//write(clients[n], header, strlen(header));
					//printf("Header:%s",header);
					send(clients[n],header, strlen(header),0);		//Sending File Info
					//}

					while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )			//Sending File Content
						write (clients[n], data_to_send, bytes_read);
					
					file_flag=0;
					http_flag=0;
				}
			
			}
			else{
				//printf("Hello");
				//fflush(stdout);
				write(clients[n], no_support, strlen(no_support));		//Sending Error code 501
	    		    }
		}
	}
	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
}

char * extract(char * firstword)			//Function to extract the content type, port number and root directory
{
	
	char *word_found;
	//char *word_req;//[50];
	buffer=malloc(200);				//Malloc allocations
	word_found=malloc(50);
	word_req=malloc(50);
	int i=0;
	int j=0;
	FILE *file;
	bzero(buffer,sizeof(buffer));			//Nulling out all the buffers
	bzero(word_found,sizeof(word_found));
	bzero(word_req,sizeof(word_req));
	file = fopen("ws.conf","rb");			//Opening ws.conf to extract content type and all other information
	while((fgets(buffer,200,file))!=NULL)
	{
		if(strstr(buffer,"Directory")!=NULL)
		continue;

		if(strstr(buffer,firstword)!=NULL)
		{
		while(*(buffer++)!=' ');
		fclose(file);
		//printf("Buffer:%s\n",buffer);
		return buffer;				//Returning the base address of the extracted string
		}
	}
	fclose(file);
	return 0;
}







