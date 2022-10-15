/********************************************/
/* file.c - Vishwas Desai (visdesai) */
/* Modified: 11/13/2021 */
/* Submission for Lab 5 - Reliable UDP part 1 */
/*********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <errno.h>

#include "file.h"
#include "stopandwait.h"

#define MAX_SIZE 224

/*
 *  Here is the starting point for your netster part.3 definitions. Add the
 *  appropriate comment header as defined in the code formatting guidelines
 */

 /* Add function definitions */

typedef struct datapacket
{
    char data[MAX_SIZE];
} Datapacket;

typedef struct frame
{
    // ACK:0, SQNO:1
    // size of frame: 12
    int frame_class;
    int sq_number;
    int len_data;
    Datapacket data;
} Frame;

void stopandwait_server(char* iface, long port, FILE* fp)
{

    struct sockaddr_in server_a, client_a;

    //char* ip = "127.0.0.1";
    server_a.sin_family = AF_INET;
    server_a.sin_addr.s_addr = INADDR_ANY;
    server_a.sin_port = htons(port);

    int content;
    int sq_no = 0;
    // data frame
    Frame recv_frame;
    //ack frame
    Frame send_frame;
    socklen_t client_size = sizeof(client_a);
    //int sockfd_cli;
    int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    int set_flag = 1;
    int setsock = setsockopt(sockfd_udp, SOL_SOCKET, SO_REUSEADDR, &set_flag, sizeof(set_flag));
    if (setsock < 0)
    {
        perror("Error: Setsockopt not set\n");
    }
    int x = bind(sockfd_udp, (struct sockaddr*)&server_a, sizeof(server_a));
    if (x < 0)
    {
        perror("ERROR:Bind Failed for UDP\n");
    }
    // infinte loop
    for (;;)
    {
        content = recvfrom(sockfd_udp, &recv_frame, sizeof(Frame), 0, (struct sockaddr*)&client_a, &client_size);
        if (0 < content)
        {
            if (recv_frame.sq_number == sq_no && recv_frame.frame_class == 1)
            {
                fwrite(recv_frame.data.data, sizeof(char), recv_frame.len_data, fp);
                printf("#----------------------------------#\n");
                // printf("\n");
                printf("    Ack sent\n");
                // printf("\n");
                printf("#----------------------------------#\n");
                printf("\n");
                printf("Frame received by client\n");
                printf("Frame Class: %d\nData: %s\nSequence Number: %d\n", recv_frame.frame_class, recv_frame.data.data, recv_frame.sq_number);
                printf("\n");
                printf("==================================================\n");
                printf("\n");
                send_frame.frame_class = 0;
                send_frame.sq_number = sq_no + 1;
                // if (1)
                // {
                sendto(sockfd_udp, &send_frame, sizeof(send_frame), 0, (struct sockaddr*)&client_a, client_size);
                // }
                if (recv_frame.len_data < MAX_SIZE - 1)
                {
                    perror("Message");
                    exit(0);
                }
                sq_no++;
            }
            else
            {
                printf("Frame not received\n");
                printf("Resend Acknowledgement\n");
                sendto(sockfd_udp, &send_frame, sizeof(send_frame), 0, (struct sockaddr*)&client_a, client_size);
                continue;
            }
        }
        else
        {
            // close(sockfd_udp);
            perror("Error");
            // exit(0);
            break;
        }
    }
}

void stopandwait_client(char* host, long port, FILE* fp)
{

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 350000;
    int content;
    int sq_no = 0;
    int content_received;

    Datapacket data;
    // data frame
    Frame send_frame;
    // ack frame
    Frame recv_frame;

    int sockfd_cli;
    struct sockaddr_in server_a;

    /* resolving the loopback IP */
    // int len = strlen("localhost");
    if (strncmp("localhost", host, strlen("localhost")) == 0)
    {
        host = "127.0.0.1";
    }

    //char* ip = "127.0.0.1";
    server_a.sin_family = AF_INET;
    server_a.sin_addr.s_addr = inet_addr(host);
    server_a.sin_port = htons(port);

    socklen_t server_size = sizeof(server_a);
    sockfd_cli = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd_cli, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
    char send_data[MAX_SIZE];

    bzero(send_data, sizeof(send_data));
    memcpy(&(send_frame.data), &data, sizeof(Datapacket));

    while ((content = fread(send_data, sizeof(char), MAX_SIZE - 1, fp)) > 0)
    {
        send_frame.frame_class = 1;
        send_frame.sq_number = sq_no;
        send_frame.len_data = content;
        memcpy(send_frame.data.data, send_data, content);

        for (;;)
        {
            //printf("Size of frame, client: %lu\n", sizeof(Frame));
            printf("\n");
            printf("Sending DATA to server\n");
            // if (1)
            // simualting lossy conncetion locally
            //if(rand() % 2) {
            sendto(sockfd_cli, &send_frame, sizeof(Frame), 0, (struct sockaddr*)&server_a, sizeof(server_a));
            // }
            content_received = recvfrom(sockfd_cli, &recv_frame, sizeof(recv_frame), 0, (struct sockaddr*)&server_a, &server_size);
            // ---- printf statemnts for debugging ---
            // printf("Received content len on client side: %d\n", content_received);
            // printf("Received frame-id and seq-nor and len of data %d %d %d \n", recv_frame.frame_class, recv_frame.sq_number, recv_frame.len_data);
            if (0 < content_received && recv_frame.sq_number == sq_no + 1 && recv_frame.frame_class == 0)
            {
                // increment sequence number
                sq_no++;
                printf("\n");
                printf("Ack received\n");
                break;
            }
            else
            {
                printf("Timeout\nRe-sending packets\n");
                continue;
            }
        }
    }
}