.c.o:
	gcc -g -c $?

# compile client and server
all: mailclient mailserver

# compile client only
mailclient: mailclient.o mailutils.o
	gcc -g -o mailclient mailclient.o  mailutils.o 

# compile server program
mailserver: mailserver.o mailutils.o
	gcc -g -o mailserver mailserver.o  mailutils.o 
  
