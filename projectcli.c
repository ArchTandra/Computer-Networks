#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>   /* inet(3) functions */
#include<time.h>
#include<float.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXLINE 4096

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv)
{
	int sockfd;
  int portno;
  char *hostname;
	struct sockaddr_in servaddr;
  struct hostent *server;
  /* check command line arguments */
  if (argc != 3) {
     fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
     exit(0);
  }
  hostname = argv[1];
  portno = atoi(argv[2]);

  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
      fprintf(stderr,"ERROR, no such host as %s\n", hostname);
      exit(0);
  }
/* build the server's Internet address */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
  (char *)&servaddr.sin_addr.s_addr, server->h_length);
	servaddr.sin_port = htons(portno);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  /* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	exit(0);
}

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{

  double timeArray[10];
	char sendline[MAXLINE]= "PING", recvline[MAXLINE+1];
  double max = 0;
  int n ,counter = 0, packetsReceived=0,  lostPackets=0;
  struct timespec tstart={0,0}, tend={0,0};   // intilization variables for RTT calculation

	for(counter=0;counter < 10;counter++)
	{
    /* getting start time of the ping message */
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    sendline[MAXLINE] =sendline[MAXLINE] +counter;
    /* send the message to the server */
		sendto(sockfd,sendline , strlen(sendline), 0, pservaddr, servlen);

    /* getting server reply from server*/
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

    /* getting start time of the ping message */
    clock_gettime(CLOCK_MONOTONIC, &tend);
		recvline[n] = '\0';    /* null terminate */
               if(strcmp(recvline,sendline)==0){ // comparing server reply
                  timeArray[packetsReceived] = (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
                  ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec)); // calculating RTT
                  printf("%d : Sent... RTT =%0.5f ms \n",counter+1,(timeArray[packetsReceived])*1000);
                  packetsReceived++;
               }
               else if(strcmp(recvline,"lost")==0){
                 lostPackets++;
                 printf("%d : Sent... Timed Out \n",counter+1);
               }
	}
  double maxValue=0;
  double minValue=DBL_MAX;
  double avgTime=0;
  for(int i=0;i<packetsReceived;i++)
  {
    if(maxValue<timeArray[i])
      maxValue = timeArray[i]; // computing maximum value
    if(minValue>timeArray[i])
      minValue=timeArray[i]; //computing minimum value
     avgTime+=timeArray[i];  //computing average value
  }
  printf("\n%d pkts xmited, %d pkts rcvd, %d %% pkt lost\n",counter,packetsReceived,(lostPackets*100/counter));
  printf("min: %.5f ms, max: %.5f ms, avg: %.5f ms", minValue*1000, maxValue*1000,(avgTime*1000)/packetsReceived);
}
