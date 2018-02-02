/*Author:Sayan Barman
Description:Creating a proxy server that manages multiple clients*/


#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <openssl/md5.h>
#include <time.h>
#include <sys/socket.h>  
#include <sys/types.h>
#include <sys/stat.h>  
#include <resolv.h>  
#include <string.h>  
#include <stdlib.h>  
#include <pthread.h>  
#include <unistd.h>  
#include <netdb.h> //hostent  
#include <arpa/inet.h>  
//#include "common.h"

#define MAX 1024
#define CLIMAX 100

int client_fd[CLIMAX];
int proxy_fd;

char * getCacheFile(char *md5value);
void cacheFile(char *md5value,char *buffer);
char * md5sum(char * fileName);
void setupProxy(char* proxy_port);
uint8_t hostname_to_ip(char *hostname,struct sockaddr_in *);
char * getHostname(char *url);
uint8_t checkBlocked(char *hostname);
void writeDNScache(char *hostname,struct sockaddr_in *ipDNS);
char * getDNSCache(char *hostname);
int timeout(char *filename);
//struct sockaddr_in server_sd;
//struct sockaddr_in oldServer;


 
void runSocket(int slot)  
{ 
   
	int server_fd =0,bytes =0,i=0,addr,slashDot=0; char asciicode[32];
	char buffer[MAX];char md5[MD5_DIGEST_LENGTH];
	unsigned int len=sizeof(buffer);    
	struct sockaddr_in ipDNS;
	struct addrinfo hint,*p,*serverInfo;   
	char *hostname=malloc(100);
	char command[20],url[1000],httpVersion[20];
   	
	printf("client:%d\n",client_fd[slot]);    
        memset(&hint, 0, sizeof(hint));
   	hint.ai_family=AF_INET;
	hint.ai_socktype=SOCK_STREAM;
	// create a socket  
   	  
   	memset(buffer, '\0', sizeof(buffer));
	
   	printf("hello0");
   	int loopCount=0;
   	char filename[500];
      
        bytes = recv(client_fd[slot], buffer, sizeof(buffer),0);
      	if(bytes<0)
	fprintf(stderr,("recv error\n"));
      	else if(bytes==0)
	fprintf(stderr,("Client disconnected\n"));
      	else
      	{
   		//loopCount++;
		char errBuf[MAX];
		memset(&errBuf,'\0',sizeof(errBuf));
	   	sscanf(buffer,"%s %s %s",command,url,httpVersion);
		//Generating md5 value
		char *webpath=url+7;
		printf("\nwebpath:%s\n",webpath);
		//char * md5value=md5sum(webpath);
		// *md5value_1=md5value;
		//md5value=filename;
		//filename=md5value;
		//strcpy(filename,md5value);
		//char *webpath_1=webpath;
		//strcpy(filename,webpath);
		char *hash=(char *)MD5((unsigned char*)webpath,strlen(webpath),(unsigned char*)md5);
		hash=asciicode;
		for(int i =0; i<16;i++){
                hash += sprintf(hash,"%0x",md5[i]);
            	}
            	printf("\nThe hash value for %s is %s \n",webpath,asciicode);

		/*while(filename[slashDot]!='\0'){
			if(filename[slashDot]=='/'){
				filename[slashDot]='.';}
			slashDot++;
		}*/
		strcpy(filename,asciicode);
		printf("\nFilename:%s\n",filename);
		//400 Bad Request Implemented
		if((strcmp(command,"GET")!=0) || ((strcmp(httpVersion,"HTTP/1.0")!=0) && (strcmp(httpVersion,"HTTP/1.1")!=0))){
			printf("\nBad Request\n");
			sprintf(errBuf,"%s","HTTP/1.1 400 Bad Request\n\n<html><body>Error 400 Bad Request</body></html>");
		write(client_fd[slot], errBuf, sizeof(errBuf));
		goto socketStop;
	   	}
	   	
		//Printing Buffer to check Request
		printf("hello1");
	   	printf("\nBuffer-%s\n",buffer);
	   	hostname=getHostname(url);
		memset(&errBuf,'\0',sizeof(errBuf));
		//int webValid;
	   	printf("\nHostname:%s\n",hostname);fflush(stdout);
		//403 Forbidden Request
		if(!(checkBlocked(hostname))){
			sprintf(errBuf,"%s","HTTP/1.1 403 Forbidden\n\n<html><body>Error 403 Forbidden</body></html>");
			write(client_fd[slot], errBuf, sizeof(errBuf));
	   		goto socketStop;
		}

	   	printf("hello2");
           	uint8_t DNSqueryFlag=0;
		FILE *fp;
		//int time_t=(int)(timeout(filename));
		//printf("\nTimeout:%d\n",time_t);
		if((fp=fopen(filename,"rb"))==NULL) //|| (((int)(timeout(filename)))>150))	
			DNSqueryFlag=1;
		if(DNSqueryFlag==1){
			//printf("\nTimeout may have exceeded if file is already cached. Timeout:%d\n",(int)(timeout(filename)));
cachetimeout:		remove(filename);
			char * serverIP=getDNSCache(hostname);
			//printf("%s",serverIP);
			
			if(serverIP==NULL){
				printf("\nWriting NEW DNS QUERY\n");
				if((addr=getaddrinfo(hostname,"80",&hint,&serverInfo))!=0){
					fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(addr));
					exit(0);
				}  
	        	for(p=serverInfo;p!=NULL;p=p->ai_next){
				if((server_fd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
					perror("Socket creation failed");
					continue;
		   		}
				if((connect(server_fd, p->ai_addr, p->ai_addrlen))<0){  
           				perror("server connection not established");
					close(server_fd);			
					continue;  
      	   			}
				memcpy(&ipDNS,p->ai_addr,sizeof(ipDNS));
			
				
				
			
				printf("server socket connected hello4\n");
				if(!(checkBlocked(inet_ntoa(ipDNS.sin_addr)))){
					sprintf(errBuf,"%s","HTTP/1.1 403 Forbidden\n\n<html><body>Error 403 Forbidden</body></html>");
					write(client_fd[slot], errBuf, sizeof(errBuf));
	   				goto socketStop;
				}
				writeDNScache(hostname,&ipDNS);
				break;
			}
			if(p==NULL){
				fprintf(stderr,"Could not connect\n");
				exit(0);
			}
			//memcpy(&ipDNS,p->ai_addr,sizeof(ipDNS));		
			freeaddrinfo(serverInfo);
		}
		else{
			printf("\nCACHED DNS query extracted\n");
			ipDNS.sin_family = AF_INET;  
        		ipDNS.sin_port = htons(80);  
        		ipDNS.sin_addr.s_addr = inet_addr(serverIP);
				
			if(!(checkBlocked(inet_ntoa(ipDNS.sin_addr)))){
				sprintf(errBuf,"%s","HTTP/1.1 403 Forbidden\n\n<html><body>Error 403 Forbidden</body></html>");
				write(client_fd[slot], errBuf, sizeof(errBuf));
	   			goto socketStop;
			}
				
			if((server_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
				perror("Socket creation failed");
				exit(0);
		   	}
			if((connect(server_fd, (struct sockaddr *)&ipDNS, sizeof(ipDNS)))<0){  
           			perror("server connection not established");
				close(server_fd);
				/*shutdown(client_fd[slot],SHUT_RDWR);  
   				close(client_fd[slot]);
   				close(server_fd);
   				client_fd[slot]=-1;*/
				exit(0);  
      	   		} 	
          	}
				
			    
		//Cached or uncached
		//if not cached
		//Sending request to the server and receiving
		//char 
		//FILE *fp;
		//if((fp=fopen(filename,"r"))==NULL)
		//{
			printf("\nNo cache. Requesting the server to cache new files\n");
           		send(server_fd, buffer,strlen(buffer),0);
	   		//printf("\nWritten request bytes:%d\n",x);   
           		printf("hello5"); fflush(stdout);  
           		memset(buffer, '\0', sizeof(buffer));
	   		printf("hello6");fflush(stdout);
	   		int readBytes=0,writtenBytes=0;int readBytesTotal=0;
			FILE *fpe=fopen(filename,"wb+");
           		while((readBytes = recv(server_fd, buffer, sizeof(buffer),0))>0){
	        		printf("hello7");
				//md5sum(buffer);
				//cache file
				//cacheFile(filename,buffer);
				fwrite(buffer,1,readBytes,fpe);
				
	        		readBytesTotal+=readBytes;
                		send(client_fd[slot], buffer, readBytes,0);
				memset(buffer, '\0', sizeof(buffer));
           		}
			fclose(fpe);
	   		printf("\nRetrieved bytes from server:%d\n",readBytesTotal);fflush(stdout);
			printf("\nRequested file received and sent to client\n");fflush(stdout);
		}//else get cached file
		else{	//getCacheFile()
			char timeoutBuffer[100]="HTTP/1.1 404 Not Found\n\n<html><body>Cache Timeout</body></html>\n";
			printf("\nGetting cached files.Checking Timout-\n");//%d\n",(int)(timeout(filename)));
			//int time_t=timeout(filename);
			int *time_t=(int)(timeout(filename));
			printf("\nTimeout:%d\n",(int)(timeout(filename)));
			printf("\nTimeout:%d\n",time_t);
			if(time_t>150){
				write(client_fd[slot],timeoutBuffer,sizeof(timeoutBuffer));
				printf("\nCache Timeout\n");
				goto cachetimeout;
			}
			fseek(fp,0,SEEK_END);
			int fileSize=ftell(fp);
			printf("\nFilesize:%d\n",fileSize);fflush(stdout);
			rewind(fp);
			//fclose(fp);
			memset(buffer, '\0', sizeof(buffer));
			//struct stat fileInfo;
			//if(stat(filename,&fileInfo)==-1){
			//	printf("\nCouldn't find file/filesize. Exiting\n");
			//	exit(0);
			//}
			//int fileSize=fileInfo.st_size;
			int readbytes=0,Totalreadbytes=0;//char *buf=malloc(1024);//buf=NULL;
			int z=0;char cacheBuffer[1024];
			while(Totalreadbytes!=fileSize){
				//buf=getCacheFile(filename);
				//buffer=NULL;
				memset(cacheBuffer,'\0',sizeof(cacheBuffer));
				printf("\nValue of z:%d\n",z);
				fseek(fp,z*1024,SEEK_SET);
				readbytes=fread(cacheBuffer,1,1024,fp);
				printf("\nReadBytes in func:%d\n",readbytes);
				//printf("\nBuffer:%s\n",cacheBuffer);
				//strcpy(buffer,buf);
				//if((strcmp(buf,buffer))==0)
				//	printf("\nCache buffer correct\n");
				write(client_fd[slot], cacheBuffer, 1024);
				printf("Cache%d Bytes sent:%d\n",z,strlen(cacheBuffer));				
				z++;
				Totalreadbytes+=readbytes;//+strlen(cacheBuffer);
				printf("\nTotal bytes sent:%d\n",readbytes);
				//memset(buffer, '\0', sizeof(buffer));
			}
			fclose(fp);
			printf("\nTotal bytes sent:%d\n",Totalreadbytes);
			printf("\nCached file transfer complete\n");
			shutdown(client_fd[slot],SHUT_RDWR);  
   			close(client_fd[slot]);
		}
	}
	socketStop:  	printf("Socket closing");
   			shutdown(client_fd[slot],SHUT_RDWR);  
   			close(client_fd[slot]);
   			close(server_fd);
   			client_fd[slot]=-1;
 }  



 // main entry point  
 int main(int argc,char *argv[])  
 {  
	char port[100],ip[100];  
	char *hostname = argv[1];  
	char proxy_port[100];  
	int slot=0;   
	strcpy(proxy_port,argv[1]); // proxy port  
          
	printf("proxy port is %s\n",proxy_port);        
        
	//socket variables  
	//int proxy_fd =0; //client_fd[slot]=0;  
	struct sockaddr_in client_sd;  
 	

	  
        setupProxy(proxy_port);    
	printf("waiting for connection..\n");  
     
	int clientInit;
	for(clientInit=0;clientInit<CLIMAX;clientInit++)
		client_fd[clientInit]=-1;
	socklen_t c = sizeof(client_sd);
	while(1)  {
	
	client_fd[slot] = accept(proxy_fd, (struct sockaddr*)&client_sd, &c);
        puts("Connection accepted");

    
	if(client_fd[slot]<0){
		perror("accept() error");
	}
	else
	{
		if(fork()==0)
		{
			runSocket(slot);
			exit(0);
		}
	}
	while(client_fd[slot]!=-1){
		slot=(slot+1)%CLIMAX;
	}
     }
      return 0;  
 }  



/*uint8_t hostname_to_ip(char *hostname)
{
    //struct sockaddr_in serveraddr;
    struct hostent *server;
    char *line=NULL;
    size_t len=0; 
    ssize_t read;
   
    server = gethostbyname(hostname);
    //printf("%s",server->h_addr[0]);
    if (server == NULL) {
        herror("gethostbyname"); 
        return -1;
    }
    
    /* build the server's Internet address */
    /*bzero((char *) &server_sd, sizeof(server_sd));
    server_sd.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&server_sd.sin_addr.s_addr, server->h_length);
    printf("\nIP-%x\n",server_sd.sin_addr.s_addr); 
    server_sd.sin_port = htons(80);
    return 1;
}*/


void setupProxy(char* proxy_port)
{
	struct sockaddr_in proxy_sd;
	
	if((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
        {  
          perror("\nFailed to create socket");  
        }  
        printf("Proxy created\n"); 
       
	
        memset(&proxy_sd, 0, sizeof(proxy_sd));  
        int optval=1;
        setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));
        
      
	// set socket variables  
        proxy_sd.sin_family = AF_INET;  
        proxy_sd.sin_port = htons(atoi(proxy_port));  
        proxy_sd.sin_addr.s_addr = inet_addr("127.0.0.1");  
      
	// bind the socket  
        if((bind(proxy_fd, (struct sockaddr*)&proxy_sd,sizeof(proxy_sd))) < 0)  
        {  
           perror("Failed to bind a socket");  
        }  
      
	// start listening to the port for new connections  
        if((listen(proxy_fd,CLIMAX)) < 0)  
        {  
           perror("Failed to listen");  
        }  
}


char * getHostname(char *url)
{	
	int i=0;
	char *hostname=malloc(100);
	hostname=strchr(url,'/');
	hostname+=2;
	while(*(hostname+i)!='/')
		i++;	
	*(hostname+i)='\0';
	printf("Hostname:%s\n",hostname);
	fflush(stdout);
	return hostname;
}

uint8_t checkBlocked(char *hostname)
{
	char *line=NULL;
	size_t len = 0;
	ssize_t read=0;
	//printf("Block ended1\n");
	FILE *file=fopen("Blocked.conf","rb");
	while((read=getline(&line,&len,file))>0){
		if(strstr(line,hostname)!=NULL){
			printf("\nBlocked Website:%s\n",hostname);
			fclose(file);
			return 0;
			}
	//printf("Block ended2\n");
	}
	fclose(file);
	return 1;
	//printf("Block ended3\n");
}

void writeDNScache(char *hostname,struct sockaddr_in *ipDNS)
{
	char a[100];//compareHostname[50];
	char *line=NULL;
	size_t len = 0;
	uint8_t flag=0;
	ssize_t read=0;

	memset(&a,'\0',sizeof(a));
	memset(&line,'\0',sizeof(line));
	
	
	sprintf(a,"%s %s\n",hostname,inet_ntoa(ipDNS->sin_addr));
	printf("Connected to hostname:%s at ip:%s and port:%hd\n",hostname,inet_ntoa(ipDNS->sin_addr),ntohs(ipDNS->sin_port));
	
	FILE *file=fopen("DNSCache.conf","rb+");
	while((read=getline(&line,&len,file))>0){
		if((strstr(line,hostname))!=NULL){
			printf("Hello in cache1.2\n");fflush(stdout);	
			flag=1;
			break;
		}
		else{
			memset(&line,'\0',sizeof(line));
			continue;
		}
	}
	if(flag==0){
		fseek(file,0,SEEK_END);
		fwrite(a,1,strlen(a),file);
	}
	fclose(file);
}

char * getDNSCache(char *hostname)
{
	char *a=malloc(100);
	char *line=NULL;
	size_t len = 0;
	ssize_t read=0;
	FILE *file=fopen("DNSCache.conf","rb");
	while((read=getline(&line,&len,file))>0){
		if(strstr(line,hostname)!=NULL){
			sscanf(line,"%*s %s",a);
			//printf("DNS cached ip:%s",a);
			fclose(file);
			return a;
		}
	}
	fclose(file);
	return NULL;
	
}

char * md5sum(char * weburl)
{
	unsigned char md5res[MD5_DIGEST_LENGTH];
    	int i;
	//weburl="Hello";
	//static int j=0;
	//char *filename;
	//memset(filename,'\0',sizeof(filename));
	//sprintf(filename,"%d.%s",j,".html");
    	//FILE *file = fopen (filename, "ab+");
    	MD5_CTX mdContext;
    	int bytes;
    	unsigned char data[MAX];
    	unsigned char bufFInal[MAX];
    	memset(data, '\0', MAX);
    	memset(bufFInal, '\0', MAX);
	strcpy(data,weburl);
		printf("\nWeburl in function:%s\n",data);
    	//if (file == NULL) {
        //	printf ("%s can't be opened.\n",filename);
        //	return 0;
    	//}
	printf("md5-1");fflush(stdout);
    	MD5_Init (&mdContext);
    	//while ((bytes = fread (data, 1, MAX, file)) != 0)
	printf("md5-2");fflush(stdout);
        MD5_Update (&mdContext, data, strlen(data));
	printf("md5-3");fflush(stdout);
    	MD5_Final (md5res,&mdContext);
	printf("md5-4");fflush(stdout);
    	for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
        	char buf[MAX];
		sprintf(buf,"%2x", md5res[i]);
		strcat(bufFInal,buf); 
    	}
	
	//fclose (file);
	printf("\nmd5sum in func: %s\n", bufFInal);
	//file=fopen(bufFinal,
    	
    	
    	
    	//j++;
	char * returnVal=bufFInal;
	strcpy(returnVal,bufFInal);
    	return (bufFInal);
}

int timeout(char *filename)
{
	time_t timeNow;
	struct stat fileInfo;
	if(stat(filename,&fileInfo)==-1)
		return -1;
	timeNow=time(NULL);
	int diff= timeNow-fileInfo.st_mtime;
	return (int)diff;//(timeNow-fileInfo.st_mtime);
}

void cacheFile(char *md5value,char *buffer)
{
	FILE *fp=fopen(md5value,"ab+");
	fwrite(buffer,1,strlen(buffer),fp);
	fclose(fp);
}


char * getCacheFile(char *md5value)
{	
	static int i=0;
	char cacheBuffer[1024];
	//buffer=NULL;
	memset(cacheBuffer,'\0',sizeof(cacheBuffer));
	printf("\nValue of i:%d\n",i);
	FILE *fp=fopen(md5value,"a+");
	fseek(fp,i*1024,SEEK_SET);
	int readBytes=fread(cacheBuffer,1,sizeof(cacheBuffer),fp);
	printf("\nReadBytes in func:%d\n",readBytes);
	i++;
	fclose(fp);
	char *returnCache=cacheBuffer;
	return returnCache;
}

