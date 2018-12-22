#ifndef _SERVER_H
#define _SERVER_H

#include <vector>
#include <map>
#include <string>
#include "define.h"
#include "channel.h"

class Server
{
    public:
        Server(std::string ip, unsigned short port);
        ~Server();

        void Init();
        void SendMsg(kcp_conv_t conv, std::string& msg);
        void SendUdpPacket(const std::string& msg, const sockaddr_in& clientaddr, const socklen_t& socklen);
        void Update();
        void AddNewChannel(const struct sockaddr_in& addr, socklen_t len);
        void CheckTimeout(unsigned long long clock);
        void HandleRecvPacket(const struct sockaddr_in& addr, socklen_t len, char* data, int size);
        void HandleKcpPacket(const char* data, int size, const struct sockaddr_in& addr, socklen_t len);

    private:
        kcp_conv_t GetUniqueId() { return ++kcp_conv_; };
    private:
        std::string ip_;
        unsigned short port_;
        int listenfd_;
        int epollfd_;
        kcp_conv_t kcp_conv_;
        std::map<kcp_conv_t, Channel*> channels;
        unsigned long long server_time_;

        const int BUFSIZE;
        const int MAXEVENT;
};
#endif
