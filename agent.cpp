#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "agent.h"
#include "make_packet.h"
#include "ikcp.h"

Agent::Agent(std::string host, unsigned short port, unsigned short bind_port) :
    server_host_(host),
    server_port_(port),
    bind_port_(bind_port),
    size(0),
    sock_state_(ESOCKINIT)
{
    bzero(buff_, sizeof(buff_));
    FD_ZERO(&fd_set_);
}

Agent::~Agent()
{
}

void Agent::Clean()
{
    if (pkcp_ != nullptr)
    {
        ikcp_release(pkcp_);
        pkcp_ = nullptr;
    }
}

int Agent::Connect()
{
    if ((connectfd_ = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("agent socket error\n");
        exit(1);
    }

    // bind
    bzero(&client_addr_, sizeof(client_addr_));
    client_addr_.sin_family = AF_INET;
    client_addr_.sin_port = htons(bind_port_);
    client_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(connectfd_, (struct sockaddr *)&client_addr_, sizeof(client_addr_)) < 0)
    {
        printf("agent bind bind error\n");
        exit(1);
    }

    // connect
    bzero(&server_addr_, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(server_port_);
    inet_pton(AF_INET, server_host_.c_str(), &server_addr_.sin_addr);
    if (connect(connectfd_, (const struct sockaddr*)(&server_addr_), sizeof(server_addr_)) < 0)
    {
        printf("agent connect error\n");
        exit(1);
    }

    return 0;
}

void Agent::DoKcpConnect(unsigned long long cur_clock)
{
    SendConnectPacket(cur_clock);
    RecvConnectPacket();
}

void Agent::SendConnectPacket(unsigned long long cur_clock)
{
    std::string connect_msg = make_connect_packet();
    printf("send connect packet\n");
    //sendto(connectfd_, connect_msg.c_str(), connect_msg.size(), 0, (struct sockaddr *)&server_addr_, sizeof(server_addr_));
    ssize_t ret = send(connectfd_, connect_msg.c_str(), connect_msg.size(), 0);
    if (ret < 0)
    {
        printf("send connect packet error\n");
    }
}

void Agent::RecvConnectPacket()
{
    const ssize_t ret_recv = recvfrom(connectfd_, buff_, sizeof(buff_), 0, NULL, NULL);
    if (ret_recv < 0)
    {
        int err = errno;
        if (err == EAGAIN)
        {
            return;
        }
        printf("recv connect packet error errno:%d\n", err);
    }
    printf("recv data:%s, size:%ld\n", buff_, ret_recv);
    if (ret_recv > 0 && is_send_back_conv_packet(buff_, ret_recv))
    {
        uint32_t conv = get_conv_from_send_back_conv_packet(buff_, ret_recv);
        printf("client connect server succeed conv:%u\n", conv);
        sock_state_ = ESOCKCONNECTED;
        InitKcp(conv);
    }
}

void Agent::InitKcp(kcp_conv_t conv)
{
    conv_ = conv;
    pkcp_ = ikcp_create(conv, (void *)this);
    pkcp_->output = &Agent::Output;

    // 启动快速模式
    // 第二个参数 nodelay 启用以后若干常规加速将启动
    // 第三个参数 interval 为内部处理时钟，默认设置为 10ms
    // 第四个参数 resend 为快速重传指标，设置为 2
    // 第五个参数 为是否禁用常规流控制，这里禁止
    // ikcp_nodelay(pkcp_, 1, 10, 2, 1);
    ikcp_nodelay(pkcp_, 1, 5, 1, 1); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒.
}

int Agent::Output(const char* buff, int len, ikcpcb* kcp, void* user)
{
    ((Agent*)user)->SendUdpPacket(buff, len);
    return 0;
}

void Agent::SendUdpPacket(const char* buff, int len)
{
    //printf("send udp packet:%s, %d\n", buff, len);
    const ssize_t send_ret = send(connectfd_, buff, len, 0);
    if (send_ret < 0)
    {
        printf("send udp packet error errno%d\n", errno);
    }
    else if (send_ret != len)
    {
        printf("send udp packet:%ld in %d not all packet", send_ret, len);
    }
}

void Agent::SendMsgInQueue()
{
    int count = 0;
    while (send_queue_.size() > 0)
    {
        std::string msg = send_queue_.front();
        int send_ret = ikcp_send(pkcp_, msg.c_str(), msg.size());
        if (send_ret < 0)
        {
            printf("ikcp send msg error:%d", send_ret);
        }
        printf("time:%llu, send msg in queue:%s\n", GetCurrentTime(), msg.c_str());
        send_queue_.pop();
        if (count > 5)
        {
            break;
        }
    }
}

void Agent::RecvUdpPacketInLoop()
{
    char recv_buff[1024] = "";
    const ssize_t recv_ret = recv(connectfd_, recv_buff, sizeof(recv_buff), 0);
    if (recv_ret < 0)
    {
        int err = errno;
        if (err == EAGAIN)
        {
            return;
        }
        printf("recv udp packet in loop error errno:%d\n", err);
        return;
    }
    if (recv_ret == 0)
    {
        printf("recv udp packet from server size 0\n");
        return;
    }
    printf("=====================================\n");
    printf("time:%llu recv udp packet from server size:%ld\n", GetCurrentTime(), recv_ret);
    HandleUdpPacket(std::string(recv_buff, recv_ret));
}

void Agent::HandleUdpPacket(const std::string& udp_packet)
{
    if (is_disconnect_packet(udp_packet.c_str(), udp_packet.size()))
    {
        return;
    }

    ikcp_input(pkcp_, udp_packet.c_str(), udp_packet.size());

    while (true)
    {
        const std::string& msg = RecvUdpPacketFromKcp();
        if (msg.size() > 0)
        {
            printf("recv udp msg from kcp:%s\n", msg.c_str());
            printf("=====================================\n");
            continue;
        }
        break;
    }
}

std::string Agent::RecvUdpPacketFromKcp()
{
    char kcp_buff[1024] = "";
    int kcp_recv_bytes = ikcp_recv(pkcp_, kcp_buff, sizeof(kcp_buff));
    if (kcp_recv_bytes < 0)
    {
        return "";
    }
    //printf("client recv udp packet:%s from kcp\n", kcp_buff);
    const std::string result(kcp_buff, kcp_recv_bytes);
    return result;
}

void Agent::SendMsg(const std::string& msg)
{
    if (msg.size() > 0)
    {
        send_queue_.push(msg);
    }
}

void Agent::Update()
{
    unsigned long long cur_clock = GetCurrentTime();

    if (sock_state_ != ESOCKCONNECTED)
    {
        DoKcpConnect(cur_clock);
    }

    if (sock_state_ == ESOCKCONNECTED)
    {
        // send msg in queue
        SendMsgInQueue();

        // recv udp msg
        FD_SET(connectfd_, &fd_set_);
        int nready = 0;
        struct timeval tm;
        tm.tv_sec = 0;
        tm.tv_usec = 0;
        if ((nready = select(connectfd_ + 1, &fd_set_, NULL, NULL, &tm)) < 0)
        {
            if (errno == EINTR)
            {
            }
            else
            {
                printf("select error errno:%d\n", errno);
            }
        }
        if (FD_ISSET(connectfd_, &fd_set_))
        {
            RecvUdpPacketInLoop(); 
        }

        // ikcp_update
        ikcp_update(pkcp_, cur_clock);
    }

    /*
    if (sock_state_ == ESOCKCONNECTED)
    {
        // send msg in queue
        SendMsgInQueue();
        // recv udp msg kcp
        //RecvUdpPacketInLoop(); 
        // ikcp_update
        ikcp_update(pkcp_, cur_clock);
    }
    */
}
