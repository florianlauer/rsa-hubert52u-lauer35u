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
  int sockfd,newsockfd;
  char * port_str = NULL;
  struct sockaddr_in addr_client, addr_serveur;  // Définition des sockets
  

  if(argc<2)
    error("veuillez rentrer le numéro de port en argument");


  /* Initalisation des sockets à zéro */

  bzero(&addr_serveur, sizeof(addr_serveur));
  bzero(&addr_client, sizeof(addr_client));

  addr_serveur.sin_family=AF_UNSPEC;                 // Adresse IPV4
  addr_serveur.sin_port=htons(atoi(argv[1]));      // Numéro de port en paramètre de l'executable
  addr_serveur.sin_addr.s_addr=INADDR_ANY;         // Accepte toutes les IPs des machines


  /* Ouverture de la socket d'écoute*/

  sockfd=socket(IPV6_V6ONLY,SOCK_STREAM,IPPROTO_TCP);

  if(sockfd<0) {
    error("Problème lors de l'initialisation");
  }

  if(bind(sockfd,(struct sockaddr*)&addr_serveur,sizeof(addr_serveur))<0) {
    error("Erreur de bind");
  }


  listen(sockfd,30);                            // La socket se met en écoute, file d'attente de 30 connexions
  int clilen=sizeof(addr_client);



  checkpoint: // ?????

  /* Création de la socket de dialogue*/

  newsockfd=accept(sockfd,(struct sockaddr*)&addr_client,&clilen);

  if(newsockfd<0) {
    error("Problème lors du accept");
  }

  /* Création d'un processus fils */

  pid=fork();


  if(pid==0) {

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


      struct addrinfo hints, * res;

      bzero(&res,sizeof(struct addrinfo));

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;

      //printf("\n %d :  %s : \n", getaddrinfo(t2, NULL, &hints, &res), gai_strerror(getaddrinfo(t2, NULL, &hints, &res)));



      if (getaddrinfo(t2, NULL, &hints, &res) != 0) {
              perror("getaddrinfo");
              return 1;
            }

      

      



      if(specified==1)
      {
        tmp=strtok(NULL,"/");
        port_str = tmp;
        port=atoi(tmp);         // tmp = "99" si specified = 1
      }

      strcat(t1,"^]");          // http://www.syphiliste.tk:99/lol -> http://www.syphiliste.tk:99/lol^]

      tmp=strtok(t1,"//");      // http://www.syphiliste.tk:99/lol^] -> "http:" et "www.syphiliste.tk:99/lol^]"

      tmp=strtok(NULL,"/");     // "www.syphiliste.tk:99/lol^]" -> "www.syphiliste.tk:99"

      if(tmp!=NULL)             // Si on est à la racine
        tmp=strtok(NULL,"^]");
      
      printf("Coucou"); 
      printf("\nPath = %s\nPort = %d\n",tmp,port);

      

          struct sockaddr_in * host_addr; 
          struct sockaddr_in6 * host_addr6;

          //bzero(host_addr, sizeof(host_addr));
          //bzero(host_addr6, sizeof(host_addr6));
             

          char  tmp2 [INET_ADDRSTRLEN];   // "Human-readable" adresse IP

      switch(res->ai_family){

          case AF_INET:
          host_addr = (struct sockaddr_in *) (res -> ai_addr) ;  // Paramétrage du socket de communication avec le serveur à partir des infos dans res
          host_addr->sin_port=htons(port);
          host_addr->sin_family=AF_INET;     // ipv4



          sockfd1=socket(AF_INET,res -> ai_socktype,res -> ai_protocol);    // Création de la socket d'écoute
          newsockfd1=connect(sockfd1,(struct sockaddr*)host_addr,sizeof(struct sockaddr));
                
          inet_ntop(AF_INET, &(host_addr->sin_addr), tmp2, INET_ADDRSTRLEN);      //Cast de l'adresse dans un format lisible
          sprintf(buffer,"\nConnecté au site %s  d'IP : %s\n",t2,tmp2);

          break;
          case AF_INET6:
          host_addr6 = (struct sockaddr_in6 *) (res -> ai_addr) ;  // Paramétrage du socket de communication avec le serveur à partir des infos dans res
          host_addr6->sin6_port=htons(port);
          host_addr6->sin6_family=AF_INET6;     // ipv6
          host_addr6->sin6_addr = in6addr_any;
          host_addr6->sin6_scope_id =0;

          /*struct sockaddr * tmp_sockaddr = (struct sockaddr*)host_addr6;
          tmp_sockaddr->sa_family = AF_INET6;*/

          sockfd1=socket(AF_INET6,res -> ai_socktype,res -> ai_protocol);    // Création de la socket d'écoute
          newsockfd1=connect(sockfd1,res -> ai_addr,res -> ai_addrlen);

          inet_ntop(AF_INET6, &(host_addr6->sin6_addr), tmp2, INET_ADDRSTRLEN); //Cast de l'adresse dans un format lisible
          sprintf(buffer,"\nConnecté au site %s  d'IP : %s\n",t2,tmp2);
          

          break;



      }


     
      

      
      
      if(newsockfd1<0)
        error("Erreur de connexion à hôte");

      printf("\n%s\n",buffer);
      //send(newsockfd,buffer,strlen(buffer),0);
      bzero((char*)buffer,sizeof(buffer));   // Reset du buffer
      if(tmp!=NULL)
        sprintf(buffer,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",tmp,t3,t2);
      else
        sprintf(buffer,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",t3,t2);


      n=send(sockfd1,buffer,strlen(buffer),0);    // Ecriture du buffer sur la socket d'écoute
      printf("\n%s\n",buffer);
      if(n<0)
        error("Erreur d'écriture sur la socket");
      else{
        do
        {
          bzero((char*)buffer,500);
          n=recv(sockfd1,buffer,500,0);     // n renvoie la longueur du message reçu
          if(!(n<=0))                       // s'il est négatif ou nul
            send(newsockfd,buffer,n,0);     // on tente de renvoyer le message
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
