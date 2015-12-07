#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 

#define FILE_CHUNK 1000
#define DATA_LENGTH 10
#define PACKET_LENGTH 50

struct xyz{
int x,y;
char data[10];

};

int computeChecksum(char str[])
{
   int ch,i;                       /* Each character read. */
   int computeChecksum = 0;             /* The computeChecksum mod 2^16. */
    
  for(i=0;i<sizeof(str);i++)
  {
    ch = str[i];
    computeChecksum = (computeChecksum >> 1) + ((computeChecksum & 1) << 15);
    computeChecksum += ch;
    computeChecksum &= 0xffff;       /* Keep it within bounds. */
  }
 //printf("\n%d",computeChecksum);
  return computeChecksum;
}
int main()
{
  FILE *fp1;
  int file_size;
  int sequence_flag =0;
  int totalsent = 0,len = 0;
  int sockfd = 0;
  char *buffer;
  int bytesread = 0,remainingBytes=0,fileByesRead=0;
   struct sockaddr_in servaddr,cliaddr;
  char errorMsg[50];
  char ackMessage[5];
  char packetData[PACKET_LENGTH];
  int readNextFileContent=1;
  char inputFileName[10];
  char outputFileName[10];
  char ipAddress[20];
  int portNo;
   struct xyz p1;
    //printf("%d",p1.x);
    //p1.seq_no=1;
    //strcpy(p1.checks,"TEST");
    //strcpy(p1.data,"TEST");



   //Accept the parameters from the user required for File transfer
  printf("\n*************************************************");
  printf("\n***********SERVER DEAMON FOR FTP*****************");
  
  printf("\nEnter the Input File Name");
  scanf("%s",inputFileName);

  printf("\nEnter the output File Name");
  scanf("%s",outputFileName);


printf("\nEnter the IP Address");
  scanf("%s",ipAddress);
printf("\nEnter the port NO");
  scanf("%d",&portNo);
  printf("\n*************************************************\n");

    
  fp1 = fopen(inputFileName, "r");
 
   if (fp1 == NULL) {
    sprintf(errorMsg,"%s:%s","\n Error in opening file. Please check your filename and try again.",inputFileName);
    write(1,errorMsg,strlen(errorMsg));
    exit(0);
  }
 
  if (fseek(fp1, 0 , SEEK_END) != 0) {
  /* Handle error */
  }
 
  file_size = ftell(fp1);
  if (file_size == -1) {
  /* Handle error */
  }
  
  fseek(fp1, 0, SEEK_SET);
 
  remainingBytes = file_size;
  
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
     printf("\n Error : Could not create socket \n");
     return 1;
  } 
  
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(ipAddress);
   servaddr.sin_port=htons(portNo);
 
  
   sendto(sockfd,outputFileName,strlen(outputFileName),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
   
  

  sleep(1);    
  while(remainingBytes > 0)
  {
    if(readNextFileContent)
    {
      
      if(remainingBytes > DATA_LENGTH)
	buffer = (char*) malloc(DATA_LENGTH);
      else
	buffer = (char*) malloc(remainingBytes);
      memset(buffer,' ',DATA_LENGTH);
      fileByesRead =fread(buffer, 1, DATA_LENGTH, fp1);
     // sprintf(packetData,"%d$%d$%s$",sequence_flag,computeChecksum(buffer),buffer);
      
    p1.x=sequence_flag;
    p1.y=computeChecksum(buffer);	
    strcpy(p1.data,buffer);

    }
    else
    {
      printf("\nRetransmitting Packet%d.",sequence_flag);
    }
    
    //sendto(sockfd,packetData,sizeof(packetData),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
 
    sendto(sockfd,(struct xyz *)&p1,sizeof(p1),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
   


    remainingBytes= remainingBytes - fileByesRead;
    
    len = sizeof(servaddr);
    bytesread = recvfrom(sockfd,ackMessage,6,0,(struct sockaddr *)&servaddr,&len);
    ackMessage[strlen(ackMessage)]= '\0';
    printf("\n%s",ackMessage);
    
    if((strcmp(ackMessage,"NACK0")==0) || (strcmp(ackMessage,"NACK1")==0))
    {
      readNextFileContent=0;
      remainingBytes= remainingBytes + fileByesRead;
    }
    else
    {
       if(sequence_flag)
	sequence_flag = 0;
      else
	sequence_flag = 1;
    
     free(buffer);
    readNextFileContent=1;
      
    }
   
    
    
  }

     p1.x=sequence_flag;
     p1.y=computeChecksum("END");	
     strcpy(p1.data,"END");
     sendto(sockfd,(struct xyz *)&p1,sizeof(p1),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
   
   // sendto(sockfd,"END\0",5,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
  
  fclose(fp1);
 printf("%dhere%d here %s",p1.x,p1.y,p1.data);
    
return 0;
}

