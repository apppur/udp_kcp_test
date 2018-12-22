#include <cstring>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include "make_packet.h"

#define KCP_CONNECT_PACKET "KCP_CONNECT_PACKET GET_CONV"
#define KCP_SEND_BACK_CONV_PACKET "KCP_SEND_BACK_CONV_PACKET GET_CONV:"
#define KCP_DISCONNECT_PACKET "KCP_DISCONNECT_PACKET"

std::string make_connect_packet()
{
    return std::string(KCP_CONNECT_PACKET, sizeof(KCP_CONNECT_PACKET));
}

bool is_connect_packet(const char* data, size_t len)
{
    return (len == sizeof(KCP_CONNECT_PACKET) && 
            memcmp(data, KCP_CONNECT_PACKET, sizeof(KCP_CONNECT_PACKET) - 1) == 0);
}

std::string make_send_back_conv_packet(uint32_t conv)
{
    char buff[256] = "";
    size_t n = snprintf(buff, sizeof(buff), "%s %u", KCP_SEND_BACK_CONV_PACKET, conv);
    return std::string(buff, n);
}

bool is_send_back_conv_packet(const char* data, size_t len)
{
    return (len > sizeof(KCP_SEND_BACK_CONV_PACKET) &&
            memcmp(data, KCP_SEND_BACK_CONV_PACKET, sizeof(KCP_SEND_BACK_CONV_PACKET) - 1) == 0);
}

uint32_t get_conv_from_send_back_conv_packet(const char* data, size_t len)
{
    uint32_t conv = atol(data + sizeof(KCP_SEND_BACK_CONV_PACKET));
    return conv;
}

std::string make_disconnect_packet(uint32_t conv)
{
    char buff[256] = "";
    size_t n = snprintf(buff, sizeof(buff), "%s %u", KCP_DISCONNECT_PACKET, conv);
    return std::string(buff, n);
}

bool is_disconnect_packet(const char* data, size_t len)
{
    return (len > sizeof(KCP_DISCONNECT_PACKET) &&
            memcmp(data, KCP_DISCONNECT_PACKET, sizeof(KCP_DISCONNECT_PACKET) - 1) == 0);
}

uint32_t get_conv_from_disconnect_packet(const char* data, size_t len)
{
    uint32_t conv = atol(data + sizeof(KCP_DISCONNECT_PACKET));
    return conv;
}
