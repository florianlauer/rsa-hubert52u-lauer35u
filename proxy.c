#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

void error(char* msg) {
  perror(msg);
  exit(0);
}



int main(int argc,char* argv[]) {
  pid_t pid;
  struct sockaddr_in cli_addr,serv_addr;  // Définition des sockets
  struct hostent* host;
  int sockfd,newsockfd;

  if(argc<2)
    error("veuillez rentrer le numéro de port en argument");


  /* Initalisation des sockets à zéro */

  bzero(&serv_addr, sizeof(serv_addr));
  bzero(&cli_addr, sizeof(cli_addr));

  serv_addr.sin_family=AF_INET;                 // Adresse IPV4
  serv_addr.sin_port=htons(atoi(argv[1]));      // Numéro de port en paramètre de l'executable
  serv_addr.sin_addr.s_addr=INADDR_ANY;         // Accepte toutes les IPs des machines


  /* Ouverture de la socket d'écoute*/

  sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  if(sockfd<0) {
    error("Problème lors de l'initialisation");
  }

  if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0) {
    error("Erreur de bind");
  }


  listen(sockfd,30);                            // La socket se met en écoute, file d'attente de 30 connexions
  int clilen=sizeof(cli_addr);



  checkpoint: // ?????

  /* Création de la socket de dialogue*/

  newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);

  if(newsockfd<0) {
    error("Problème lors du accept");
  }

  /* Création d'un processus fils */

  pid=fork();

  if(pid==0) {
    struct sockaddr_in host_addr;
    int specified=0,newsockfd1,n,port=0,i,sockfd1;
    char buffer[510],t1[300],t2[300],t3[10];
    char* tmp=NULL;
    bzero((char*)buffer,500);
    
    // Equivalent à read(), attente d'un message de la socket de dialogue

    recv(newsockfd,buffer,500,0);

    /*

    Découpage de la requête en trois variables 
    
    GET -> t1 
    url -> t2 
    HTTP/1.1 -> t3 

    */

    sscanf(buffer,"%s %s %s",t1,t2,t3);

    /*

    Vérification de la structure des requêtes : 
    
    GET url HTTP/1.1 

    */

    if(((strncmp(t1,"GET",3)==0))&&((strncmp(t3,"HTTP/1.1",8)==0)||(strncmp(t3,"HTTP/1.0",8)==0))&&(strncmp(t2,"http://",7)==0))
    {
      strcpy(t1,t2);

      specified=0;

      for(i=7;i<strlen(t2);i++)   //Scanne http://www.syphiliste.tk:99/lol pour trouver un ':' en cas de connexion sur un autre portque 80.
      {
        if(t2[i]==':')
        {
          specified=1;
          break;
        }
      }

      tmp=strtok(t2,"//");      // http://www.syphiliste.tk:99/lol -> "http:" et "www.syphiliste.tk:99/lol "

      if(specified==0)
      {
        port=80;
        tmp=strtok(NULL,"/");   // www.syphiliste.tk/lol -> "www.syphiliste.tk" et "lol" 
      }
      else
      {
        tmp=strtok(NULL,":");   // www.syphiliste.tk:99 -> "www.syphiliste.tk et "99" 
      }

      sprintf(t2,"%s",tmp);
      printf("host = %s",t2);

      host=gethostbyname(t2);

      if(specified==1)
      {
        tmp=strtok(NULL,"/");
        port=atoi(tmp);         // tmp = "99" si specified = 1
      }

      strcat(t1,"^]");          // http://www.syphiliste.tk:99/lol -> http://www.syphiliste.tk:99/lol^]
     
      tmp=strtok(t1,"//");      // http://www.syphiliste.tk:99/lol^] -> "http:" et "www.syphiliste.tk:99/lol^]"
     
      tmp=strtok(NULL,"/");     // "www.syphiliste.tk:99/lol^]" -> "www.syphiliste.tk:99"
     
      if(tmp!=NULL)             // Si on est à la racine
        tmp=strtok(NULL,"^]");  
      printf("\npath = %s\nPort = %d\n",tmp,port);


      bzero(&host_addr,sizeof(host_addr));
      
      host_addr.sin_port=htons(port);
      host_addr.sin_family=AF_INET;     // ipv4
      
      bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);

      sockfd1=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      newsockfd1=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));
      sprintf(buffer,"\nConnected to %s  IP - %s\n",t2,inet_ntoa(host_addr.sin_addr));
      if(newsockfd1<0)
        error("Error in connecting to remote server");

      printf("\n%s\n",buffer);
      //send(newsockfd,buffer,strlen(buffer),0);
      bzero((char*)buffer,sizeof(buffer));
      if(tmp!=NULL)
        sprintf(buffer,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",tmp,t3,t2);
      else
        sprintf(buffer,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",t3,t2);


      n=send(sockfd1,buffer,strlen(buffer),0);
      printf("\n%s\n",buffer);
      if(n<0)
        error("Error writing to socket");
      else{
        do
        {
          bzero((char*)buffer,500);
          n=recv(sockfd1,buffer,500,0);
          if(!(n<=0))
            send(newsockfd,buffer,n,0);
        }while(n>0);
      }
    }
    else
    {
      send(newsockfd,"400 : BAD REQUEST\nONLY HTTP REQUESTS ALLOWED",18,0);
    }
    close(sockfd1);
    close(newsockfd);
    close(sockfd);
    _exit(0);
  }

  else
  {
    close(newsockfd);
    goto checkpoint;
  }
  return 0;
}
