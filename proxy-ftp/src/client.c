/*********************************************** 
*  ASR => Gpr3                                 * 
************************************************ 
*  Groupe: D                                   * 
************************************************ 
*  Nom-prenom : ARTIGOUHA Noemy                * 
*  Nom-prenom : BOIRON Gregoire                * 
************************************************ 
*  TP API SOCKET                               * 
************************************************ 
*  Nom du fichier : client.c	               * 
***********************************************/ 

#include  <stdio.h>
#include  <unistd.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <stdlib.h>

#define MAXHOSTLEN 64
#define MAXPORTLEN 6
#define MAXBUFFERLEN 1024

int main(int argc, char* argv[]){
		int descSock;                 // Descripteur de la socket
		int err;                    // Retour des fonctions
		struct addrinfo *res;         // Résultat de la focntion getaddrinfo
		struct addrinfo hints = {     // Cette structure permet de contrôler l'exécution de la fonction getaddrinfo
			 0,
			 AF_INET,    //seule les adresses IPv4 seront présentées par la fonctiongetaddrinfo
			 SOCK_STREAM,
			 0,
			 0,
			 NULL,
			 NULL,
			 NULL
		};
		char serverName[MAXHOSTLEN]; // Nom de la machine serveur
		char serverPort[MAXPORTLEN]; // Numéro de port
		char buffer[MAXBUFFERLEN];     // buffer stockant les messages échangés entre le client et le serveur
		char tampon[200+1];	 	 //buffer stockant les messages envoyés entre le client et le serveur
		char user[9+1];
		char commande[MAXBUFFERLEN];

		//On teste les valeurs rentrées par l'utilisateur
		if (argc != 3) { 
		perror("Mauvaise utilisation de la commande: <nom serveur> <numero de port>\n");
		exit(1);
		}
		if (strlen(argv[1]) >= MAXHOSTLEN) {
			perror("Le nom de la machine serveur est trop long\n");
			exit(2);
		}
		if (strlen(argv[2]) >= MAXPORTLEN) {
			perror("Le numero de port du serveur est trop long\n");
			exit(2);
		}
		
		//Initialise variable serverName
		strncpy(serverName, argv[1], MAXHOSTLEN);
		serverName[MAXHOSTLEN-1] = '\0';
		//Initialise variable serverPort
		strncpy(serverPort, argv[2], MAXPORTLEN);
		serverPort[MAXPORTLEN-1] = '\0';
		
		//Création de la socket IPv4/TCP
		descSock = socket(AF_INET, SOCK_STREAM, 0);
		if (descSock == -1) {
			perror("Erreur creation socket");
			exit(4);
		}
		//Récupération des informations sur le serveur
		err = getaddrinfo(serverName,serverPort,&hints,&res);
		if (err != 0){
			fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(err));
			exit(3);
		}
		//Connexion au serveur
		err = connect(descSock, res->ai_addr, res->ai_addrlen);
		if (err == -1) {
			close(descSock);
			freeaddrinfo(res);
			perror("Erreur connect");
			exit(5);
		}
		freeaddrinfo(res);
		
		//Echange de donneés avec le proxyftp
		err = read(descSock, buffer, MAXBUFFERLEN);
		if (err == -1) {
			perror("Probleme de lecture\n");
			exit(6);
		}
		//Met le '\0' a la fin de la chaine (err = nombre d'octets lus)
		buffer[err] = '\0';
		printf("MESSAGE RECU DU SERVEUR: \"%s\".\n", buffer);

		//Lecture de la commande "USER login@serveur" avec attente
		printf("Veuillez saisir la commande USER :\n");
		fgets(tampon,sizeof(tampon),stdin);
		strcat(tampon,"\r\n");		

		printf(tampon);
		//Ecriture de la commande dans le tube associé (descSock)
		err = write(descSock, tampon, strlen(tampon));
		if (err == -1) {
			perror("Probleme d'ecriture\n");
			exit(7);
		}
		
		//Echange de donneés avec le serveur
		err = read(descSock, buffer, MAXBUFFERLEN);
		if (err == -1) {
			perror("Problème de lecture\n");
			exit(8);
		}
		while (strncmp(buffer,"221",3) || strncmp(buffer,"200",3)) {
			//Lecture du message reçu du proxyftp
			printf("%s\n",buffer);
			
			//Remet a zero la chaine de caractere buffer
			memset(buffer, 0, sizeof(buffer));

			//Lecture de la commande avec attente
			fgets(tampon,sizeof(tampon),stdin);

			//Ecriture de la commande dans le tube associé (descSock)
			err = write(descSock, tampon, strlen(tampon));
			if (err == -1) {
				perror("Probleme d'ecriture\n");
				exit(9);
			}

			//Echange de donneés avec le proxyftp
			err = read(descSock, buffer, MAXBUFFERLEN);
			if (err == -1) {
				perror("Problème de lecture\n");
				exit(10);
			}

		}
		printf("Fin de la boucle : %s\n",buffer);
		
		err = close(descSock);
		if (err == -1) {
			error("Probleme lors du close");
			exit(11);
		}
}

