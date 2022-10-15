/********************************************/
/* gobackn.c - Vishwas Desai (visdesai) */
/* Modified: 11/23/2021 */
/* Submission for Lab 6 - Reliable UDP part 2 w/o Congestion Control */
/*********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <errno.h>

#include "stopandwait.h"
#include "gobackn.h"

#define MAX_SIZE 224
// #define TIMEOUT 5
// #define WINDOW 3


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


/*
 *  Here is the starting point for your netster part.4 definitions. Add the
 *  appropriate comment header as defined in the code formatting guidelines
 */

 /* Add function definitions */

void gbn_server(char* iface, long port, FILE* fp)
{
    struct sockaddr_in server_a, client_a;
    server_a.sin_family = AF_INET;
    server_a.sin_addr.s_addr = INADDR_ANY;
    server_a.sin_port = htons(port);

    int content;
    // intializing new squence number in server
    int sq_no = 0;
    // data frame
    Frame recv_frame;
    //ack frame
    Frame send_frame;
    socklen_t client_size = sizeof(client_a);

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

    // infinite loop
    for (;;)
    {
        content = recvfrom(sockfd_udp, &recv_frame, sizeof(recv_frame), 0, (struct sockaddr*)&client_a, &client_size);
        if (0 < content)
        {
            if (sq_no == recv_frame.sq_number && recv_frame.frame_class == 1)
            {
                fwrite(recv_frame.data.data, sizeof(char), recv_frame.len_data, fp);
                printf("==================================================\n");
                printf("\n");
                printf("#----------------------------------#\n");
                printf("    Ack sent\n");
                printf("#----------------------------------#\n");
                printf("\n");
                printf("Frame received by client\n");
                printf("Frame Class: %d\nData: %s\nSequence Number: %d\n", recv_frame.frame_class, recv_frame.data.data, recv_frame.sq_number);
                printf("\n");
                printf("==================================================\n");
                printf("\n");
                send_frame.frame_class = 0;
                send_frame.sq_number = sq_no;
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
                // printf("Frame not received\n");
                printf("Resending Acknowledgement\n");
                send_frame.frame_class = 0;
                send_frame.sq_number = sq_no - 1;
                sendto(sockfd_udp, &send_frame, sizeof(send_frame), 0, (struct sockaddr*)&client_a, client_size);
                // continue;
            }
        }
        // else
        // {
        //     // close(sockfd_udp);
        //     perror("Error");
        //     break;
        // }
    }
}



void gbn_client(char* host, long port, FILE* fp)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 300000;
    //int content;
    int sq_no = 0;


    // Datapacket data;
    // data frame
    Frame send_frame;
    // ack frame
    Frame recv_frame;

    int sockfd_cli;
    struct sockaddr_in server_a;

    /* resolving the loopback IP */
    if (strncmp("localhost", host, strlen("localhost")) == 0)
    {
        host = "127.0.0.1";
    }

    //char* ip = "127.0.0.1";
    server_a.sin_family = AF_INET;
    server_a.sin_addr.s_addr = inet_addr(host);
    server_a.sin_port = htons(port);

    char send_data[MAX_SIZE];
    bzero(send_data, sizeof(send_data));
    socklen_t server_size = sizeof(server_a);

    sockfd_cli = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd_cli, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));


    size_t file_content;

    char message[MAX_SIZE];
    int count = 0;
    bzero(message, sizeof(message));

    do
    {
        count = count + 1;
    } while ((file_content = fread(message, sizeof(char), MAX_SIZE - 1, fp)) != 0);

    // int fp_size = ftell(fp);
    // fseek(fp, fp_size-1, SEEK_END);

    fseek(fp, 0, SEEK_SET);
    // new frame with frame count
    Frame msg_frame[count];
    int sqno = 0, cur = 0;
    for (;;)
    {
        if ((file_content = fread(send_data, sizeof(char), MAX_SIZE - 1, fp)) > 0)
        {
            send_frame.frame_class = 1;
            send_frame.sq_number = sqno;
            send_frame.len_data = file_content;
            // copying send_data to frame -> data
            memcpy(send_frame.data.data, send_data, file_content);
            // copying data from send_frame to a temp frame -> msg_frame
            memcpy(&msg_frame[cur], &send_frame, sizeof(Frame));
            //increment curr index
            cur++;
            //increment sequence number
            sqno++;
            continue;
        }
        else
        {
            // throw an error
            // perror("e");
            break;
        }

    }
    // intialize base and window
    short base = 0;
    short window_size = 3;
    // int chunks_of_data == sqno;
    int frame_chunk = sqno;
    for (;;)
    {
        if (base < frame_chunk)
        {
            if (sq_no < base + window_size)
            {
                printf("Sending DATA to server\n");
                sendto(sockfd_cli, &msg_frame[sq_no], sizeof(Frame), 0, (struct sockaddr*)&server_a, sizeof(server_a));
                sq_no++;
            }
        }
        else
        {
            perror("Error");
            continue;
        }

        int content_received = recvfrom(sockfd_cli, &recv_frame, sizeof(recv_frame), 0, (struct sockaddr*)&server_a, &server_size);

        if (0 < content_received)
        {
            // ---- printf statemnts for debugging ---
            printf("Received content len on client side: %d\n", content_received);
            printf("=================================================\n");
            printf("Received ACK\n");
            printf("Received frame-id and seq-nor and len of data %d\n%d\n%d\n", recv_frame.frame_class, recv_frame.sq_number, recv_frame.len_data);
            printf("=================================================\n");
            base = recv_frame.sq_number + 1;
            if (base == frame_chunk)
            {
                exit(0);
            }
        }
        else
        {
            sq_no = base;
            // printf("Window full: waiting for ACKs\n");
            printf("Timeout\nRe-sending packets\n");

        }
    }

}

