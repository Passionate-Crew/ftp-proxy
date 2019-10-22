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
*  Nom du fichier : proxyftp.c	               * 
***********************************************/ 

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>

#define SERVADDR "127.0.0.1"          // Définition de l'adresse IP d'écoute
#define SERVPORT "0"                // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1                 // Taille du tampon de demande de connexion
#define MAXBUFFERLEN 1024
#define MAXHOSTLEN 64
#define MAXPORTLEN 6

int main(){

		int ecode;                       // Code retour des fonctions
		int n1,n2,n3,n4,n5,n6;		 // Variables pour la commande PORT
		//Variable client
		char serverAddr[MAXHOSTLEN];     // Adresse du serveur
		char serverPort[MAXPORTLEN];     // Port du server
		int descSockRDV;                 // Descripteur de socket de rendez-vous
		int descSockCOM;                 // Descripteur de socket de communication
		int descSock;
		struct addrinfo hints;           // Contrôle la fonction getaddrinfo
		struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
		struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
		struct sockaddr_storage from;    // Informations sur le client connecté
		
		//Variable serveur
		struct addrinfo *resultat;         // Résultat de la focntion getaddrinfo
        struct addrinfo hints2 = {     // Cette structure permet de contrôler l'exécution de la fonction getaddrinfo
			0,
			AF_INET,    //seule les adresses IPv4 seront présentées par la fonctiongetaddrinfo
			SOCK_STREAM,
			0,
			0,
			NULL,
			NULL,
			NULL
    	};
		socklen_t len;                   // Variable utilisée pour stocker les 
						 // longueurs des structures de socket
		char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur
		char serverName[MAXHOSTLEN]; 	// Nom de la machine serveur

		long nb1;
		long nb2;

		char tampon[MAXBUFFERLEN];
		char commande[MAXBUFFERLEN];
		char host[MAXBUFFERLEN];
		char server[MAXBUFFERLEN];
		char addrClient[MAXHOSTLEN];	//Variable utilisé pour stocker l'@ du client lors transfer
		char portClient[MAXPORTLEN];	//Port du client pour transfer de données
		char addrServeur[MAXHOSTLEN];	//Variable utilisé pour stocker l'@ du serveur lors transfer
		char portServeur[MAXPORTLEN];	//Port du serveur pour transfer de données


		// Initialisation de la socket de RDV IPv4/TCP
		descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
		if (descSockRDV == -1) {
			 perror("Erreur création socket RDV\n");
			 exit(2);
		}
		// Publication de la socket au niveau du système
		// Assignation d'une adresse IP et un numéro de port
		// Mise à zéro de hints
		memset(&hints, 0, sizeof(hints));
		// Initailisation de hints
		hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
		hints.ai_socktype = SOCK_STREAM;  // TCP
		hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par 
						  // la fonction getaddrinfo

		 // Récupération des informations du serveur
		 ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
		 if (ecode) {
			 fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
			 exit(1);
		 }
		 // Publication de la socket
		 ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
		 if (ecode == -1) {
			 perror("Erreur liaison de la socket de RDV");
			 exit(3);
		 }
		 // Nous n'avons plus besoin de cette liste chainée addrinfo
		 freeaddrinfo(res);
		 // Récuppération du nom de la machine et du numéro de port pour affichage à l'écran

		 len=sizeof(struct sockaddr_storage);
		 ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
		 if (ecode == -1)
		 {
			 perror("SERVEUR: getsockname");
			 exit(4);
		 }
		 ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
							 serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
		 if (ecode != 0) {
				 fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
				 exit(4);
		 }
		 printf("L'adresse d'ecoute est: %s\n", serverAddr);
		 printf("Le port d'ecoute est: %s\n", serverPort);
		 // Definition de la taille du tampon contenant les demandes de connexion
		 ecode = listen(descSockRDV, LISTENLEN);
		 if (ecode == -1) {
			 perror("Erreur initialisation buffer d'écoute");
			 exit(5);
		 }

		len = sizeof(struct sockaddr_storage);
		 // Attente connexion du client
		 // Lorsque demande de connexion, creation d'une socket de communication avec le client
		 descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
		 if (descSockCOM == -1){
			 perror("Erreur accept\n");
			 exit(6);
		 }
		 // Echange de données avec le client connecté
		 strcpy(buffer, "220 Connexion etablie\n");
		 write(descSockCOM, buffer, strlen(buffer));

		 //lecture des données envoyé par le client (USER login@serveur)
		 ecode = read(descSockCOM, tampon, sizeof(tampon));
		 if (ecode == -1) {
			perror("Problème de lecture\n");
			exit(9);
		 }
		 //Decomposition de la chaine de caractere
		 sscanf(tampon,"%[^' '] %[^@]@%[^\r]",commande,host,server);

		//Remet a zero la chaine de caractere tampon
		memset(tampon, 0, sizeof(tampon));
		 
		//----------------------------------------------------------------//
		//------------------ Connection au server ftp---------------------//
		//----------------------------------------------------------------//

		if (strlen(server) >= MAXHOSTLEN) {
			perror("Le nom de la machine serveur est trop long\n");
			exit(2);
		}

		//Initialise variable serverPort (21 par default)
		strncpy(serverPort, "21\0", 3);
		//serverPort[MAXPORTLEN-1] = '\0';
		printf("\n",serverPort);

		//Création de la socket IPv4/TCP
		descSock = socket(AF_INET, SOCK_STREAM, 0);
		if (descSock == -1) {
			perror("Erreur creation socket");
			exit(4);
		}

		//Récupération des informations sur le serveur
		ecode = getaddrinfo(server,serverPort,&hints2,&resultat);
		if (ecode != 0){
			fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
			exit(3);
		}

		//Connexion au serveur ftp
		ecode = connect(descSock, resultat->ai_addr, resultat->ai_addrlen);
		if (ecode == -1) {
			close(descSock);
			freeaddrinfo(resultat);
			perror("Erreur connect");
			exit(5);
		}
		freeaddrinfo(resultat);

		//Echange de donneés avec le serveur (verifie si connexion a était etablie)
		ecode = read(descSock, buffer, MAXBUFFERLEN);
		if (ecode == -1) {
			perror("Problème de lecture\n");
			exit(6);
		}
		//Met le '\0' a la fin de la chaine (ecode = nombre d'octets lus)
		buffer[ecode] = '\0';
		//Verifie si la connexion est etablie
		char ok[3+1] = "220\0";
		if(strncmp(buffer,ok,3)) {
			perror("La connexion n'a pas était etablie correctement");
 			//Fermeture de la connexion
		 	close(descSockCOM);
		 	close(descSockRDV);
			exit(7);
		}

		//Remet a zero la chaine de caractere server
		memset(tampon, 0, sizeof(server));

		//Remet a zero la chaine de caractere tampon
		memset(tampon, 0, sizeof(tampon));
		//------------------------------------------------------------------------------//
		//------------------------Fin de connexion au server FTP------------------------//
		//------------------------------------------------------------------------------//
		 
		 //Envoie de la commande "USER login" avec la variable "commande"
		 //Concatene "commande" avec \r\n
		 strcat(host,"\n");
		 strcat(commande," ");
		 strcat(commande,host);
		 //Echange de donnée avec le serveur ftp
		 ecode = write(descSock, commande, strlen(commande));
		 if (ecode == -1) {
			perror("Problème d'ecriture\n");
			exit(8);
		 }

		 memset(commande, 0, sizeof(commande));

		 //Lecture des données du serveur ftp
		 ecode = read(descSock, buffer, MAXBUFFERLEN);
		 if (ecode == -1) {
			perror("Problème de lecture\n");
			exit(9);
		 }
		 //Met le '\0' a la fin de la chaine (ecode = nombre d'octets lus)
		 buffer[ecode] = '\0';
		 //Ecriture des données au client
		 ecode = write(descSockCOM, buffer, strlen(buffer));
		 if (ecode == -1) {
			perror("Problème d'ecriture\n");
			exit(10);
		 }

		 //Lecture des données du client
		 ecode = read(descSockCOM, tampon, MAXBUFFERLEN);
		 if (ecode == -1) {
			perror("Problème de lecture\n");
			exit(11);
		 }

		//on a récupéré la commande PASS XXX qui doit être relayée
		while(strncmp(tampon,"QUIT",4) != 0) {
		 while(strncmp(tampon,"QUIT",4) != 0 && strncmp(tampon,"PORT",4) != 0){

			printf("Le client envoye : %s FIN\n",tampon, strlen(tampon));

			//Echange de donnée avec le serveur ftp
		 	ecode = write(descSock, tampon, strlen(tampon));
		 	if (ecode == -1) {
				perror("Problème d'ecriture\n");
				exit(8);
		 	}
			//Remet a zero la chaine de caractere buffer
			memset(tampon, 0, sizeof(tampon));

			//Remet a zero la chaine de caractere buffer
			memset(buffer, 0, sizeof(buffer));

		 	//Lecture des données du serveur ftp
		 	ecode = read(descSock, buffer, MAXBUFFERLEN);
		 	if (ecode == -1) {
				perror("Problème de lecture\n");
				exit(9);
			}
			//Met le '\0' a la fin de la chaine (ecode = nombre d'octets lus)
		 	buffer[ecode] = '\0';

			printf("Le serveur reponds : %s FIN\n",buffer);

			//Envoye des données au client
			ecode = write(descSockCOM, buffer, strlen(buffer));
		 	if (ecode == -1) {
				perror("Problème d'ecriture\n");
				exit(10);
		 	}
			//Remet a zero la chaine de caractere tampon
			memset(tampon, 0, sizeof(tampon));

			//Remet a zero la chaine de caractere buffer
			memset(buffer, 0, sizeof(buffer));

			//Lecture des données de la part du client
			ecode = read(descSockCOM, tampon, MAXBUFFERLEN);
		 	if (ecode == -1) {
				perror("Problème de lecture\n");
				exit(9);
			}
		 }
		 if(!strncmp(tampon,"PORT",4)) {
		 	printf("Commande pOrT demandé\n");
			//Decomposition de la commande "PORT n1,n2,...,n6"
			sscanf(tampon,"PORT %d,%d,%d,%d,%d,%d",&n1,&n2,&n3,&n4,&n5,&n6);
			
			//Decodage de l'@ et du numero de port envoyé par le client ftp 
			//L'adresse
			sprintf(addrClient,"%d.%d.%d.%d",n1,n2,n3,n4);
			printf("@ : %s FIN\n",addrClient);

			//Numero de port		
			n5 = (n5*256)+n6;
			sprintf(portClient,"%d",n5);
			printf("@ : %s FIN\n",portClient);

			//Remet a zero les variables n1,n2,n3,n4,n5,n6
			/*memset(&n1, 0);
			memset(&n2, 0);
			memset(&n3, 0);
			memset(&n4, 0, sizeof(&n4));
			memset(&n5, 0, sizeof(&n5));
			memset(&n6, 0, sizeof(&n6));*/
			
			//Preparation de la commande PASV a envoyer au serveur
			strcat(commande,"PASV\0");
			
			printf("%s",commande);
			//Echange de donnée avec le serveur ftp
		 	ecode = write(descSock, commande, strlen(commande));
		 	if (ecode == -1) {
				perror("Problème d'ecriture\n");
				exit(8);
		 	}
			//Lecture des données envoyées par le serveur ftp
			ecode = read(descSock, buffer, MAXBUFFERLEN);
		 	if (ecode == -1) {
				perror("Problème de lecture\n");
				exit(9);
			}
			//Met le '\0' a la fin de la chaine (ecode = nombre d'octets lus)
		 	buffer[ecode] = '\0';
			
			printf("Serveur envoye : %s FIN\n",buffer);
			//Decomposition de la commande "PORT n1,n2,...,n6"
			sscanf(buffer,"%*[^(](%d,%d,%d,%d,%d,%d",&n1,&n2,&n3,&n4,&n5,&n6);

			//Decodage de l'@ et du numero de port envoyé par le serveur ftp
			//L'adresse
			sprintf(addrServeur,"%d.%d.%d.%d",n1,n2,n3,n4);
			printf("@ : %s FIN\n",addrServeur);

			//Numero de port		
			n5 = (n5*256)+n6;
			sprintf(portServeur,"%d",n5);
			printf("@ : %s FIN\n",portServeur);
			
			//Remet a zero la chaine de caractere buffer
			memset(tampon, 0, sizeof(tampon));
		 }
		printf("Commande pas Port demandé\n");

		}
		 //Fermeture de la connexion
		 close(descSockCOM);
		 close(descSockRDV);
}




