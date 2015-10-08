/* 
David Cmar
CSCE 3530 Program 2
10/8/2015
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MESLEN 1000000 // message length
#define PORTNUM 56565 // server port number

typedef struct {
	char url[256];
	char data[256];
}CACHE;
/* parse url from client for GET request */
void parse_client (char* message, char* url, char* host)
{
//	printf ("message length: %d\n",strlen(message));
	int i,j;
	int boolean=0;

	memset(host,'\0',256);
	memset(url,'\0',MESLEN-256);

	/* if message does not end in '/' */
	if (message[strlen(message)-2]!='/')
	{
		/* set last char to '/' temporarily */
		message[strlen(message)-1]='/';
		boolean=1; // set flag
	}
	/* pull host until '/' */
	for (i=0;message[i]!='/';i++)
		host[i]=message[i];

	message[strlen(message)-1]='\n'; // set last char to check against
	
	/* pull url until newline */
	for(j=0;message[i]!='\n';i++,j++)
		url[j]=message[i];

	/* set end of string char */
//	printf ("url strlen: %d\n",strlen(url)); //testing
	if (strlen(url)==0)
		url[0]='/';
	else
		url[strlen(url)]='\0';

}

/* construct HTTP request */
void request (char* message, char* url, char* host)
{
	memset(message,'\0',MESLEN);

	strcpy(message,"GET ");
	strcat(message,url);
	strcat(message," HTTP/1.1\r\n");
	strcat(message,"Host: ");
	strcat(message,host);
	strcat(message,"\r\n\r\n");
}
/* check if url is blackisted. return 1 if blacklisted, 0 if not */
int chk_blist (char* message, char blist[][256])
{
	int i;
	char buffer[strlen(message)];
	memset (buffer,'\0',strlen(message));
	strcpy (buffer,message);
	i=0;
	buffer[strlen(buffer)-1]='\n';
	while (i<25 && strlen(blist[i])!=0)
	{
		printf ("%s", blist[i]);
		if (strcmp (buffer,blist[i])==0)
			return 1;
		i++;
	} 
	return 0;
}
/* check if page is in cache */
int chk_cache (CACHE* cache_list, char* message)
{

}
/* set/delete values in cache and cached files */
void set_cache (FILE* f_cache, CACHE* cache_list)
{

}
/* main function */
int main (void)
{
	int sock_descript, sock_cli_ser, sock_inet, size;
	int i, j, boolean, check_blist;
	int size_recv;
	struct sockaddr_in server, client, proxy;
	char message[MESLEN], url[MESLEN-256], host[256], buffer[MESLEN],cache_buffer[256];
	struct hostent* he;
	struct in_addr** addr_list;
	char ip_addr[50];
	char blist[25][256];
	FILE *f_blist, *f_cache, *f_buffer;
	CACHE* cache_list[5];

	/* open file pointers to cache and blacklist */
	f_cache=fopen("list.txt","r");
	f_blist=fopen("blacklist.txt","r");
	/* init blacklist */
	for (i=0;i<25;i++)
	{
		memset(blist[i],'\0',256);
	}
	i=0;
	while (i<25 && fgets(blist[i],256,f_blist)!=NULL)
	{
		if (blist[i][strlen(blist[i])-1]=='\n')	
		{
			blist[i][strlen(blist[i])-1]='\0'; //overwrite newline
		}
		i++;
	}
	/* init cache */
	memset (cache_buffer,'\0',256);
	for (i=0;i<5;i++)
	{
		cache_list[i]= (CACHE*)malloc(sizeof(CACHE));
		memset (cache_list[i]->url,'\0',256);
		memset (cache_list[i]->data,'\0',256);
		if (fgets(cache_list[i]->url,256,f_cache)!=NULL)
		{
			fgets(cache_list[i]->data,256,f_cache);
			if (cache_list[i]->url[strlen(cache_list[i]->url)-1]=='\n')
				cache_list[i]->url[strlen(cache_list[i]->url)-1]='\0'; //overwrite newline
			if (cache_list[i]->data[strlen(cache_list[i]->data)-1]=='\n')
				cache_list[i]->data[strlen(cache_list[i]->data)-1]='\0'; //overwrite newline
//			printf ("%s",cache_list[i]->data);
//			f_buffer=fopen(cache_list[i]->data,"r");
		}
	}
	fclose (f_blist);
	fclose (f_cache);
	/* create socket to client */
	sock_descript=socket(AF_INET,SOCK_STREAM,0);
	if (sock_descript==-1)
	{
		printf ("Failed to create socket.\n");
		return 1;
	}

	/* set fields in sockaddr_in struct */
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(PORTNUM);

	/* bind socket */
	if (bind(sock_descript,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf ("Bind failed.\n");
		close (sock_descript);
		return 1;
	}

	printf ("Bind successful.\n");

	/* listen */
	listen (sock_descript,3);

	printf ("Ready for incoming connection.\n");
	size=sizeof (struct sockaddr_in);
	sock_cli_ser=accept(sock_descript, (struct sockaddr *)&client, (socklen_t *)&size);
	if (sock_cli_ser < 0)
	{
		printf ("Connection not accepted.\n");
		close (sock_descript);
		close (sock_cli_ser);
		return 1;
	}
	printf ("Connection accepted.\n");

	/* communicate with client */
	memset(message,'\0',MESLEN);
	strcpy (message,"Enter a URL for which you want a HTTP request (do not include http://): ");
	write (sock_cli_ser, message, strlen(message));
	memset(message,'\0',MESLEN);
	read (sock_cli_ser, message, 256);


	/* check blacklist */
	check_blist = chk_blist(message,blist);
	if (check_blist==1)
	{
		strcpy (message,"That URL is blacklisted.");
		write(sock_cli_ser,message, strlen(message));
		return 1;
	}

	/* check cache */

//	printf ("message:%s\n", message); // testing
	parse_client (message, url, host);
	printf ("url: %s\thost: %s\n",url,host);





	if (check_blist==0)
	{
		/* find ip addess based on host */
		if ((he = gethostbyname(host))==NULL)
		{
			printf ("Get host by name failed");
			close (sock_descript);
			close (sock_cli_ser);
			return 1;
		}
		addr_list = (struct in_addr **) he->h_addr_list;
		/* add ip to addr_list */
		for (i=0; addr_list[i]!=NULL;i++)
		{
			strcpy (ip_addr,inet_ntoa(*addr_list[i]));
		}
		printf ("host: %s\t resolved to: %s\n", host, ip_addr); //testing

		/* create socket to inet */
		sock_inet=socket(AF_INET,SOCK_STREAM,0);
		if (sock_inet==-1)
		{
			printf ("Failed to create socket.\n");
			close (sock_descript);
			close (sock_cli_ser);
			return 1;
		}
		/* set fields in sockaddr_in struct */
		proxy.sin_family=AF_INET;
		proxy.sin_addr.s_addr= inet_addr(ip_addr);
		proxy.sin_port=htons(80);
		/* bind socket */
		if (connect(sock_inet,(struct sockaddr*)&proxy, sizeof(proxy)) < 0)
		{
			printf ("Connection failed.\n");
			close (sock_descript);
			close (sock_cli_ser);
			close (sock_inet);
			return 1;
		}
		printf ("Connected.\n");
		request(message,url,host);
		printf ("%s", message);

		/* send request to webserver on port 80 */
		if (send(sock_inet,message,strlen(message),0) < 0)
		{
			printf ("Request failed.\n");
			close (sock_descript);
			close (sock_cli_ser);
			close (sock_inet);
			return 1;
		}
		memset (buffer,'\0',MESLEN); // reset buffer
		/* accept response from webserver */
		/*if (recv(sock_inet,buffer,MESLEN,0) < 0)
		{
			printf ("No reply from webserver.\n");
			close (sock_descript);
			close (sock_cli_ser);
			close (sock_inet);
			return 1;
		}*/

		size_recv=0;
		memset (message, '\0', MESLEN);
		size_recv=recv(sock_inet,buffer,MESLEN,MSG_PEEK);
		if (size_recv < 0)
		{
			printf ("No reply from webserver.\n");
			close (sock_descript);
			close (sock_cli_ser);
			close (sock_inet);
			return 1;
		}
		else
		{
			recv (sock_inet, buffer, MESLEN, 0);
			write (sock_cli_ser, buffer, strlen(buffer));
		}

	//	printf ("size_recv: %d\n",size_recv);
		
	//	printf ("%s",buffer); //testing 
	//	printf ("strlen: %d\n",strlen(buffer)); //testing

		/* write request to client socket */
	//	write (sock_cli_ser, buffer, strlen(buffer));
	}
	close (sock_inet);
	close (sock_descript);
	close (sock_cli_ser);
	return 0;
}