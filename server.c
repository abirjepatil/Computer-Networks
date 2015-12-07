#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>

#define FILE_CHUNK 1000
#define DATA_LENGTH 10
#define FILE_WRITE_CHUNK 5

#define PACKET_LENGTH 50

#define ACK0 "ACK0"
#define ACK1 "ACK1"
#define NACK0 "NACK0"
#define NACK1 "NACK1"


struct xyz{
int x,y;
char data[10];
};

FILE *fpOut;
//function to check if the checksum matches
int ChecksumCompare(int checksum,int inputChecksum)
{
  if(checksum == inputChecksum)
    return 1;
  else
    return 0;
  
}
//function to check if the states match
int SequenceMatch(int packetseqno,int inputPacketSeq)
{
  if(packetseqno == inputPacketSeq)
    return 1;
  else
    return 0;
  
}




//function to split checksum


int splitPacketStructure(struct xyz a,int *packet_seq,int *randomChecksumGen)
{

//printf("in this function");


  int isChecksumCompare,isSequenceValid;

	
  int dataSize=0;
  char *seqNumber,*inputChecksum,*fileData;
  //seqNumber = strtok(packet_data, "$");
  //inputChecksum = strtok(NULL, "$");
 // fileData = strtok(NULL, "$");
//sprintf(seqNumber,"%d",a.x);
//sprintf(inputChecksum,"%d",a.y);        
//strcpy(fileData,a.data);  

printf("\n%d %d %s\n",a.x,a.y,a.data);
//simulates a checksum error 
  if((*randomChecksumGen%10)!=0)
{
    isChecksumCompare = ChecksumCompare(a.y,checksum(a.data));
	//printf("\nCorrect Checksum!!");
} 
 else
{
     isChecksumCompare = ChecksumCompare(a.y,checksumRandom(a.data));
	//printf("\nIncorrect Checksum. Request for retransmission!"); 
} 
//checks if the sequence Number is valid
  isSequenceValid = SequenceMatch(a.x,*packet_seq);
  
  //printf("\n%d %d",isChecksumCompare,isSequenceValid);
  //if the checksum and the sequence number match
  if(isChecksumCompare && isSequenceValid)
  {
    //change the state if 0 make it 1 and vice versa
    if(*packet_seq == 0)
      *packet_seq = 1;
    else
      *packet_seq = 0;
    
    //calculate the size of the packet
    dataSize = strlen(a.data);
    //copy it byte by byte in the file
     if(dataSize <= 5)
      {
	fwrite(a.data,1,dataSize,fpOut);
	
      }
      else
      {
	fwrite(a.data,1,FILE_WRITE_CHUNK,fpOut);
	fwrite(&a.data[FILE_WRITE_CHUNK],1,(dataSize-FILE_WRITE_CHUNK),fpOut);
      }
      return 0;
   
  }
  else
    return 1;
 
  
  
}























//function to calculate the checksum .. Used directly from wikipedia
int checksum(char str[])
{
   int ch,i;                       /* Each character read. */
   int checksum = 0;             /* The checksum mod 2^16. */
    
  for(i=0;i<sizeof(str);i++)
  {
    ch = str[i];
    checksum = (checksum >> 1) + ((checksum & 1) << 15);
    checksum += ch;
    checksum &= 0xffff;       /* Keep it within bounds. */
  }
 //printf("\n%d",checksum);
  return checksum;
}

//random number generator
int checksumRandom(char str[])
{
  
  return rand();
}
void main()
{
  int randomChecksumGen=0;
  int sockfd;
  int packet_seq =0;
  int listenfd = 0,connfd = 0;
   struct sockaddr_in servaddr,cliaddr;
  char outputFileName[30];
  int totalsent = 0;
  char packetData[50];
  int bytesread=0,len=0,isRetransmit=0;
  int PORT_NO;
  struct xyz p1;
   

 //Accept the parameters from the user required for File transfer
  printf("\n*************************************************");
  printf("\n***********SERVER DEAMON FOR FTP*****************");
  
  printf("\nEnter the Port Number to listen for socket requests");
  scanf("%d",&PORT_NO);

  printf("\n*************************************************\n");


 //create a socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT_NO);
  
  bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
  len = sizeof(cliaddr);
  //get the file name from the client
  bytesread = recvfrom(sockfd,outputFileName,10,0,(struct sockaddr *)&cliaddr,&len);
  

//getche();
   outputFileName[bytesread] = '\0';
  //open or create  a new file
  fpOut = fopen(outputFileName,"w");
  while(1)
  {
      len = sizeof(cliaddr);
      //get data from the client 
      //bytesread = recvfrom(sockfd,packetData,PACKET_LENGTH,0,(struct sockaddr *)&cliaddr,&len);

     
      bytesread = recvfrom(sockfd,(struct xyz *)&p1,sizeof(p1),0,(struct sockaddr *)&cliaddr,&len);
      totalsent = totalsent+bytesread;
 

      //if it is the last packet then exit  
      if(strcmp(p1.data,"END")==0)
	exit(0);
     //check for checksum. if it is correct or incorrect

     // isRetransmit = splitPacket(packetData,&packet_seq,&randomChecksumGen);
      // printf("%dhere%d here %s\n",p1.x,p1.y,p1.data);

      isRetransmit = splitPacketStructure(p1,&packet_seq,&randomChecksumGen);
      
      //send NACK0 or NACK1 depending upon the state
      if(isRetransmit)
      {
	if(packet_seq)
	 sendto(sockfd,NACK1,6,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
	else
	 sendto(sockfd,NACK0,6,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
	
      }
      //Send an acknowledgement	
      else
      {
	if(packet_seq)
	 sendto(sockfd,ACK0,6,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
	else
	 sendto(sockfd,ACK1,6,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      }
      randomChecksumGen++;
       
  }
  //write(1,p1.x,sizeof(p1.x));
  fclose(fpOut);
  close(connfd);
   
}



Status 
