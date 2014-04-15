///////////////////////////////////////////////////////////////////////////////
//
// File Name: mailserver.c
// Description: This file contains methods to send and receive emails from
//				server.
// Author: Santosh K Tadikonda, stadikon@gmu.edu
// Date: Dec 1, 2013
// Version: 1.0
//
///////////////////////////////////////////////////////////////////////////////

// Include files

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include "common.h"

// info about a client
typedef struct _member {

	// member name
	char * name;

	// member socket
	int sock;

	// member ip-address
	char * ipaddr;

	// next member
	struct _member * next;

	// prev member
	struct _member * prev;

} Member;

// info about a mail
typedef struct _mail {

	int mailid;

	// recipient name
	char * name;

	// recipient ip address
	char * ipaddr;

	// sender name
	char * sendername;

	// sender ip
	char * senderip;

	char * message;

	// next member
	struct _mail * next;

	// prev member
	struct _mail * prev;

} Mail;

// Global Variables
Member * memblist = NULL;
Mail * maillist = NULL;
int globalmailid = 0;

// find the member with given name
Member *findmemberbyname(char *name) {
	Member *memb;
	// go thru all members
	for (memb = memblist; memb; memb = memb->next) {
		if (strcmp(memb->name, name) == 0)
			return (memb);
	}
	return (NULL);
}

// find the member by name and ip
Member *findmembbynameip(char *name, char *ip) {
	// printf("findmembbynameip(%s, %s)\n", name, ip);
	Member *memb;
	for (memb = memblist; memb; memb = memb->next) {
		if(memb->name == NULL){
			// printf("Software error. Name NULL or yet to be updated.\n");
		} else if(memb->ipaddr == NULL){
			// printf("Software error. ipaddr NULL\n.");
		}
		else if ((strcmp(memb->name, name) == 0) && (strcmp(memb->ipaddr, ip) == 0))
			return (memb);
	}
	return (NULL);
}

// find the member with given sock
Member *findmemberbysock(int sock) {
	// printf("findmemberbysock(%d)\n", sock);
	Member *memb;

	// go thru all members
	for (memb = memblist; memb; memb = memb->next) {
		if (memb->sock == sock)
			return (memb);
	}
	return (NULL);
}

// add a member with given sock and ipaddr to the list. name will be added
// later.
int addmember(int sock, char * ipaddr) {

	// printf("addmember(%d, %s).\n", sock, ipaddr);
	Member * memb;

	// make this a member of the group
	memb = (Member *) calloc(1, sizeof(Member));
	if (!memb) {
		fprintf(stderr, "error : unable to calloc\n");
		exit(0);
	}
	memb->name = NULL;
	memb->sock = sock;
	memb->ipaddr = strdup(ipaddr);
	memb->prev = NULL;
	memb->next = memblist;
	if (memblist) {
		memblist->prev = memb;
	}
	memblist = memb;
	return 1;
}

// updated the name of the member connected on the given socket descriptor.
int updatemember(int sock, char *mname) {
	// printf("updatemember (%d, %s)\n", sock, mname);
	Member * memb;
	memb = findmemberbysock(sock);

	if (!memb) {
		return 0;
	}

	memb->name = strdup(mname);
	return 1;
}

// delete the member connected on given socket.
int deletemember(int sock) {
	// printf("deletemember(%d)\n", sock);
	Member * memb;

	// get hold of the member
	memb = findmemberbysock(sock);
	if (!memb) {
		return (0);
	}

	// exclude from the group
	if (memb->next) {
		memb->next->prev = memb->prev;
	}

	// remove from list
	if (memblist == memb) {
		// the head of list
		memblist = memb->next;
	} else {
		// the middle of list
		memb->prev->next = memb->next;
	}

	// free up member
	free(memb->name);
	free(memb->ipaddr);
	free(memb);
	return (1);
}

// displays the list of members connected and have their name updated.
int listmembers() {
	Member * memb;
	printf("================\nList of members\n================\n");
	for (memb = memblist; memb; memb = memb->next) {
		if(memb->name)
			printf("%s\n", memb->name);
	}
	printf("================\n");
}


// find the mail with given mail id
Mail *findmailbyid(int mailid) {
	Mail *mail;

	// go thru all members
	for (mail = maillist; mail; mail = mail->next) {
		if (mail->mailid == mailid)
			return (mail);
	}
	return (NULL);
}

// add the mail to the list with given name, ip and message.
int addmail(int sendersock, char *mname, char* ipaddr, char* mailmsg) {

	// printf("addmail(%s, %s, %s)\n", mname, ipaddr, mailmsg);

	Mail * mail;
	mail = (Mail *) calloc(1, sizeof(Mail));
	if (!mail) {
		fprintf(stderr, "error : unable to calloc mail\n");
		exit(0);
	}

	globalmailid++;
	mail->mailid = globalmailid;
	mail->name = strdup(mname);
	mail->message = strdup(mailmsg);
	mail->ipaddr = strdup(ipaddr);

	Member * sender;
	sender = findmemberbysock(sendersock);

	if(sender != NULL)
	{
		mail->sendername = strdup(sender->name);
		mail->senderip = strdup(sender->ipaddr);
	}
	else
	{
		// It can be the case that sender got disconnected
		// before the mail details are updated.
		mail->sendername = NULL;
		mail->senderip = NULL;
	}

	mail->prev = NULL;
	mail->next = maillist;
	if (maillist) {
		maillist->prev = mail;
	}
	maillist = mail;
}

// delete the email with given mail id from the list.
int deletemail(int mailid) {
	// printf("deletemail(%d)", mailid);
	Mail * mail;

	// get hold of the mail
	mail = findmailbyid(mailid);
	if (!mail) {
		return (0);
	}

	// exclude from the group
	if (mail->next) {
		mail->next->prev = mail->prev;
	}

	// remove from list
	if (maillist == mail) {
		// the head of list
		maillist = mail->next;
	} else {
		// the middle of list
		mail->prev->next = mail->next;
	}

	// free up mail
	free(mail->name);
	free(mail->ipaddr);
	free(mail->message);
	free(mail->sendername);
	free(mail->senderip);
	free(mail);
	return (1);
}

// displays all emails available.
int listmails() {
	Mail * mail;
	printf("\n================\nList of emails\n================\n");
	for (mail = maillist; mail; mail = mail->next) {
		printf("Recipient name: %s\nIP: %s\nMessage Body: %s\n================\n",
				mail->name, mail->ipaddr, mail->message);
	}
	printf("================\n");
}

// displays all lists available.
int listall()
{
	listmembers();
	listmails();
}

// sends pending emails to the connected clients and deletes the email from
// the email list.
int sendmails() {
	// printf("sendmails(): method entry\n");

	Mail * mail, * nextmail;
	Member * memb;
	for (mail = maillist; mail; ) {
		nextmail = mail->next;
		if ((memb = findmembbynameip(mail->name, mail->ipaddr)) != NULL) {

			char outputmail[MAXPKTLEN];
			strcpy(outputmail, "From: ");
			strcat(outputmail, mail->sendername);
			strcat(outputmail, "@");
			strcat(outputmail, mail->senderip);
			strcat(outputmail, "\n");
			strcat(outputmail, mail->message);
			sendpkt(memb->sock, EMAIL_MSG_TO_CLIENT, strlen(outputmail) + 1, outputmail);

			if(!deletemail(mail->mailid))
			{
				fprintf(stderr, "error: unable to delete delivered email.\n");
			}

		}
		mail = nextmail;
	}
}

// sends emails on time out and again starts the timer.
void handletimeout() {

	// printf("handletimeout(): method entry\n");

	// Look for emails and send them to clients.
	sendmails();

	if (!starttimer()) {
		fprintf(stderr, "could not start timer.");
	}
	return;
}

// starts a timer with 30 seconds.
int starttimer() {

	// printf("starttimer(): method entry\n");
	signal(SIGALRM, handletimeout);
	alarm(30);
	return 1;
}

main(int argc, char *argv[]) {

	setbuf(stdout, NULL);
	// server socket descriptor
	int servsock;

	// set of live client sockets
	fd_set livesdset, tempset;

	// largest live client socket descriptor
	int livesdmax;

	// check usage
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	// get ready to receive requests
	servsock = startserver();
	if (servsock == -1) {
		exit(1);
	}

	if (!starttimer()) {
		fprintf(stderr, "error: could not start timer.\n");
	}

	// startserver also binds and listens.


	FD_ZERO(&livesdset);
	FD_ZERO(&tempset);
	FD_SET(servsock, &livesdset);
	FD_SET(0, &livesdset);


	livesdmax = servsock;

	// Timed expiry of select is not required in current context.
	// struct timeval tv;
	// tv.tv_sec = 2;
	// tv.tv_usec = 500000;

	// receive requests and process them
	while (1) {

		// loop variable
		int frsock;

		tempset = livesdset;

		// wait using select() for
		// messages from existing clients and
		// connect requests from new clients

		int pret;
		pret = select(livesdmax + 1, &tempset, NULL, NULL, NULL);
		if (pret == 0)
		{
			// printf("No SD is set, lets continue..\n");
			continue;
		}
		else if( pret < 0 && errno == EINTR )
		{
			// An Interupt because of timer expiry is expected.
			continue;
		}
		else if( pret < 0 && errno != EINTR )
		{
			printf("Select return code is %d\n", pret);
			printf("Oh dear, something went wrong with select()! %s\n", strerror(errno));
			continue;
		}

		// look for messages from live clients
		for (frsock = 0; frsock <= livesdmax; frsock++) {
			// skip the listen socket
			// this case is covered separately
			if (frsock == servsock) {
				if (FD_ISSET(frsock, &tempset)) {
					// printf("Its a server fd %d\n", frsock);
				}
				continue;
			}

			if ( FD_ISSET(frsock, &tempset)) {

				if(frsock == 0){
					char intxt[MAXMSGLEN];

					if (!fgets(intxt, MAXMSGLEN, stdin))
						exit(0);

					if (strncmp(intxt, "list", 4) == 0) {
						listall();
						continue;
					}else
					{
						fprintf(stderr, "error: invalid command.\n");
						continue;
					}
				}

				Packet *pkt;

				// read the message
				pkt = recvpkt(frsock);

				if (!pkt) {

					// Delete this member from the list.
					deletemember(frsock);

					// remove this guy from the set of live clients
					FD_CLR(frsock, &livesdset);

					// close the socket
					close(frsock);
				} else {
					char* mname;

					// take action based on messge type
					switch (pkt->type) {
						case USER_NAME:
							mname = pkt->text;

							Member *memb;
							memb = findmemberbysock(frsock);
							if( memb == NULL )
							{
								fprintf(stderr,"error: member does not exist with this socket.\n");
								continue;
							}
							else
							{
								Member * samememb;
								samememb = findmembbynameip(mname, memb->ipaddr);
								if ( samememb != NULL )
								{
									fprintf(stderr, "error: user %s already connected from %s\n", mname, memb->ipaddr);
									char bufr[MAXPKTLEN] = "user with same username already connected from this machine.\0";
									sendpkt(frsock, SERVER_ERROR, strlen(bufr) + 1, bufr);
									deletemember(frsock);
									FD_CLR(frsock, &livesdset);

									// close the socket
									close(frsock);
									continue;
								}
							}

							if(!updatemember(frsock, mname))
							{
								fprintf(stderr,"error: unable to update member name.\n");
								continue;
							}
							break;
						case EMAIL_MSG_TO_SERVER:
							{
								char * msg;
								msg = pkt->text;
								char * pch1, *pch2, *user, *ipaddr, *mailmsg;
								pch1 = strstr(msg, " ");
								pch2 = strstr(msg, "@");

								// All these checks are already available on
								// client but making sure so that we don't run
								// into any issues.

								if (pch1 == NULL || pch2 == NULL) {
									fprintf(stderr,
											"error: invalid e-mail format. ignoring mail.\n");
									continue;
								}

								if (pch1 < pch2)
								{
									fprintf(stderr, "error: user name cannot contain spaces.\n");
									fprintf(stderr, "error: invalid e-mail format. ignoring mail.\n");
									continue;
								}


								user = (char *) malloc((strlen(msg) - strlen(pch2) + 1)*sizeof(char));
								strncpy(user, msg, (strlen(msg) - strlen(pch2))*sizeof(char));
								user[strlen(msg) - strlen(pch2)] = '\0';

								// fprintf(stderr, "server: user: %s", user);

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
											"error: Invalid IP address format. Ignoring email.\n");
									free(user);
									free(ipaddr);
									continue;
								}


								mailmsg = (char *) malloc(strlen(pch1) * sizeof(char));
								strncpy(mailmsg,
										msg + strlen(user) + strlen(ipaddr) + 2,
										(strlen(pch1) - 1) * sizeof(char));
								mailmsg[strlen(pch1)-1] = '\0';

								// fprintf(stderr, "server: mailmsg: %s", mailmsg);

								addmail(frsock, user, ipaddr, mailmsg);

								// make sure all temporary char arrays are freed.
								free(user);
								free(ipaddr);
								free(mailmsg);

							}
							break;
						case CLOSE_CON:
							deletemember(frsock);
							FD_CLR(frsock, &livesdset);

							// close the socket
							close(frsock);
							break;
						default:
							printf("Unexpected message type. Dont know how to handle.\n");
					}

					// free the message
					freepkt(pkt);
				}
			}
		}

		// client address
		struct sockaddr_in remoteaddr;
		socklen_t addrlen;
		int csd;

		// look for connect requests
		if (FD_ISSET(servsock, &tempset)) {
			// fprintf(stderr,"About to accept request.\n");
			addrlen = sizeof remoteaddr;
			csd = accept(servsock, (struct sockaddr *) &remoteaddr, &addrlen);
			// fprintf(stderr,"Accepted request.\n");

			// if accept is fine?
			if (csd != -1) {
				// printf("Sending welcome message.\n");
				char bufr[MAXPKTLEN] = "Welcome to Santosh\'s Email Server, running on port 5945.\0";
				sendpkt(csd, WELCOME_MSG, strlen(bufr) + 1, bufr);

				// printf("Sent welcome message.\n");
				char str[INET_ADDRSTRLEN];

				inet_ntop(AF_INET, &(remoteaddr.sin_addr), str,
						INET_ADDRSTRLEN);

				// Add client to member list. We will update member name later.
				addmember(csd, str);

				// add this guy to set of live clients
				FD_SET(csd, &livesdset);
				if (csd > livesdmax) {
					livesdmax = csd;
				}
			} else {
				perror("accept");
				exit(0);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
