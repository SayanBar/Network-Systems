/*Author-Sayan Barman
Description:Client code*/

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
#include <errno.h>

#define MAXBUFSIZE 1025     //Size of each packet+packet sequence number



typedef struct A                                      //structure to send and receive data about file information
{	
	char command[10];
	char file_name[20];
	long int file_size; 
}A;
A file_info;

FILE *fp;
                             
int sockfd;                               //this will be our socket
int remote_length,from_length;
char buffer[MAXBUFSIZE];
struct hostent *hp;
//unsigned long int count=0;
int i=0;
//int size=1;
char ch;
struct sockaddr_in remote,from;              //"Internet socket address structure"
char file_name_client[20];
char command_client[10];

//Function protypes
int menu();			
int sendFile();
int recvFile();
void listFile();
void quit();
void encryption(char * file_Name);
void decryption(char* file_Name);


//Main function
int main (int argc, char * argv[])
{	
	

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	//remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	//hp=gethostbyname(argv[1]);
	inet_pton(AF_INET, argv[1], & remote.sin_addr);
	
		
	
	//bcopy((char *)hp->h_addr,(char *)&remote.sin_addr,hp->h_length);
	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("unable to create socket");
	}

	
	//char command[] = "apple";	
	remote_length=sizeof(struct sockaddr_in);
	from_length=sizeof(struct sockaddr_in);
	printf("Socket created\n");	
	
	while(1)
	{	
		memset(file_name_client,0,sizeof(file_name_client));
		memset(command_client,0,sizeof(command_client));
		if(menu()==1)
		break;

	}
	
	
}

void decryption(char* file_Name){						//decryption method to decrypt filename
	char character;		
	char secret = 'q'; //secret to XOR
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

//Menu to decide functionality according to command
int menu()
{
	int ch,n;
	remote_length=sizeof(remote);
	printf("UDP file transfer\n1.Get\n2.Put\n3.Delete\n4.Ls\n5.Exit\n");
	scanf("%d",&ch);		
	switch(ch)
	{	
		case 1:printf("Please enter the filename to receive\n");
		       scanf("%s",file_name_client);
		       bzero(&file_info,sizeof(file_info));
		       strcpy(file_info.command,"get");
		       strcpy(file_info.file_name,file_name_client);
		       file_info.file_size=0;
		       //printf("%s\n %s\n %d\n",file_info.command,file_info.file_name,file_info.file_size);
		       if(recvFile()==-1)
		       printf("\nFile does not exist\n");
		       break;
		case 2:printf("Please enter the filename to send\n");
		       scanf("%s",file_name_client);
		       bzero(&file_info,sizeof(file_info));
		       strcpy(file_info.command,"put");
		       strcpy(file_info.file_name,file_name_client);
		       if(sendFile()==-1)
		       printf("\nFile does not exist\n");
		       break;
		case 3:printf("Please enter the file name to delete\n");
		       scanf("%s",file_name_client);
		       bzero(&file_info,sizeof(file_info));
		       strcpy(file_info.command,"del");
		       strcpy(file_info.file_name,file_name_client);
		       file_info.file_size=0;
		       if((n=sendto(sockfd,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,remote_length))>0)
		       perror("Sending command");
		       //delFile();
		       break;
		case 4:printf("Getting list of files from the server\n");
		       bzero(&file_info,sizeof(file_info));
		       strcpy(file_info.command,"ls");
		       //strcpy(file_info.file_name,NULL);
		       file_info.file_size=0;
		       if((n=sendto(sockfd,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,remote_length))>0)
		       perror("Sending command");
		       listFile();
		       break;
		case 5:printf("Closing UDP socket\n");
		       bzero(&file_info,sizeof(file_info));
		       strcpy(file_info.command,"close");
		       quit();
		       return 1;
	       default:printf("Wrong choice.Try again\n");
		       return -1;
	}
	
}

void encryption(char * file_Name)							//Encyption method to encrypt file

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
	

int sendFile()   //Function to send the file
{	
	char packet_number=0;
	int nbytes=0;
	int n=0;
	long int file_size_client=0;
	long int offset=0;//////look at the type whether the size is compatible 
	int flag=0;
	int ack_byte;
	struct timeval timeout={1,0}; //set timeout for 60 msecs
	//char ack_confirm[10]="ACK#";
	//int ack_confirm_length=strlen(ack_confirm);
	char ack[20];
	long int size=0;
	long int count=0;
	encryption(file_name_client);
	if((fp=fopen(file_name_client,"rb"))==0)
	{
	perror("\nNo file:");
	return -1;
	}
	fseek(fp,0,SEEK_END);
	file_size_client=ftell(fp);
	rewind(fp);
	
	file_info.file_size=file_size_client;
	memset(buffer,0,MAXBUFSIZE);
	
	
	if((n=sendto(sockfd,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,remote_length))>0)
	perror("Sending command");
	//n=recvfrom(sock,buffer,strlen(file_name)+sizeof(file_size),0,(struct sockaddr *) &remote,&remote_length);

	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	while((size=(file_size_client-((sizeof(buffer)-1)*count)))>0)
	{	
		//if(count%10==0)
		//packet_number=0;
		//else
		packet_number=(char)(count%10);
		printf("Packet number:%d",packet_number);
		//sendto(sockfd,packet_number,sizeof(packet_number)
		if(size<(sizeof(buffer)-1))
		{	
			n=fread(buffer,1,size,fp);
			buffer[size]=packet_number;
			nbytes = sendto(sockfd,buffer,size+1,0,(struct sockaddr *)&remote,remote_length);
			fclose(fp);
			flag=1;
		}
		else
		{
			
			n=fread(buffer,1,(sizeof(buffer)-1),fp);
			buffer[MAXBUFSIZE-1]=packet_number;	
			nbytes = sendto(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&remote,remote_length);
			flag=0;
		}	
	bzero(buffer,sizeof(buffer));
	
	
	

	
	ack_byte=recvfrom(sockfd,ack,sizeof(ack),0,(struct sockaddr *)&remote,&remote_length);
	//if(strcmp(ack,ack_confirm)!=0)
	//printf("ACK#%ld hasn't been received yet.Waiting..",count);
	//else
	if(ack_byte>0)
	{
	printf("%s\n\n",ack);
	count++;
	//break;
	}
	else
	{
	//timeout={0,60000};
	//printf("ACK#%ld timed out.Sending again.",count);
	if(flag==0)
	{
		offset=ftell(fp)-(sizeof(buffer)-1);
		printf("not last packet\n");
		fseek(fp,offset,SEEK_SET);
		count--;
	}
	else
	{
		fp=fopen(file_name_client,"rb");
		printf("last packet\n");
		fseek(fp,-(size),SEEK_END);
		count--;
		flag=0;
	}
	}
	}
	//count=0;
	printf("\nFile:%s was sent successfully\n",file_name_client);
}

int recvFile()    //Function to receive the file
{
	int nbytes=0;
	char packet_number=0;
	int n=0;
	long int size=0;
	long int count=0;
	long int file_size_recv=0;
	char ack[20]="ACK#";

	int ack_length=strlen(ack);
	//fseek(fp,0,SEEK_END);
	//file_size_client=ftell(fp);
	//rewind(fp);
	//file_info.file_size=0;
	
	if((n=sendto(sockfd,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,remote_length))>0)
	perror("Sending command");
	bzero(&file_info,sizeof(file_info));
	
	if((n=recvfrom(sockfd,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,&remote_length))>0)
	perror("Receiving file info");	
	memset(buffer,0,MAXBUFSIZE);
	printf("Filename:%s\nFilesize:%ld\n",file_info.file_name,file_info.file_size);
	if((strcmp(file_info.command,"No"))==0)
	return -1;
	file_size_recv=file_info.file_size;
	
	//printf("Receive file size: %d",file_size_recv);
	fp=fopen(file_info.file_name,"ab");
	
	while((size=file_size_recv-((sizeof(buffer)-1)*count))>0)
	{
		packet_number=(char)(count%10);
		printf("Packet number:%d",packet_number);
		if(size<(sizeof(buffer)-1))
		{
			nbytes=recvfrom(sockfd,buffer,size+1,0,(struct sockaddr *)&remote,&remote_length);
			printf("Packet from server:%d\n",(int)buffer[size]);
			if(((int)(buffer[size]))!=((int)(packet_number)))
			{
				printf("Hello\n");
				count--;
				goto mismatch;
			}
			n=fwrite(buffer,1,size,fp);
			//printf("Received=%d Written=%d\n",nbytes,n);
			//count++;
			fclose(fp);
			decryption(file_info.file_name);
		}
		else
		{
			nbytes=recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&remote,&remote_length);
			printf("Packet from server:%d\n",(int)buffer[MAXBUFSIZE-1]);
			if(((int)(buffer[MAXBUFSIZE-1]))!=((int)(packet_number)))
			{
				printf("Hi\n");
				count--;
				goto mismatch;
			}
			n = fwrite(buffer,1,(sizeof(buffer)-1),fp);
		}
      mismatch:	bzero(buffer,sizeof(buffer));
		sprintf(ack+ack_length,"%ld",count);
		count++;
		//printf("\n%s\n",ack);
		sendto(sockfd,ack,sizeof(ack),0,(struct sockaddr *)&remote,remote_length);
		//printf("\nAck sent\n");
		
	}
	//count=0;
	printf("\nFile:%s was received successfully\n",file_info.file_name);
}

void quit()   //Function to close the socket
{
	remote_length=sizeof(remote);
	sendto(sockfd,&file_info,sizeof(file_info),0,(struct sockaddr *)&remote,remote_length);
	close(sockfd);
	printf("\nSocket closed\n");
}

void listFile()  //Function to display ls
{
	char namelist[500];
	remote_length=sizeof(remote);
	recvfrom(sockfd,namelist,sizeof(namelist),0,(struct sockaddr *)&remote,&remote_length);
	printf("\n%s",namelist);
}

