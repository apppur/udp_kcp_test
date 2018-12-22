#ifndef _CHANNEL_
#define _CHANNEL_
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include "define.h"

class Server;

class Channel
{
    public:
        Channel(std::string name, Server* server);
        Channel(Channel&& channel);
        Channel& operator=(Channel&& channel);
        ~Channel();

        void Create(kcp_conv_t conv, const struct sockaddr_in& addr, socklen_t len);
        void Input(const char* udp_data, size_t bytes, const struct sockaddr_in& addr, const socklen_t& len);
        void SendKcp(const std::string& msg);
        void UpdatKcp(unsigned long long clock);

        void SetPkcp(ikcpcb * kcp) { pkcp_ = kcp; }
        void SetName(std::string name) { name_ = name; }
        const std::string& GetName() { return name_; }
        void SetConv(kcp_conv_t conv) { conv_ = conv; }
        kcp_conv_t GetConv() { return conv_; }
        void SetChannelAddr(struct sockaddr_in& channel_add) { channel_addr_ = channel_add; }
        const struct sockaddr_in& GetChannelAddr() { return channel_addr_; }
        void SetSockLen(socklen_t len) { sock_len_ = len; }
        socklen_t GetSockLen() { return sock_len_; }
        void SetLastPacketRecvTime(uint32_t tm) { last_packet_recv_time_ = tm; }
        uint32_t GetLastPacketRecvTime() { return last_packet_recv_time_; }
        static int Dump(void* user);

        bool Timeout();

    private:
        void InitKcp(const kcp_conv_t& conv);
        void Clean();

        static int Output(const char* buff, int len, ikcpcb* kcp, void* user);
        void SendPackage(const char* buff, int len);
    private:
        std::string name_;
        kcp_conv_t conv_;
        ikcpcb* pkcp_;
        unsigned long long last_packet_recv_time_;
        struct sockaddr_in channel_addr_;
        socklen_t sock_len_;

        Server* server_;
};

#endif
