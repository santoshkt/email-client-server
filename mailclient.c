///////////////////////////////////////////////////////////////////////////////
//
// File Name: mailclient.c
// Description: This file contains methods to send and receive emails from
//				client.
// Author: Santosh K Tadikonda, stadikon@gmu.edu
// Date: Dec 1, 2013
// Version: 1.0
//
///////////////////////////////////////////////////////////////////////////////

// Include files.

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include "common.h"

extern int hooktoserver(char *user, char *servhost, ushort servport);

main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	int sock;

	// check usage
	if (argc != 4) {
		fprintf(stderr, "usage : %s <username> <server_ip_address> <5945>\n", argv[0]);
		exit(1);
	}

	// get hooked on to the server
	sock = hooktoserver(argv[1], argv[2], atoi(argv[3]));

	if (sock == -1)
		exit(1);

	fflush(stdout);

	// Initialize FDs to zero.
	// Assign Input FD to client Fds.
	// Assign socked FD to client Fds.

	fd_set clientfds, tempfds;
	FD_ZERO(&clientfds);
	FD_ZERO(&tempfds);
	FD_SET(sock, &clientfds);
	FD_SET(0, &clientfds);

	while (1) {

		// Use tempfds as it will be overwritten on select call
		// We want clientfds to keep track of all connected fds.

		tempfds = clientfds;

		if (select(FD_SETSIZE, &tempfds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// For every fd in the list, if the fd is set, check if
		// that is socket fd. If so, then it means that it we
		// received some message from server. It can be a message
		// from different client or server death. Also if the fd
		// is 0, it means there is some input from the user.
		// Read that input and send it to the server.

		int fd;
		for (fd = 0; fd < FD_SETSIZE; fd++) {
			if (FD_ISSET(fd,&tempfds)) {
				if (fd == sock) {
					Packet *pkt;
					pkt = recvpkt(sock);
					if (!pkt) {

						// server killed, exit
						fprintf(stderr, "Server closed the connection.\n");
						exit(1);
					}

					// display the text
					if (pkt->type == EMAIL_MSG_TO_CLIENT) {
						printf("New email received !\n>> %s\n", pkt->text);
					}else if(pkt->type == WELCOME_MSG){

						// Received welcome message. Print it.
						printf(">> %s\n", pkt->text);

						// Send username to client.
						char msg[MAXMSGLEN];
						strcpy(msg, argv[1]);

						// fprintf(stderr, "user: %s", msg);
						sendpkt(sock, USER_NAME, strlen(msg) + 1, msg);

					}else if(pkt->type == SERVER_ERROR) {

						printf(">> %s\n", pkt->text);
					}
					else{
						fprintf(stderr, "error: unexpected reply from server\n");
						exit(1);
					}

					// free the message
					freepkt(pkt);
				}

				if (fd == 0) {
					char msg[MAXMSGLEN];

					if (!fgets(msg, MAXMSGLEN, stdin))
						exit(0);

					if (strncmp(msg, QUIT_STRING, strlen(QUIT_STRING)) == 0) {
						sendpkt(sock, CLOSE_CON, 0, NULL);
						break;
					}

					char * pch1, *pch2, *user, *ipaddr;
					pch1 = strstr(msg, " ");
					pch2 = strstr(msg, "@");

					if (pch1 == NULL || pch2 == NULL) {
						fprintf(stderr,
								"error: invalid e-mail format.\nsyntax: <recp_user>@<ip_addr> <mesg>\n");
						break;
					}

					if (pch1 < pch2)
					{
						fprintf(stderr, "error: user name cannot contain spaces.\n");
						fprintf(stderr,
								"error: invalid e-mail format.\nsyntax: <recp_user>@<ip_addr> <mesg>\n");
						break;
					}

					user = (char *) malloc((strlen(msg) - strlen(pch2) + 1)*sizeof(char));
					strncpy(user, msg, (strlen(msg) - strlen(pch2))*sizeof(char));
					user[strlen(msg) - strlen(pch2)] = '\0';

					// fprintf(stderr, "client: user: %s", user);

					if(strcmp(user,"") == 0)
					{
						fprintf(stderr, "error: no username entered.\n");
						free(user);
						break;
					}

					ipaddr = (char *) malloc(
							(strlen(pch2) - strlen(pch1))* sizeof(char));
					strncpy(ipaddr, msg + strlen(user) + 1,
							(strlen(pch2) - strlen(pch1) - 1) * sizeof(char));
					ipaddr[strlen(pch2) - strlen(pch1) - 1] = '\0';

					// fprintf(stderr, "server: ip: %s", ipaddr);

					struct sockaddr_in sa;
					int result = inet_pton(AF_INET, ipaddr, &(sa.sin_addr));
					if(result == 0){
						fprintf(stderr,
								"error: invalid ip-address format.\nsyntax: <recp_user>@<ip_addr> <mesg>\n");
						free(user);
						free(ipaddr);
						break;
					}

					char * mailmsg;
					mailmsg = (char *) malloc(strlen(pch1) * sizeof(char));
					strncpy(mailmsg,
							msg + strlen(user) + strlen(ipaddr) + 2,
							(strlen(pch1) - 1) * sizeof(char));
					mailmsg[strlen(pch1)-1] = '\0';

					// It is okay to have empty body in the email message.

					if(strlen(mailmsg) > 80)
					{
						fprintf(stderr, "error: mail message length can be atmost 80 characters.\n");
						free(user);
						free(ipaddr);
						free(mailmsg);
						break;
					}

					// fprintf(stderr, "server: mailmsg: %s", mailmsg);

					free(user);
					free(ipaddr);
					free(mailmsg);

					msg[strlen(msg) - 1] = '\0';
					sendpkt(sock, EMAIL_MSG_TO_SERVER, strlen(msg) + 1, msg);

				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
