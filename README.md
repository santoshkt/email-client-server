* What is in this directory? 

  * README           this file
  * mailclient.c     contains source code of mail client
  * mailserver.c     contains source code of mail server
  * mailutils .c     routines used by server and client
  * common.h         header file included by all .c files
  * makefile         make file to build mailserver and mailclient

* How do i compile my programs? 

  The 'makefile' is provided just for that purpose. To compile, run

    % make

  It will create 'mailserver' and 'mailclient'.

* How do i run these programs? 

  The 'mailserver' program does not have any arguemnts. 
  For example you can run it as follows.

    % mailserver
 
  The 'mailclient' program takes the username, ip adress and port no in
  following format.

    % mailclient <user>@<ip-addr> <port>

  You run the server first and then many clients. Once the clients are
  connected, you can give list command on server to see list of members
  connected and listing of pending emails to be sent out. For every 30
  seconds, the server sends out the pending emails. Same user cannot login
  into a machine again. Users running with same name and on different
  machines are both physically and technically different users.
  
  You can send an email to user with ip-address in following format
  
  	% <user>@<ip-addr> <message>
  	
  username cannot ocntain spaces. IP address should be in valid IP4
  format. Message cannot be longer than 80 characters. user will receive
  this message when logs into a machine with the ip-address mentioned
  only.

* How do i exit from these programs? 

  In  case of  client, you can exit by send 'close' command.
  In case of server, you can just press Ctrl-C to kill it. 

* Do these programs run on any machine? 

  They are compiled and will work on any 32-bit and 64-bit Linux machines. No warranty is given for
  other kinds of machines.
  
