#ifndef _UDP_SERVER_
#define _UDP_SERVER_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

#define BUFSIZE 1024
#define MAX_EVENT 64

int main(int argc, char** argv)
{
    printf("udp server will start ...\n");
    int port; /*listen port*/
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    printf("server will listen port :%d\n", port);

    Server server("127.0.0.1", port);
    server.Init();
    server.Update();

    //int sockfd; /*socket fd*/
    //int port; /*listen port*/
    //socklen_t clientlen; /* byte size of client's address */
    //struct sockaddr_in serveraddr; /* server's addr */
    //struct sockaddr_in clientaddr; /* client addr */
    //struct hostent *hostp; /* client host info */
    //char buf[BUFSIZE]; /* message buf */
    //char *hostaddrp; /* dotted decimal host addr string */
    //int optval; /* flag value for setsockopt */
    //int n; /* message byte size */

    /*
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("create socket fd error\n");
        exit(1);
    }

    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);

    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        printf("bind socket errro\n");
        exit(1);
    }

    struct epoll_event ev, events[MAX_EVENT];
    int epollfd, nfds;
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        printf("epoll_create1 error \n");
        exit(1);
    }

    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        printf("epoll_ctl error \n");
        exit(1);
    }
    for ( ; ; ) {
        nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
        if (nfds == -1) {
            printf("epoll_wait error\n");
            exit(1);
        }
        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == sockfd) {
                clientlen = sizeof(clientaddr);
                bzero(buf, BUFSIZE);
                n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);
                if (n < 0) {
                    printf("recvfrom error\n");
                }

                hostp = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
                if (hostp != NULL) {
                    hostaddrp = inet_ntoa(clientaddr.sin_addr);
                    if (hostaddrp != NULL) {
                        printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
                    }
                } else {
                    printf("gethostbyaddr error\n");
                }

                printf("server received %ld/%d bytes: %s\n", strlen(buf), n, buf);

                n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
                if (n < 0) {
                    printf("sendto error\n");
                }
            }

        }
    }
    */

    /*

    clientlen = sizeof(clientaddr);
    while (1) {
        bzero(buf, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0) {
            printf("recvfrom error\n");
        }
        
        hostp = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp != NULL) {
            hostaddrp = inet_ntoa(clientaddr.sin_addr);
            if (hostaddrp != NULL) {
                printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
            }
        } else {
            printf("gethostbyaddr error\n");
        }

        printf("server received %ld/%d bytes: %s\n", strlen(buf), n, buf);

        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) {
            printf("sendto error\n");
        }
    }
    */
}

#endif
