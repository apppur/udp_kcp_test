#ifndef _AGENT_H
#define _AGENT_H
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string>
#include <queue>
#include "define.h"

class Agent
{
    public:
        Agent(std::string host, unsigned short port, unsigned short bind_port);
        ~Agent();

        int Connect();
        void Update();
        void SendConnectPacket(unsigned long long cur_clock);
        void DoKcpConnect(unsigned long long cur_clock);
        void RecvConnectPacket();
        void SendMsgInQueue();
        void RecvUdpPacketInLoop();
        void HandleUdpPacket(const std::string& udp_packet);
        std::string RecvUdpPacketFromKcp();
        void SendMsg(const std::string& msg);
    private:
        void InitKcp(kcp_conv_t conv);
        static int Output(const char* buff, int len, ikcpcb* kcp, void* user);
        void SendUdpPacket(const char* buff, int len);
        void Clean();
    private:
        std::string server_host_;
        unsigned short server_port_;
        int connectfd_;
        fd_set fd_set_;
        unsigned short bind_port_;
        kcp_conv_t conv_;
        ikcpcb* pkcp_;
        struct sockaddr_in server_addr_;
        struct sockaddr_in client_addr_;

        char buff_[1024];
        int size;
        int sock_state_;

        std::queue<std::string> send_queue_;
};

#endif
