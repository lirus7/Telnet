#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
int main()

{
  char SERVER_IP[256];
  int port;
  struct addrinfo hint, *info_ptr;//FOr resolving the DNS
  printf("Enter the Telnet_server\n");
  scanf("%s",SERVER_IP);
  printf("Enter the port\n");
  scanf("%d",&port);
  printf("%s  %d \n",SERVER_IP,port);
  //Have to connect to the address of SERVER_IP on port
  hint.ai_family = AF_INET;

  int val = getaddrinfo(SERVER_IP, NULL, &hint, &info_ptr);

  if (val)
    printf("getaddrinfo: %s\n", gai_strerror(val)),exit(0);

  struct addrinfo *ptr;

  char host[256];

  for(ptr = info_ptr; ptr != NULL; ptr = ptr->ai_next)
   {
    getnameinfo(ptr->ai_addr, ptr->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    puts(host);
  }

  freeaddrinfo(info_ptr);
 //finally we have only the IP ADRESS SO WE CAN PASS IT FURTHER

  struct sockaddr_in server;

  server.sin_family=AF_INET;
  server.sin_port=htons(port); //htons is for converts to network byte order

  int sockid=socket(AF_INET,SOCK_STREAM,0); //sockid helps in referencing the socket

  if(sockid<0)
    printf("Couldnt create a socket \n"),exit(0);

  in_addr_t in_addr = inet_addr(host);

  if (INADDR_NONE == in_addr)
  {
     printf("inet_addr() failed i.e. NONE in the address field \n");
     exit(0); /* or whatever error handling you choose. */
  }
    server.sin_addr.s_addr = in_addr;
    //server.sin_addr.s_addr=inet_addr("52.88.68.92");

  int choice=connect(sockid,(struct sockaddr *)&server,sizeof(server));

  if(choice==-1)
      printf("Couldnt connect to the server \n"),exit(0);
      //MAIN LOOP
  int count=0;
  while(1)
  {
    //struct timeval Timeout;
    //Timeout.tv_sec = 1;       // timeout (secs.)
    //Timeout.tv_usec = 500000;            // 0 microseconds
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);//stdin manually trigger reading
    FD_SET(sockid, &readSet);//tcp socket
    int num = select(sockid+1, &readSet, NULL, NULL, NULL);
                          //IT ONLY GETS PAST SELECT ON RETURN FROM THE KEYBOARD
      if(FD_ISSET(sockid,&readSet))
      {
          char s[10000];
          char buffer[10000] = {'\0'};
          int bytesRead = read(sockid, &buffer, sizeof(buffer));
          for(int i=0;i<bytesRead;i++) //CREATING THE STRING FOR SENDING DATA
          {
            if((int)buffer[i]==255)
            {
              if((int)buffer[i+1]==251 || (int)buffer[i]==252 && (int)buffer[i+1]==253 ||  (int)buffer[i+1]==253 )
              {
                if((int)buffer[i+1]==251)
                {
                  s[count++]=buffer[i];
                  s[count++]=(char)254;
                  s[count++]=buffer[i+2];
                }
                else if((int)buffer[i+1]==253)
                {
                  s[count++]=buffer[i];
                  s[count++]=(char)252;
                  s[count++]=buffer[i+2];
                }
              }
              else
               i++;
             }
            else
              printf("%c",buffer[i]);
          }
          if(bytesRead)
          {
          //printf("\n bytesRead %i \n %s", bytesRead, buffer); //PRINTING THE DATA RECEIVED FROM BUFFER
          if(count)
          {
            s[count++]='\r'; //ENDING THE STRING
            s[count++]='\n'; //ENDING THE STRING
            s[count]='\0'; //ENDING THE STRING
            printf("Im the string %s\n",s); //TRYING TO DISPLAY THE STRING
            fflush(stdout);
            send(sockid,s,count,0);//SENDING TO THE SERVER
          }
        }
      }
    else
    {
      printf("\ni have entered\n");
      char *command;
      size_t bufsize = 10;
      size_t b_read;
      printf("Enter the command\n");
      b_read=getline(&command,&bufsize,stdin);
      command[b_read++]='\r';
      command[b_read++]='\n';
      command[b_read]='\0';
      send(sockid,command,b_read,0);
    }
  }
  return 0;
}
