///////////////////////////////////////////////////////////////////////////////
//
// File Name: commonh.
// Description: This file contains methods and definitions which are common to
//				both cient and server.
// Author: Santosh K Tadikonda, stadikon@gmu.edu
// Date: Dec 1, 2013
// Version: 1.0
//
///////////////////////////////////////////////////////////////////////////////

// buffer limits
#define MAXNAMELEN 256
#define MAXPKTLEN  2048
#define MAXMSGLEN  1024

// defined strings
#define QUIT_STRING "close"

// messsges received/sent by server or client
#define WELCOME_MSG    0
#define USER_NAME     1
#define EMAIL_MSG_TO_SERVER    2
#define EMAIL_MSG_TO_CLIENT 3
#define CLOSE_CON 4
#define SERVER_ERROR 5

// structure of a packet
typedef struct _packet {

	// packet type
	uint8_t     type;

	// packet length
	uint32_t      lent;

	// packet text
	char *    text;

} Packet;

extern int startserver();
extern Packet *recvpkt(int sd);
extern int sendpkt(int sd, uint8_t typ, uint32_t len, char *buf);
extern void freepkt(Packet *msg);

///////////////////////////////////////////////////////////////////////////////
