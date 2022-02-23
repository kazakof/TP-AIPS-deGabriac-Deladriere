/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>


void construire_message(char *message, char motif, int lg,int j);
void afficher_message(char *message, int lg,int s);
void sourceudp (int numport,char* host,int lg,int nb_message);
void puitudp(int numport,int lg,int nb_message);
void sourcetcp(int numport, char* host,int lg,int nb_message);
void puittcp(int numport,int lg, int nb_message,int nb_message_lire);

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message_lire=0;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int u=0;
	int lg = 30; /* longueur des messages à lire ou écrire selon si puit ou source peut être modifié en écrivant -l*/
	while ((c = getopt(argc, argv, "pn:sul:t")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;
		case 'u':
			u=1;
			break; 
		case 'n':
			nb_message = atoi(optarg);
			nb_message_lire = nb_message;
			break;
		case 'l':
			lg= atoi(optarg);
			break;
		


		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else 
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à recevoir = infini\n");

	}
	if (source == 1){
		printf("on est dans la source\n");
		if (u==1) {
			int port=atoi(argv[argc-1]);
			char * host=argv[argc-2];
            printf("SOURCE: lg_msg_émis:%d, n°port local:%d, nb_messages: %d, TP:UDP, dest:%s \n",lg,port,nb_message,host); 
			sourceudp(port,host,lg,nb_message);
            printf("SOURCE : fin\n");
		}
		if(u==0){
			int port=atoi(argv[argc-1]);
			char * host=argv[argc-2];
            printf("SOURCE: lg_msg_émis:%d, n°port local:%d, nb_messages: %d, TP:TCP, dest:%s \n",lg,port,nb_message,host);
			sourcetcp(port,host,lg,nb_message);
            printf("SOURCE : fin\n");
            
		}

	}
	if (source == 0){
		if (u==1) {
			int port=atoi(argv[argc-1]);
            if(nb_message==-1){
                printf("PUITS: lg_msg_reçu:%d, n°port local:%d, TP:UDP\n",lg,port);
            }
            else {
                printf("PUITS: lg_msg_reçu:%d, n°port local:%d, nb_messages: %d, TP:UDP\n",lg,port,nb_message);
                }
			puitudp(port,lg,nb_message);
			printf("PUITS : fin\n");
		}
		if (u==0){
			int port=atoi(argv[argc-1]);
            if(nb_message==-1){
                printf("PUITS: lg_msg_reçu:%d, n°port local:%d, TP:TCP\n",lg,port);
            }else{
            printf("PUITS: lg_msg_reçu:%d, n°port local:%d, nb_messages: %d, TP:TCP\n",lg,port,nb_message);
            }
			puittcp(port,lg,nb_message,nb_message_lire);
			printf("PUITS : fin\n");
			
		}
	}
} 

void sourceudp (int numport,char* host,int lg,int nb_message){
	
	int sock;
	/* Création de socket */
	if ((sock=socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("echec de creation de socket");
		exit(1);
	}
	
	struct hostent *hp;
	struct sockaddr_in adr_distant; // Réservation

	

	memset((char*)&adr_distant,0,sizeof(adr_distant)); /* reset */
	adr_distant.sin_family = AF_INET; 
	adr_distant.sin_port = htons(numport);

	if((hp=gethostbyname(host))== NULL){
		printf("erreur gethostbyname\n");
		exit(1);
	}
	
	memcpy((char*)&(adr_distant.sin_addr.s_addr),
		hp->h_addr,
		hp->h_length);
	
	
	char* msg = malloc(lg*sizeof(char));
	char motif='a';
	for (int i=0; i<nb_message; i++){
		construire_message(msg,motif+(i%26),lg,i+1);
		sendto(sock,msg,lg,0,(struct sockaddr *)&adr_distant,sizeof(adr_distant));
		afficher_message(msg,lg,1); 
	}
	free(msg);
	close (sock); 

}

void construire_message(char *message, char motif, int lg,int j) {
	int i;
	sprintf(message,"%5d",j);//création du champ numero de message sur 5 espaces 
	for (i=5;i<lg;i++) message[i] = motif;}//remplissage du message avec les caractères

void afficher_message(char *message, int lg,int s) {
	char num[6];
	strncpy(num,message,5); //récuperation de l'entête du message contenant le numero
	if(s==1){
        printf("SOURCE : Envoi n°%s (%d) [%s]\n",num,lg,message);//Affichage au format pour les programmes sources
	}else{
        printf("PUITS : Réception n°%s (%d) [%s]\n",num,lg,message);//Affichage au format pour les programmes puits
	}
}

void puitudp(int numport,int lg,int nb_message){
	int sock2;
	/* Création de socket */
		if ((sock2=socket(AF_INET,SOCK_DGRAM,0))==-1){
		printf("echec de creation de socket");
		exit(1);
	}
	
	struct sockaddr_in adr_local; // Réservation

	memset((char*)&adr_local,0,sizeof(adr_local)); /* reset */
	adr_local.sin_family = AF_INET; 
	adr_local.sin_port = htons(numport);
	adr_local.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock2,(struct sockaddr*)&adr_local,sizeof(struct sockaddr))==-1){
		printf("echec du bind\n");
		perror("bind");
		exit(1);
	}
	
	char * msg = malloc(lg*sizeof(char));
	struct sockaddr * adr_distant = malloc(sizeof(adr_local));
	int *  lg_adr= malloc(sizeof(int)) ;
	if (nb_message==-1){

		while(1){
			recvfrom(sock2,msg,lg,0,adr_distant,lg_adr);
			afficher_message(msg,lg,0);
		}
	}
	else{
			for (int i=0;i<nb_message;i++){
				recvfrom(sock2,msg,lg,0,adr_distant,lg_adr);
				afficher_message(msg,lg,0);
			}
		}
	
	free(msg);
	free(adr_distant);
	free(lg_adr);	
	close(sock2);

}

void sourcetcp(int numport, char* host,int lg,int nb_message){
	int sock;
	int request=-1;
	int lg_message=lg;
	// Création de socket 
	if ((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("echec de creation de socket");
		exit(1);
	}

	struct hostent *hp;
	struct sockaddr_in adr_serveur; // Réservation


	memset((char*)& adr_serveur,0,sizeof(adr_serveur)); // reset 
	adr_serveur.sin_family = AF_INET; 
	adr_serveur.sin_port = htons(numport);

	if((hp=gethostbyname(host))== NULL){
		printf("erreur gethostbyname\n");
		exit(1);
	}

	memcpy((char*)&(adr_serveur.sin_addr.s_addr),
		hp->h_addr,
		hp->h_length);
	
	if(request=connect(sock,(struct sockaddr*)&adr_serveur,sizeof(adr_serveur))==-1){
		printf("erreur connect\n");
		exit(1);
	}
	
	char* message=malloc(lg*sizeof(char));
	char motif='a';
	
	
	for (int i=0;i<nb_message;i++){
		construire_message(message,motif+(i%26),lg,i+1);
		
		if((lg_message=send(sock,message,lg,0))<0){
			printf("erreur send\n");
			exit(1);
		}
		afficher_message(message,lg,1);
	}

free(message);
close(sock);
}

void puittcp(int numport,int lg, int nb_message,int nb_message_lire){
	int sock,sock_bis;
	struct sockaddr_in adr_client,adr_local; // Réservation
	int lg_adr_client=sizeof(adr_client);
	int lg_rec;
	int max=nb_message;
	char * message = malloc(lg*sizeof(char));

	
	/* Création de socket */
	if ((sock=socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("echec de creation de socket");
		exit(1);
	}
	
	memset((char*)& adr_client,0,sizeof(adr_client)); /* reset */
	memset((char*)& adr_local,0,sizeof(adr_local)); /* reset */
	adr_local.sin_family = AF_INET; 
	adr_local.sin_port = htons(numport);
	adr_local.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock,(struct sockaddr*)&adr_local,sizeof(adr_local))==-1){
		printf("echec du bind\n");
		exit(1);
	}

	listen(sock,5);


	if ((sock_bis=accept(sock,(struct sockaddr*)&adr_client,&lg_adr_client))==-1){
		printf("échec du accept\n");
		perror("accept");
		exit(1);
		}
	
	if (nb_message_lire==0){ //Cas ou l'utilisateur n'indique pas un nombre précis de message à lire via la commande -n
		int i=1;
		while((lg_rec=read(sock_bis,message,lg))!=0) {
			if(lg_rec<0){
				printf("échec du read\n");
				exit(1);
				}
			afficher_message(message,lg_rec,0);
			i++;
			}
		
	}else{
		for(int i=0;i<max;i++){ //Cas ou l'utilisateur impose un nombre de message à lire 
			if((lg_rec=read(sock_bis,message,lg))<0){
				printf("échec du read\n");
				exit(1);
			}
			if(lg_rec==0){ //arrêt de l'affichage si toutes les données envoyées ont bien été reçues et que les affichages sont vides
				exit(1);}
			afficher_message(message,lg_rec,0);
		}
	}	

	free(message);
	close(sock);

}
