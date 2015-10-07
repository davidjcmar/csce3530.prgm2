/* 
David Cmar
CSCE 3530 Program 1
10/8/2015
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#define MESLEN 2048 // message length
#define PORTNUM 56565 // proxy port number

int main (void)
{
	int sock_descrip;
	int size_recv, total_recv=0;
	struct sockaddr_in server;
	char message[MESLEN];

	/* create socket */
	sock_descrip=socket(AF_INET,SOCK_STREAM,0);
	if (sock_descrip==-1)
	{
		printf ("Failed to create socket.\n");
		return 1;
	}
	/* set fields in sockaddr_in struct */
	server.sin_addr.s_addr = inet_addr("129.120.151.94");
	server.sin_family = AF_INET;
	server.sin_port = htons(PORTNUM);

	/* connect to proxy socket */
	if (connect(sock_descrip, (struct sockaddr*)&server, sizeof (server)) < 0)
	{
		printf ("Connection failed.\n");
		return 1;
	}

	printf ("Connected.\n");
	memset(message,'\0',MESLEN); // reset message buffer
	/* receive hello */
	if (read(sock_descrip, message, MESLEN) == -1)
	{
		printf ("Failed to receive message from server.\n");
		return 1;
	}
	printf ("%s\n", message);
	memset(message,'\0',MESLEN); // reset message buffer
	fgets (message, MESLEN, stdin); // get url
	write (sock_descrip, message, strlen(message)); // write url to socket
	/* read response */
	/*if (read(sock_descrip, message,MESLEN) == -1)
	{
		printf ("Failed to receive message from server.\n");
		return 1;
	}*/
	while (1)
	{
		memset (message, '\0', MESLEN);
		if ((size_recv=read(sock_descrip, message, MESLEN)) < 0)
		{
			printf ("Failed to receive message from server.\n");
			return 1;
		}
		else
		{
			printf ("%s",message);
		}
		if (size_recv==0)
			break;
		printf ("size_recv: %d\n",size_recv);
	}
//	printf ("%s", message);

	close (sock_descrip);
	return 0;
}