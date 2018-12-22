#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "ikcp.h"
#include "define.h"
#include "channel.h"
#include "server.h"

Channel::Channel(std::string name, Server* server) :
    name_(name),
    server_(server),
    conv_(),
    pkcp_(NULL),
    last_packet_recv_time_(0)
{
}

Channel::Channel(Channel&& channel)
{
    name_ = channel.name_;
    conv_ = channel.conv_;
    pkcp_ = channel.pkcp_;
    last_packet_recv_time_ = channel.last_packet_recv_time_;
    channel_addr_ = channel.channel_addr_;
    sock_len_ = channel.sock_len_;
    server_ = channel.server_;
    
    channel.SetConv(0);
    channel.SetPkcp(NULL);
}

Channel& Channel::operator=(Channel&& channel)
{
    if (this != &channel)
    {
        Clean();
        name_ = channel.name_;
        conv_ = channel.conv_;
        pkcp_ = channel.pkcp_;
        last_packet_recv_time_ = channel.last_packet_recv_time_;
        channel_addr_ = channel.channel_addr_;
        sock_len_ = channel.sock_len_;
        server_ = channel.server_;

        channel.SetConv(0);
        channel.SetPkcp(NULL);
    }
}

Channel::~Channel()
{
    Clean();
}

void Channel::Clean()
{
    if (NULL != pkcp_)
    {
        std::cout << "clean connection covn" << std::endl;
        ikcp_release(pkcp_);
        pkcp_ = NULL;
        conv_ = 0;
    }
    else
    {
        std::cout << "move channel" << std::endl;
    }
}

void Channel::Create(kcp_conv_t conv, const struct sockaddr_in& addr, socklen_t len)
{
    channel_addr_ = addr;
    sock_len_ = len;
    InitKcp(conv);
    last_packet_recv_time_ = GetCurrentTime();
    struct hostent *hostp;
    char *hostaddrp;
    hostp = gethostbyaddr((const char*)&channel_addr_.sin_addr.s_addr, sizeof(channel_addr_.sin_addr.s_addr), AF_INET);
    if (hostp != NULL) {
        hostaddrp = inet_ntoa(channel_addr_.sin_addr);
        if (hostaddrp != NULL) {
            printf("server received datagram from %s (%s %u)\n", hostp->h_name, hostaddrp, addr.sin_port);
        }
    } else {
        printf("gethostbyaddr error\n");
    }

    //SendPackage("", 0);
}

void Channel::InitKcp(const kcp_conv_t& conv)
{
    conv_ = conv;
    pkcp_ = ikcp_create(conv, (void *)this);
    pkcp_->output = &Channel::Output;

    // 启动快速模式
    // 第二个参数 nodelay 启用以后若干常规加速将启动
    // 第三个参数 interval 为内部处理时钟，默认设置为 10ms
    // 第四个参数 resend 为快速重传指标，设置为 2
    // 第五个参数 为是否禁用常规流控制，这里禁止
    // ikcp_nodelay(pkcp_, 1, 10, 2, 1);
    ikcp_nodelay(pkcp_, 1, 5, 1, 1); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒.
}

int Channel::Output(const char* buff, int len, ikcpcb* kcp, void* user)
{
    ((Channel*)user)->SendPackage(buff, len);
    return 0;
}

void Channel::Input(const char* udp_data, size_t bytes, const struct sockaddr_in& addr, const socklen_t& len)
{
    last_packet_recv_time_ = GetCurrentTime();
    ikcp_input(pkcp_, udp_data, bytes);

    char kcp_buff[1024*4] = "";
    int kcp_recv_bytes = ikcp_recv(pkcp_, kcp_buff, sizeof(kcp_buff));
    if (kcp_recv_bytes <= 0)
    {
        printf("kcp recv bytes<=:%d\n", kcp_recv_bytes);
        return;
    }
    const std::string msg(kcp_buff, kcp_recv_bytes);
    printf("time:%llu recv kcp msg:%s, %d\n", GetCurrentTime(), kcp_buff, kcp_recv_bytes);
    SendKcp(msg);
}

void Channel::SendPackage(const char* buff, int len)
{
    if (server_ != NULL)
    {
        std::string echo(buff, len);
        server_->SendUdpPacket(echo, channel_addr_, sock_len_);
    }
    else
    {
        printf("server_ is nullptr\n");
    }
}

void Channel::SendKcp(const std::string& msg)
{
    int send_ret = ikcp_send(pkcp_, msg.c_str(), msg.size());
    if (send_ret < 0)
    {
        std::cout << "send_ret<0: " << send_ret << std::endl;
    }
}

void Channel::UpdatKcp(unsigned long long clock)
{
    ikcp_update(pkcp_, clock);
}

int Channel::Dump(void* user)
{
    std::cout << "CHANNEL DUMP START" << std::endl;
    Channel* c = (Channel *)user;
    if (c != NULL)
    {
        std::cout << "name:" << c->name_ << std::endl;
    }
    std::cout << "CHANNEL DUMP END" << std::endl;

    return 0;
}

bool Channel::Timeout()
{
    unsigned long long cur = GetCurrentTime();
    if (last_packet_recv_time_ + KCP_CONNECTION_TIMEOUT_TIME < cur)
    {
        return true;
    }
    else
    {
        return false;
    }
}
