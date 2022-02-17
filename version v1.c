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


void construire_message(char *message, char motif, int lg);
void afficher_message(char *message, int lg);
void sourceudp (int numport,char* host,int lg,char motif, int nb_message);
void puitudp(int numport,char* host,int lg,char motif,int nb_message);
//void sourcetcp();
//void puittcp();

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int u =0;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	while ((c = getopt(argc, argv, "pn:su")) != -1) {
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
			sourceudp(port,host,30,'a',nb_message);
		}
	}
	if (source == 0){
		printf("on est dans le puits\n");
		if (u==1) {
			int port=atoi(argv[argc-1]);
			char * host=argv[argc-2]; 
			puitudp(port,host,30,'a',nb_message);
		}
	}
} 

void sourceudp (int numport,char* host,int lg,char motif, int nb_message){
	
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

	for (int i=0; i<nb_message; i++){
		construire_message(msg,motif+i,lg);
		sendto(sock,msg,lg,0,(struct sockaddr *)&adr_distant,sizeof(adr_distant));
		afficher_message(msg,lg); 
	}
	free(msg);
	close (sock); 

}

void construire_message(char *message, char motif, int lg) {
	int i;
	for (i=0;i<lg;i++) message[i] = motif;}

void afficher_message(char *message, int lg) {
	int i;
	printf("message construit : ");
	for (i=0;i<lg;i++) printf("%c", message[i]); printf("\n");
}

void puitudp(int numport,char* host,int lg,char motif,int nb_message){
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
			afficher_message(msg,lg);
		}
	}
	else{
			for (int i=0;i<=nb_message;i++){
				recvfrom(sock2,msg,lg,0,adr_distant,lg_adr);
				afficher_message(msg,lg);
			}
		}
	
	free(msg);
	free(adr_distant);
	free(lg_adr);	
	close(sock2);

}