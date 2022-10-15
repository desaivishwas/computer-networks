/********************************************/
/* file.c - Vishwas Desai (visdesai) */
/* Modified: 10/19/2021 */
/* Submission for Lab 4 - Applying Sockets */
/*********************************************/

#include "file.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>

/*
 *  Here is the starting point for your netster part.2 definitions. Add the 
 *  appropriate comment header as defined in the code formatting guidelines
 */

/* Add function definitions */
void file_server(char* iface, long port, int use_udp, FILE* fp) {
	int sockfd;
	struct sockaddr_in server_a;

	server_a.sin_family = AF_INET;
	server_a.sin_addr.s_addr = INADDR_ANY;
	server_a.sin_port = htons(port);
	
	// UDP Connection
	if (use_udp ==  1)
	{
		int sockfd_udp = socket(AF_INET, SOCK_DGRAM,0);
		int x = bind(sockfd_udp, (struct sockaddr *)&server_a, sizeof(server_a));			
		if(x<0)
		{
			perror("ERROR:Bind Failed for UDP\n");
		}
		int file_content;
		struct sockaddr_in client_a;
		socklen_t  client_size = sizeof(client_a);
		size_t content;
		char new_buff[256];
		// recvfrom in UDP
		while((file_content = recvfrom(sockfd_udp, new_buff, 255, 0, (struct sockaddr *)&client_a, &client_size))>0)
		{
			printf("Receiving file from Client\n");
			content = fwrite(new_buff, sizeof(char), file_content, fp);
			// file error handling
			if (content < 255 || file_content == EOF)
			{
				//perror("Error\n");
				break;
			}
			bzero(new_buff, sizeof(new_buff));
		}
	}
	// TCP Connection
	else
	{
		/* creating a scoket */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		/* binding */
		int x = bind(sockfd, (struct sockaddr *)&server_a, sizeof(server_a));
		if(x<0)
		{
			perror("Error: Bind failed for TCP\n");
		}
		/* printf("Binding");*/
		int l = listen(sockfd, 5);
		if (l<0)
		{
			perror("ERROR: Listen failed for TCP\n");
		}
		/* printf("Listneining...");*/

		int client_sockfd;
		struct sockaddr_in client_a;
		socklen_t client_size = sizeof(client_a);
		int file_content;
	
		client_sockfd = accept(sockfd, (struct sockaddr *)&client_a, &client_size);
		printf("connection from ('%s', %d)\n", inet_ntoa(client_a.sin_addr), ntohs(client_a.sin_port));
		size_t  content;
		char buff[256];
	
		while((file_content = read(client_sockfd, buff, 255))>0)
		{
			printf("Receiving file from server\n");
			content = fwrite(buff, sizeof(char), file_content, fp);
			// file error handling
			if (content < 255 || file_content==EOF)
			{
				//perror("Error");
				break;
	
			}
			bzero(buff, sizeof(buff));
		}
      }
}

void file_client(char* host, long port, int use_udp, FILE* fp) {
 	int sockfd_cli;
	struct sockaddr_in server_a;

    	/* resolving the loopback IP */
	int len = strlen("localhost");
	if (strncmp("localhost", host, len) == 0)
	{
		host = "127.0.0.1";
	}

	char* ip = "127.0.0.1";
	server_a.sin_family = AF_INET;
	server_a.sin_addr.s_addr = inet_addr(ip);
	server_a.sin_port = htons(port);

	// UDP Connection	
	if (use_udp == 1)
	{
		int sockfd_cli_udp = socket(AF_INET, SOCK_DGRAM, 0);
		char new_buff[256];
		size_t content;
		bzero(new_buff, sizeof(new_buff));
		while((content = fread(new_buff, sizeof(char), 255, fp))>0)
		{
			// sendto in UDP
			sendto(sockfd_cli_udp, new_buff, content, 0, (struct sockaddr *)&server_a, sizeof(server_a));
			printf("Sending file to server\n");
		} 
	}
	// TCP Connection
	else
	{
		sockfd_cli = socket(AF_INET, SOCK_STREAM, 0);
		connect(sockfd_cli, (struct sockaddr *)&server_a, sizeof(server_a));	
		size_t content;
		char buff[256];
		while((content = fread(buff, sizeof(char), 255, fp))>0)
		{
			send(sockfd_cli, buff, content, 0);
			printf("Sending file to server\n");
		}
	} 
}
