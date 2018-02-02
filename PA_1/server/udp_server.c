/*Author-Sayan Barman
Description:Server code*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
/* You will have to modify the program below */

#define MAXBUFSIZE 1025

typedef struct A
{	
	char command[10];
	char file_name[20];
	long int file_size; 
}A;
A file_info;

int process(A file);
int sendFile();
void recvFile();
void delFile();
void getList();
void quit();
void encryption(char * file_Name);
void decryption(char* file_Name);


char command_serv[10];
char file_name_serv[20];

long int file_size_serv=0;
FILE *fp;
int sock;                           //This will be our socket
struct sockaddr_in remote;     //"Internet socket address structure"
unsigned int remote_length;         //length of the sockaddr_in structure

char buffer[MAXBUFSIZE];             //a buffer to store our received message
//struct file_info *file

int main (int argc, char * argv[] )
{
	//struct sockaddr_in remote;     //"Internet socket address structure"	
	
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&remote,sizeof(remote));                    //zero the struct
	remote.sin_family = AF_INET;                   //address family
	remote.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	remote.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("unable to create socket");
	}

	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0)
	{
		printf("unable to bind socket\n");
	}

	remote_length = sizeof(remote);
	//char msg[] = "orange";
	printf("\nSocket created\n");
	//int count=0;

	while(1)
	{
		//bzero(&file_info,sizeof(file_info));
		//bzero(command_serv,sizeof(command_serv));
		//bzero(file_name_serv,sizeof(file_name_serv));
		//file_size_serv=0;
		int n=recvfrom(sock,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,&remote_length);
		if(process(file_info)==-1)
		continue;
			
	}
}

void decryption(char* file_Name){						//decryption
	char character;		
	char secret = 'q';
	FILE * fq;
	fq = fopen(file_Name,"rw");
	character = getc(fq);							
	while(character!=EOF)

	{
		fputc(character^secret,fq);
		character = fgetc(fq);
	}
	fclose(fq);

}

void encryption(char * file_Name)							//exor file charactertion

{
	char character;
	char secret = 'q';
	FILE * fq;
	fq = fopen(file_Name,"rw");
	character = getc(fq);	
	while(character!=EOF)
	{
		fputc(character^secret,fq);			//encryption
		character = fgetc(fq);
	}
	fclose(fq);
}

int process(A file)
{	
	//decryption(file.file_name);
	strcpy(command_serv,file.command);
	strcpy(file_name_serv,file.file_name);
	file_size_serv=file.file_size;
	//printf("%s\n %s\n %d\n",file.command,file.file_name,file.file_size);
	//printf("%s\n %s\n %d\n",command_serv,file_name_serv,file_size_serv);
	if(strcmp(command_serv,"get")==0)
	{
	if(sendFile()==-1)
	return -1;
	}
	if(strcmp(command_serv,"put")==0)
	recvFile();
	if(strcmp(command_serv,"del")==0)
	delFile();
	if(strcmp(command_serv,"ls")==0)
	getList();
	if(strcmp(command_serv,"close")==0)
	quit();
}


void recvFile()         //Function to receive the file
{	
	long int count=0;
	char packet_number=0;
	long int pos=0;
	long int size=0;
	int nbytes=0;
	int n=0;
	int n1=0;                        //number of bytes we receive in our message
	char ack[20]="ACK#";
	int ack_length=strlen(ack);
	remote_length = sizeof(remote);
	memset(buffer,0,MAXBUFSIZE);
	printf("Filename:%s\nFilesize:%ld\n",file_name_serv,file_size_serv);
	if((fp=fopen(file_name_serv,"wb"))<0)
	perror("All wrong");
	//printf("Hi");
	while((size=file_size_serv-((sizeof(buffer)-1)*count))>0)
	{
		packet_number=(char)(count%10);
		printf("Packet number:%d\n",packet_number);
		if(size<(sizeof(buffer)-1))
		{
			//printf("Hello2");			
			nbytes=recvfrom(sock,buffer,size+1,0,(struct sockaddr *)&remote,&remote_length);
			//printf("Hello2");			
			printf("Packet from client:%d\n",(int)buffer[size]);			
			if(((int)(buffer[size]))!=((int)(packet_number)))
			{
				printf("Hello\n");
				count--;
				goto mismatch;
			}
			//buffer[strlen(buffer)]='\0';
			n=fwrite(buffer,1,size,fp);
			
			//printf("Received=%d Written=%d\n",nbytes,n);
			//count++;
			fclose(fp);
			decryption(file_name_serv);
		}
		else
		{
			//printf("Hello1");			
			n=recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *)&remote,&remote_length);
			
			printf("Packet from client:%d\n",(int)buffer[MAXBUFSIZE-1]);
			if(((int)(buffer[MAXBUFSIZE-1]))!=((int)(packet_number)))
			{
				printf("Hi\n");
				count--;
				goto mismatch;
			}
			//buffer[strlen(buffer)]='\0';
			n1 = fwrite(buffer,1,(sizeof(buffer)-1),fp);
			//bzero(buffer,sizeof(buffer));
			//printf("Received=%d Written=%d\n",n,n1);
			//count++;
			//printf("count %d\n",count);
		}
      mismatch :bzero(buffer,sizeof(buffer));
		sprintf(ack+ack_length,"%ld",count);
		count++;
		//printf("\n%s\n",ack);
		sendto(sock,ack,sizeof(ack),0,(struct sockaddr *)&remote,remote_length);
	}
	//count=0;
	printf("\nFile:%s was received successfully\n",file_name_serv);
}

int sendFile()      //Function to send the file
{
	char ack[20];
	char packet_number=0;
	long int count=0;
	long int size=0;
	long int offset=0;
	int nbytes=0;
	int n=0;
	int n1=0;                        //number of bytes we receive in our message
	int flag=0;
	int ack_byte;
	long int file_size_send=0;
	struct timeval timeout={0,60000}; //set timeout for 60 msecs
	remote_length = sizeof(remote);
	bzero(&file_info,sizeof(file_info));
	encryption(file_name_serv);
	if((fp=fopen(file_name_serv,"rb"))==0)
	{
	strcpy(file_info.command,"No");
	flag=1;
	}
	else
	{	
	fseek(fp,0,SEEK_END);
	file_size_send=ftell(fp);
	//printf("File Name:%s",file_name_serv);
	//printf("File size:%d",file_size_send);
	rewind(fp);
	memset(buffer,0,MAXBUFSIZE);
	}
	
	strcpy(file_info.file_name,file_name_serv);
	file_info.file_size=file_size_send;
	if((n=sendto(sock,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,remote_length))>0)
	perror("Sending command");
	if(flag==1)
	return -1;
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	//n=recvfrom(sock,buffer,strlen(file_name)+sizeof(file_size),0,(struct sockaddr *) &remote,&(sizeof(remote)));
	while((size=(file_size_send-((sizeof(buffer)-1)*count)))>0)
	{
		packet_number=(char)(count%10);
		printf("Packet number:%d",packet_number);
		if(size<(sizeof(buffer)-1))
		{	
			n=fread(buffer,1,size,fp);
			buffer[size]=packet_number;
			//sprintf(buffer+strlen(buffer),"%c",(char)(count%10));
			nbytes = sendto(sock,buffer,size+1,0,(struct sockaddr *)&remote,remote_length);
			//printf("Read=%d Sent=%d\n",n,nbytes);
			fclose(fp);
			flag=1;
		}
		else
		{
			n=fread(buffer,1,(sizeof(buffer)-1),fp);
			buffer[MAXBUFSIZE-1]=packet_number;
			//sprintf(buffer+strlen(buffer),"%c",(char)(count%10));
			nbytes = sendto(sock,buffer,sizeof(buffer),0,(struct sockaddr *)&remote,remote_length);
			//printf("Read=%d Sent=%d\n",n,nbytes);	
		}	
	bzero(buffer,sizeof(buffer));
	count++;
	//recvfrom(sock,ack,sizeof(ack),0,(struct sockaddr *)&remote,&remote_length);
	//printf("%s\n\n",ack);
	
	ack_byte=recvfrom(sock,ack,sizeof(ack),0,(struct sockaddr *)&remote,&remote_length);
	//if(strcmp(ack,ack_confirm)!=0)
	//printf("ACK#%ld hasn't been received yet.Waiting..",count);
	//else
	if(ack_byte>0)
	printf("%s\n\n",ack);
	else
	{
	printf("ACK#%ld timed out.Sending again.",count);
	if(flag==0)
	{
		offset=ftell(fp)-(sizeof(buffer)-1);
		fseek(fp,offset,SEEK_SET);
		count--;
		printf("Hi1");
	}
	else
	{
		fp=fopen(file_name_serv,"rb");
		fseek(fp,-(size),SEEK_END);
		count--;
		flag=0;
		printf("Hi2");
	}
	}
	}
	//fclose(fp);
	printf("\nFile:%s was sent successfully\n",file_name_serv);

}

void delFile()
{	
	//printf("\nHello delete\n");
	printf("\nFile:%s was deleted successfully",file_name_serv);//,file_name_serv);
	fflush(stdout);
	remove(file_name_serv);
	
}

void getList()        //Function to display ls
{
	struct dirent **namelist;
	char buffer_list[500];
	remote_length=sizeof(remote);
	int no_files=0;	
	if((no_files=scandir(".",&namelist,NULL,alphasort))<0)
	perror("scandir");
	bzero(buffer_list,sizeof(buffer_list));
	while (--no_files>1)
	{
		//printf("\n %s \n",namelist[no_files]->d_name);
		sprintf(buffer_list+strlen(buffer_list),"%s\n",namelist[no_files]->d_name);
		free(namelist[no_files]);
		//no_files--;
	}
	//printf("\n%s",buffer_list);
	sendto(sock,buffer_list,sizeof(buffer_list),0,(struct sockaddr *)&remote,remote_length);
}

void quit()    //Function to close the socket
{	
	//printf("\nHello\n");
	//sleep(10);
	//int x;
	shutdown(sock,2);
	printf("\nSocket closed\n");
	exit(-1);
}

