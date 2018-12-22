#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "define.h"
#include "server.h"
#include "make_packet.h"
#include "ikcp.h"

Server::Server(std::string ip, unsigned short port) :
    ip_(ip),
    port_(port),
    kcp_conv_(0),
    server_time_(0),
    BUFSIZE(1024),
    MAXEVENT(1024)
{
    listenfd_ = 0;
    epollfd_ = 0;
    channels.clear();
}

Server::~Server()
{
    ip_.clear();
    port_ = 0;
    listenfd_ = 0;
    epollfd_ = 0;
    channels.clear();
}

void Server::Init()
{
    listenfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (listenfd_ < 0)
    {
        std::cout << "sokcet error" << std::endl;
        exit(1);
    }

    struct sockaddr_in serveraddr; /* server's addr */
    int optval = 1;
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port_);

    if (bind(listenfd_, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        std::cout << "bind error" << std::endl;
        exit(1);
    }
    struct epoll_event ev;
    epollfd_ = epoll_create1(0);
    if (epollfd_ == -1) {
        std::cout << "epoll_create1 error" << std::endl;
        exit(1);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd_;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, listenfd_, &ev) == -1) {
        std::cout << "epoll_ctl error" << std::endl;
        exit(1);
    }
}

void Server::Update()
{
    int nfds = 0;
    struct epoll_event events[MAXEVENT];
    char buff[BUFSIZE];
    socklen_t clientlen; /* byte size of client's address */
    struct sockaddr_in clientaddr; /* client addr */
    server_time_ = GetCurrentTime();
    size_t size = 0;
    for ( ; ; ) {
        nfds = epoll_wait(epollfd_, events, 1024, 5);
        if (nfds == -1) {
            printf("epoll_wait error\n");
            exit(1);
        }
        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == listenfd_) {
                clientlen = sizeof(clientaddr);
                bzero(buff, sizeof(buff));
                size = recvfrom(listenfd_, buff, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);
                if (size < 0) {
                    printf("recvfrom error\n");
                }
                printf("========================================\n");
                //printf("server received %ld/%ld bytes: %s\n", strlen(buff), size, buff);
                HandleRecvPacket(clientaddr, clientlen, buff, size);
            }

        }
        server_time_ = GetCurrentTime();
        CheckTimeout(server_time_);
    }
}

void Server::SendUdpPacket(const std::string& msg, const sockaddr_in& clientaddr, const socklen_t& clientlen)
{
    struct hostent *hostp;
    char *hostaddrp;
    hostp = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp != NULL) {
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp != NULL) {
            printf("time:%llu server send datagram to %s (%s %u) data size:%ld\n", GetCurrentTime(), hostp->h_name, hostaddrp, clientaddr.sin_port, msg.size());
        }
    } else {
        printf("gethostbyaddr error\n");
    }

    int n = 0;
    //printf("send msg:%ld, %s", msg.size(), msg.c_str());
    n = sendto(listenfd_, msg.c_str(), msg.size(), 0, (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0)
    {
        printf("sendto error\n");
    }
}

void Server::AddNewChannel(const struct sockaddr_in& addr, socklen_t len)
{
    kcp_conv_t conv = GetUniqueId();
    printf("new client connect conv:%d\n", conv);
    //Channel channel("apple", this);
    //channel.Create(conv, addr, len);
    std::map<kcp_conv_t, Channel*>::const_iterator iter = channels.find(conv);
    if (iter == channels.end())
    {
        Channel* ptr = new Channel("apple", this);
        ptr->Create(conv, addr, len);
        //channels.insert(std::pair<kcp_conv_t, Channel>(conv, std::move(channel)));
        channels.insert(std::pair<kcp_conv_t, Channel*>(conv, ptr));
    }

    std::string msg = make_send_back_conv_packet(conv);
    sendto(listenfd_, msg.c_str(), msg.size(), 0, (struct sockaddr *)&addr, len);
}

void Server::HandleRecvPacket(const struct sockaddr_in& addr, socklen_t len, char* data, int size)
{
    if (data != nullptr && size > 0)
    {
        if (is_connect_packet(data, size))
        {
            AddNewChannel(addr, len);
        }
        else
        {
            HandleKcpPacket(data, size, addr, len);
        }
    }
}

void Server::HandleKcpPacket(const char* data, int size, const struct sockaddr_in& addr, socklen_t len)
{
    kcp_conv_t conv = 0;
    conv = ikcp_getconv(data);
    if (conv == 0)
    {
        printf("ikcp conv get error\n");
        return;
    }

    std::map<kcp_conv_t, Channel*>::iterator iter = channels.find(conv);
    if (iter != channels.end() && iter->second != NULL)
    {
        iter->second->Input(data, size, addr, len);
    }
}

void Server::CheckTimeout(unsigned long long clock)
{
    for (std::map<kcp_conv_t, Channel*>::iterator iter = channels.begin(); iter != channels.end();)
    {
        if (iter->second == NULL)
        {
            channels.erase(iter++);
            continue;
        }

        if (iter->second->Timeout())
        {
            printf("remove conv:%d\n", iter->second->GetConv());
            Channel* ptr = iter->second;
            channels.erase(iter++);
            delete ptr;
            printf("========================================\n");
        }
        else
        {
            iter->second->UpdatKcp(clock);
            iter++;
        }
    }
}
