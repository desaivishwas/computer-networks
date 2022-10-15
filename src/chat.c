/*
 *  chat.c - Vishwas Desai (visdesai)
 *  Created - 09/24/2021
 *  03_sockets - Fun with Sockets!
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
// header file
#include "chat.h"

// Multithreading struct
typedef struct multi_thread_data{
  pthread_t *ptid;
  int client_sockfd;
  struct sockaddr_in client_a;
} multi_thread_data;

void* multithreading(void *arg);

void* multithreading(void *arg)
{
	multi_thread_data *thread_data = (multi_thread_data *)arg;
	int client_sockfd = thread_data->client_sockfd;
	struct sockaddr_in client_a = thread_data->client_a;
	for(;;)
	{  
		char buff[255];
		bzero(buff, sizeof(buff));
		
		int msg = read(client_sockfd, buff, 255);
		/* if (msg<0){perror("Invalid");} */
		printf("got message from ('%s', %d) \n", inet_ntoa(client_a.sin_addr), ntohs(client_a.sin_port));
		
		if(strncmp("hello", buff, strlen("hello")) == 0){
			msg = write(client_sockfd, "world\n", strlen("world\n"));
		}
		else if(strncmp("goodbye", buff, strlen("goodbye")) == 0){
			msg = write(client_sockfd, "farewell\n", strlen("farewell\n"));
		        break;
		}
		else if(strncmp("exit", buff, strlen("exit")) == 0){
			msg = write(client_sockfd, "ok\n", strlen("ok\n"));
			close(client_sockfd);
			exit(0);
		}
		else {
      		  msg = write(client_sockfd, buff, 255);
		}
                if (msg<0)
		{
 			perror("Error");
		}
        /* if(msg<0){perror("Inavlid");} */
		bzero(buff, sizeof(buff));
	}
	return NULL;
}

/* server */
void chat_server(char* iface, long port, int use_udp)
{
	int sockfd;
	struct sockaddr_in server_a;
	int inc = -1;
	server_a.sin_family = AF_INET;
	server_a.sin_addr.s_addr = INADDR_ANY;
	server_a.sin_port = htons(port);

 /* TCP & UDP */
 // default tcp cpnnection when use_udp == 0
	if (use_udp == 0)
	{
		/* creating a scoket */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		/* binding */
		bind(sockfd, (struct sockaddr *)&server_a, sizeof(server_a));
		/* printf("Binding");*/
		listen(sockfd, 5);
		/* printf("Listneining...");*/
		for(;;)
			{
				int client_sockfd;
				struct sockaddr_in client_a;
				socklen_t client_size = sizeof(client_a);
				client_sockfd = accept(sockfd, (struct sockaddr *)&client_a, &client_size);
				inc++;
				printf("connection %d from ('%s', %d)\n", inc, inet_ntoa(client_a.sin_addr), ntohs(client_a.sin_port));
				// creating and processing threads
				pthread_t thread_1;
				multi_thread_data thread_data;
				thread_data.client_sockfd = client_sockfd;
				thread_data.client_a = client_a;
				thread_data.ptid = &thread_1;
				pthread_create(&thread_1, NULL, multithreading, (void *)&thread_data);
			}
	// end  of TCP
	}

	// UDP 
	else
	{
		int sockfd_udp = socket(AF_INET, SOCK_DGRAM,0);
		int x = bind(sockfd_udp, (struct sockaddr *)&server_a, sizeof(server_a));
		int hello = strlen("hello");
		int goodbye = strlen("goodbye");
		int exit = strlen("exit");
		if (x<0)
		{
			perror("Invalid");
		}
		//printf("%d\n",x);
		for(;;)
		{
			char new_buff[255];
			//printf("Inside for loop\n");
			struct sockaddr_in client_a;
			socklen_t  client_size = sizeof(client_a);
			inc++;
			bzero(new_buff, sizeof(new_buff));
			
			// recvfrom for udp  is used to receive data on a socket whether or not connected
			int msg = recvfrom(sockfd_udp, new_buff, 255, 0, (struct sockaddr *)&client_a, &client_size);
			// error handling for buff
			if (msg <0)
			{
				perror("Invalid\n");
			}
			if (strlen(new_buff) > sizeof(new_buff))
			{
				perror("Invalid\n");
			}
			printf("got message from ('%s', %d)\n", inet_ntoa(client_a.sin_addr), ntohs(client_a.sin_port));
			if (strncmp("hello", new_buff,hello)==0)
			{
				strcpy(new_buff, "world\n");
				// sendto() genrally used for UDP connections
				sendto(sockfd_udp,new_buff, msg, 0,(struct sockaddr *)&client_a, client_size);
			}
			else if (strncmp("goodbye", new_buff, goodbye)==0)
			{
				strcpy(new_buff, "farewell\n");
				sendto(sockfd_udp, new_buff, msg, 0,(struct sockaddr *)&client_a, client_size);
			}
			else if (strncmp("exit", new_buff, exit)==0)
			{
				strcpy(new_buff, "ok\n");
				sendto(sockfd_udp, new_buff, msg, 0,(struct sockaddr *)&client_a, client_size);
				break;	
			}
			else 
			{
				sendto(sockfd_udp, new_buff,msg, 0,(struct sockaddr *)&client_a, client_size);
			}
			// erasing the buffer
			bzero(new_buff, sizeof(new_buff));
			}
		// closing the connection
		close(sockfd_udp);
	}	

}
// client method
void chat_client(char* host, long port, int use_udp)
{
	int sockfd_cli;
	struct sockaddr_in server_a;
	char buff[255];
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
  
  // default tcp
	if (use_udp == 0)
	{
		sockfd_cli = socket(AF_INET, SOCK_STREAM, 0);
		connect(sockfd_cli, (struct sockaddr *)&server_a, sizeof(server_a));
		int goodbye = strlen("goodbye");
		int exit = strlen("exit");
		for(;;)
		{
			char buff_tmp[255];
			bzero(buff, sizeof(buff));
			if(fgets(buff, sizeof(buff), stdin)==NULL)
			{
				perror("Invalid");
			}
			if(write(sockfd_cli, buff, strlen(buff))!=0)
			{
				if(strncmp("goodbye",buff,goodbye) == 0 || strncmp("exit", buff, exit) == 0)
				{
					strcpy(buff_tmp, buff);
				}
			}	
			bzero(buff, sizeof(buff));
			if(read(sockfd_cli, buff, 255)== 0)
			{
				perror("Invalid");
			}
			else{
				// printing message from server
				printf("%s", buff);
			}
			if(strncmp("goodbye",buff_tmp,goodbye) == 0 || strncmp("exit", buff_tmp, exit) == 0)
			{
				break;
			}  
		}
		//close client
		close(sockfd_cli);
	}
// UDP connection
 	else
	{
		int sockfd_cli_udp;
		int goodbye = strlen("goodbye");
		int exit = strlen("exit");
		sockfd_cli_udp = socket(AF_INET, SOCK_DGRAM, 0);
		for(;;)
		{
			char buff_udp[255],buff_tmp[255];
			bzero(buff_udp, sizeof(buff_udp));
			if(fgets(buff_udp, sizeof(buff_udp), stdin)!=NULL)
			{
			socklen_t x  = sizeof(server_a);
			sendto(sockfd_cli_udp, buff_udp, strlen(buff_udp)+1, 0, (struct sockaddr *)&server_a, x);	
			}
			if (strncmp("goodbye",buff_udp, goodbye) == 0 || strncmp("exit", buff_udp, exit) == 0)
			{
				strcpy(buff_tmp, buff_udp);
			}
			bzero(buff_udp, sizeof(buff_udp));
			recvfrom(sockfd_cli_udp, buff_udp, sizeof(buff_udp), 0, NULL, NULL);
			// if buff is not empty
			if(buff_udp != 0)
			{
				printf("%s", buff_udp);
			}
			// to close the connection
			if (strncmp("goodbye",buff_tmp, goodbye) == 0 || strncmp("exit", buff_tmp, exit) == 0)
			{
				break;
			}
		}
		close(sockfd_cli_udp);
    }
}

