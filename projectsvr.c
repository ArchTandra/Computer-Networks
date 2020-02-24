#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>   /* inet(3) functions */
#include<stdlib.h>
#include <stdlib.h>  // rand(), srand()
#include <time.h>

#define MAXLINE 4096

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen);
int getRandomVal();
int x1, x2,x3, x4;
int main(int argc, char **argv)
{

	int sockfd;
  int portno;
	struct sockaddr_in servaddr,cliaddr;
  srand(time(0));

  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  /*
   * build the server's Internet address
   */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	dg_echo(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

  return 0;
}

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen)
{
	int n;
	socklen_t len;
	char mesg[MAXLINE];
  int i =0;
  printf("[server]: ready to accept data... \n");

	for( ; ; )
	{
  if(i%10 == 0) //for every 10 ping generating new random values for packet loss simulation
  {
    x1=getRandomVal();
    x2=getRandomVal();
    x3=getRandomVal();
    x4=getRandomVal();
  }
		len = clilen;
		n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
    printf("[client]: %s \n",mesg);
    if(i == x1 || i == x2 || i == x3 || i == x4 ){  //packet loss simulation
      sendto(sockfd, "lost", n, 0, pcliaddr, len);  //droping some of the messages
      printf("[server]: dropped packet \n");
    }
    else {
   sendto(sockfd, mesg, n, 0, pcliaddr, len);
    }
     i++;
     if(i == 10) // making server simulate as a ping service for every 10 messeges from client
       i = 0;
	}
}

int getRandomVal()  // creating unique random variable
{
  int randVal = rand()%10;
  if(randVal == x1 || randVal == x2 || randVal ==x3 || randVal == x4)
    getRandomVal();
  else
   return randVal;
}
