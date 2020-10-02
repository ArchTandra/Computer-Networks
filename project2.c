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
#include <time.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv)
{
	int sockfd;
  char *protocol;
  int portlow;
  int porthigh;
  char *hostname;
  int port;
	struct timeval tv;
	char mesg[MAXLINE];
	struct sockaddr_in servaddr;
  struct hostent *server;
  struct servent *srvport;
	char *servicename;
	fd_set read_fds, write_fds;

  /* check command line arguments */
  if (argc != 5) {
     fprintf(stderr,"usage: ./portScan <hostname> <protocol> <portlow> <porthigh>\n");
     exit(0);
  }
  hostname = argv[1];
  protocol = argv[2];
  portlow = atoi(argv[3]);
  porthigh = atoi(argv[4]);
  printf("scanning host=%s, protocol=%s, ports: %d -> %d \n",hostname,protocol,portlow,porthigh);
  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(hostname);
  if (server == NULL) {
      fprintf(stderr,"error: host %s not exist \n", hostname);
      exit(0);
  }
  if(strcasecmp(protocol, "tcp")==0)
  protocol = "tcp";
  else if (strcasecmp(protocol, "udp")==0)
  protocol = "udp";
  else
  {
  fprintf(stderr, "invalid protocol: %s. Specify tcp or udp \n",protocol);
  exit(-1);
  }

  if(strcasecmp(protocol, "tcp")==0) {    // portscanner TCP
  	for(port = portlow; port <= porthigh; port++)
		    {
		  	  // open stream socket
		  	  if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		  	  	{
		  	    perror("* socket(,SOCK_STREAM,) failed ***n");
		  	    exit(-1);
		  	  	}
		  	  	bzero(&servaddr, sizeof(servaddr));
		      	servaddr.sin_family = AF_INET;
		      	servaddr.sin_port = htons(port);
		      	servaddr.sin_addr = *((struct in_addr *)server->h_addr);

		      if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0) // To establish connection and see whether port open or not
		  	  	{
		      		srvport = getservbyport(htons(port), "tcp");
				  	  if(srvport != NULL){
								servicename = srvport->s_name;
							}else{
								servicename = "svc name unavail";  // assigning service name if name is not available
							}
				  		printf("port %d: open: %s\n", port, servicename);
				  	  fflush(stdout);
		  	  	}else if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
		         srvport = getservbyport(htons(port), "tcp");
						 if(srvport != NULL)
				     printf("port %d: closed : %s \n", port, srvport->s_name);
				     fflush(stdout);
		      }
		  	  close(sockfd);
    	 	}//end of for()
  }else if(strcasecmp(protocol, "udp")==0){  //PortScanner UDP
				for(port = portlow; port <= porthigh; port++)
				    {
							sockfd = socket(AF_INET, SOCK_DGRAM, 0);
						  /*
						   * build the server's Internet address
						   */
							bzero(&servaddr, sizeof(servaddr));
							servaddr.sin_family = AF_INET;
							servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
							servaddr.sin_port = htons((unsigned short)port);

							FD_ZERO(&read_fds);
							FD_SET(sockfd, &read_fds);
							tv.tv_sec = 10;
							tv.tv_usec = 1;

							if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0){ //To verify connection
					   	int conRestult = sendto(sockfd,"Hello" ,5,0,(struct sockaddr *) &servaddr, sizeof(servaddr)); //to send packets
							int nready = select(sockfd+1, &read_fds, NULL, NULL, &tv);
							if(nready==0){
							 	srvport = getservbyport(htons(port), "udp");
									if(srvport != NULL){
										servicename = srvport->s_name;
									}else{
										servicename = "svc name unavail";  // assigning service name if name is not available
									}
						  		printf("port %d: open: %s\n", port, servicename);
						  	  fflush(stdout);
									close(sockfd);
								}else if(nready==1){
									int nbytes = recvfrom(sockfd, mesg, MAXLINE, 0, NULL, NULL);  //receving data from the  server
									if(nbytes >=0){
											srvport = getservbyport(htons(port), "udp");
											if(srvport != NULL){
												servicename = srvport->s_name;
											}else{
												servicename = "svc name unavail";
											}
								  		printf("port %d: open: %s\n", port, servicename);
								  	  fflush(stdout);
									}else{
									srvport = getservbyport(htons(port), "udp");
									if(srvport != NULL)
									printf("port %d: closed : %s\n", port, srvport->s_name);
									fflush(stdout);
									close(sockfd);
								}
							}
							else{
								 close(sockfd);   //closing socket
							}

						}
						else{
							printf("\n Error : Connect Failed \n"); // Error if connect is failed in UDP
						   continue;
						}
		    	 	}//end of for()
			}

	exit(0);
}
