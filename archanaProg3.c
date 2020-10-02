#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct client{
  struct sockaddr_in clientAddress;
  int length;
  int clientRegisteredFlag;
  int clientQuitFlag;
  int clientIndex;
  int socketId;
  char *usrName;
};

struct client ClientDetails[1024];
pthread_t thread[1024];
int noOfClients = 0;
int noOFClientsRegistered = 0;
int retrieveSockIdFrmUsrNm(char *usrNm);
char* retrieveUsrNmFrmSockId(int reqSockId);
int clientRegisteredStatus(int sockId);
char* retrieveUsrNameFrmCmnd(char *userName, char *command);
char* getToken(char *userName, int reqToken);
char* getMessage(char *message, int isBcst);
void * doNetworking(void * ClientDetail);
void printError(char *message);

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2)
	{
		fprintf(stderr,"usage: ./prog3svr <svr_port> \n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		printError("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printError("ERROR on binding");
	}
	listen(sockfd,10);
	printf("Waiting for Incoming Connections... \n");
  while(1){
		ClientDetails[noOfClients].socketId = accept(sockfd, (struct sockaddr*) &ClientDetails[noOfClients].clientAddress, &ClientDetails[noOfClients].length);
		ClientDetails[noOfClients].clientIndex = noOfClients;
		pthread_create(&thread[noOfClients], NULL, doNetworking, (void *) &ClientDetails[noOfClients]);
		printf("Client (%d): Connection Accepted \n",ClientDetails[noOfClients].socketId);
		printf("Client (%d): Connection Handler Assigned \n",ClientDetails[noOfClients].socketId);
		noOfClients ++;
	}

	for(int i = 0 ; i < noOfClients ; i ++)
	{	pthread_join(thread[i],NULL);}

	close(newsockfd);
	close(sockfd);

	return 0;
}
int retrieveSockIdFrmUsrNm(char *usrNm)
{
  for(int i=0;i<noOfClients;i++)
  {
    if(ClientDetails[i].clientQuitFlag!=1 &&
      strcmp(ClientDetails[i].usrName, usrNm)==0 &&
      ClientDetails[i].clientRegisteredFlag!=0)
    {
        return ClientDetails[i].socketId;
    }
  }
  return -1;
}
char* retrieveUsrNmFrmSockId(int reqSockId)
{
  for(int i=0;i<noOfClients;i++)
  {
    if(ClientDetails[i].clientQuitFlag!=1 && reqSockId==ClientDetails[i].socketId)
    {
        return ClientDetails[i].usrName;
    }
  }
  return "";
}
int clientRegisteredStatus(int sockId)
{
  for(int i=0;i<noOfClients;i++)
  {
    if(ClientDetails[i].clientQuitFlag!=1 &&
      ClientDetails[i].socketId == sockId &&
      ClientDetails[i].clientRegisteredFlag == 1)
    return 1;
  }

  return 0;
}
char* retrieveUsrNameFrmCmnd(char *userName, char *command)
{
  char *result = (char*)malloc(150);
  char *token = strtok(userName, " ");
  strcpy(result,"");
   while( token != NULL ) {
       if(strcmp(token, command)>0)
       {
              strcat(result, token);
       }
      token = strtok(NULL, " ");
   }
  result=strndup(result, strlen(result)-2);
  return result;
}


char* getFirstToken(char *userName, int reqToken)
{

  char *result = (char*)malloc(100);
  strcpy(result,"");
  char *token = strtok(userName, " ");
  int tokenIndex = 1;
   while(token != NULL) {
     if(reqToken == tokenIndex)
     {
       return token;
     }
     tokenIndex++;
     token = strtok(NULL, " ");
   }
   strcat(result, "\0");

   return result;
}

char* getToken(char *usrNm, int indexOfToken)
{
  char *resultVal = (char*)malloc(100);
  strcpy(resultVal,"");
  char *separatedInutItems = strtok(usrNm, " ");
  int tokenPosition = 1;
   while(separatedInutItems != NULL) {
     if(indexOfToken == tokenPosition)
     {
       strcat(separatedInutItems, "\0");
       return separatedInutItems;
     }
     tokenPosition++;
     separatedInutItems = strtok(NULL, " ");
   }
   strcat(resultVal, "\0");
   return resultVal;
}

char* getMessage(char *message, int isBcst)
{
  char *result = (char*)malloc(100);
  strcpy(result,"");
  char *token = strtok(message, " ");
  int tokenIndex = 1;
   while( token != NULL ) {

       if(isBcst==0 && (tokenIndex!=1 && tokenIndex!=2))
       {
              strcat(result, token);
              strcat(result, " ");
       }
       else if(isBcst ==1 && tokenIndex!=1)
       {
            strcat(result, token);
            strcat(result, " ");
       }
      token = strtok(NULL, " ");
      tokenIndex++;
   }
  result=strndup(result, strlen(result)-2);
  return result;
}
void printError(char *message)
{
    perror(message);
    exit(1);
}
void * doNetworking(void * ClientDetail){
	struct client* clientDetail = (struct client*) ClientDetail;
	int clientIndex = clientDetail -> clientIndex;
	int clientSocket = clientDetail -> socketId;
	int registered = 0;
	while(1){
		char data[1024];
		int read = recv(clientSocket,data,1024,0);
		data[read] = '\0';

    if(!(strcmp(getFirstToken(data, 1),"JOIN") == 0 || strcmp(getFirstToken(data, 1),"MESG") == 0 || strcmp(getFirstToken(data, 1),"LIST") == 0 ||
    strcmp(getFirstToken(data, 1),"BCST") == 0 || strcmp(getFirstToken(data, 1),"QUIT") == 0))
    {
      char outputDiscard[1024];
      printf("Client (%d): Unknown Message. Discarding UNKNOWN Message.\n",clientSocket);
      snprintf(outputDiscard,1024,"Unknown Message. Discarding UNKNOWN Message.\n");
      send(clientSocket,outputDiscard,strlen(outputDiscard),0);
    }

      if(strcmp(getFirstToken(data, 1),"JOIN") == 0){
        char output[1024];
        if(clientRegisteredStatus(clientSocket) != 1)
        {
          if(noOFClientsRegistered <10){
            clientDetail-> clientRegisteredFlag =1;
            clientDetail->usrName = retrieveUsrNameFrmCmnd(data, "JOIN");
  		    clientDetail->clientQuitFlag=0;
            clientDetail->socketId = clientSocket;
            printf("Client (%d): JOIN  %s\n",clientSocket, clientDetail->usrName);
            snprintf(output,1024,"%s Request Accepted\n", data);
            send(clientSocket,output,strlen(output),0);
            noOFClientsRegistered++;
          }else if(noOFClientsRegistered >=10){
            clientDetail->clientRegisteredFlag =0;
            clientDetail->clientQuitFlag=1;
            snprintf(output,1024,"Too Many Users. Disconnecting User.\n");
            send(clientSocket,output,strlen(output),0);
            close(clientSocket);
            pthread_exit(thread[clientDetail->clientIndex]);
            strcpy(data, "");
          }
        }
        else
        {
          snprintf(output,1024,"User Already Registered: Username (%s), FD (%d).\n",clientDetail->usrName, clientDetail->socketId );
          send(clientSocket,output,strlen(output),0);
        }
        strcpy(data, "");

      }

      if(strcmp(getToken(data, 1),"MESG") == 0){
        char output[1024];

        if(clientRegisteredStatus(clientSocket) == 1)
        {
          char dataFrmUsr[100];
          strcpy(dataFrmUsr, data);
          char *userName = getToken(dataFrmUsr,2);
          strcat(userName, "\0");
          int sockIfForUsrName = retrieveSockIdFrmUsrNm(userName);
          if(sockIfForUsrName!=-1 && clientDetail-> clientRegisteredFlag==1)
          {
             char messg[100];
              strcpy(messg, data);

              snprintf(output,1024,"FROM %s: %s\n", retrieveUsrNmFrmSockId(clientSocket), getMessage(messg, 0));
              send(sockIfForUsrName,output,strlen(output),0);
          }
          else
          {
			printf("Unable to Locate Recipient (%s) in Database. Discarding MESG.\n",userName);
            snprintf(output,1024,"Unknown Recipient (%s) . MESG Discarded.\n" , userName);
            send(clientSocket,output,strlen(output),0);
          }
        }
        else
        {
          snprintf(output,1024,"Unregistered User. Use \"JOIN <usrName>\" to Register.\n");
          send(clientSocket,output,strlen(output),0);
        }
  		}

  		if(strcmp(getToken(data, 1),"LIST") == 0){
        char outputList[1024];
          if(clientDetail->clientRegisteredFlag == 1){
            int l = 0;
            l += snprintf(outputList + l,1024,"USERNAME     FD\n");
            l += snprintf(outputList + l,1024,"------------------------\n");
            for(int i = 0 ; i < noOfClients ; i++){
              if(ClientDetails[i].clientRegisteredFlag == 1){
                l += snprintf(outputList + l,1024,"%s",ClientDetails[i].usrName);
                for(int f=13-strlen(ClientDetails[i].usrName);f>=0;f--)
                  l += snprintf(outputList + l,1024,"%s"," ");
                  l+= snprintf(outputList + l,1024,"%d\n",ClientDetails[i].socketId);
              }
            }
          l += snprintf(outputList + l,1024,"------------------------\n");
            printf("Client (%d): LIST\n",clientSocket);
          send(clientSocket,outputList,strlen(outputList),0);
        }else{
          printf("Unable to Locate Client (%d) in Database. Discarding LIST \n",clientSocket);
          snprintf(outputList,1024,"Unregistered User. Use 'JOIN <usrName>' to Register.\n");
          send(clientSocket,outputList,strlen(outputList),0);
        }
  		}

  		if(strcmp(getToken(data, 1),"BCST") == 0){
        char output[1024];
        if(clientDetail->clientRegisteredFlag == 1){
          for(int i = 0 ; i < noOfClients ; i ++){
            if(ClientDetails[i].clientRegisteredFlag == 1)
            {
              char messg[100];
              strcpy(messg, data);
              if(clientSocket != ClientDetails[i].socketId)
              {
                snprintf(output,1024,"FROM %s: %s\n", retrieveUsrNmFrmSockId(clientSocket), getMessage(messg, 1));
                send(ClientDetails[i].socketId,output,strlen(output),0);
              }
            }
          }
        }
        else
        {
          printf("Unable to Locate Client (%d) in Database. Discarding BCST \n",clientSocket);
          snprintf(output,1024,"Unregistered User. Use 'JOIN <usrName>' to Register.\n");
          send(clientSocket,output,strlen(output),0);
        }
  		}

    if(strcmp(getToken(data, 1),"QUIT") == 0){
	  if(clientDetail->clientRegisteredFlag == 1)
	  {
		  printf("Client (%d): QUIT\n",clientSocket);
		  clientDetail->clientRegisteredFlag =0;
		  noOFClientsRegistered--;
		  clientDetail->clientQuitFlag=1;
		  printf("Client (%d): Disconnecting User.\n",clientSocket);
		  close(clientSocket);
		  pthread_exit(thread[clientDetail->clientIndex]);
		  strcpy(data, "");
	  }
	  else
	  {
		  printf("Client (%d): QUIT\n",clientSocket);
		  printf("Unable to Locate Client (%d) in Database. Discarding User. \n",clientSocket);
		  printf("Client (%d):Disconnecting User.\n",clientSocket);
		  clientDetail->clientRegisteredFlag =0;
		  clientDetail->clientQuitFlag=1;
		  close(clientSocket);
		  pthread_exit(thread[clientDetail->clientIndex]);
		  strcpy(data, "");
	  }

    }



}
	return NULL;
}
