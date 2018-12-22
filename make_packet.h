#ifndef _MAKE_PACKET_H
#define _MAKE_PACKET_H

#include <stdint.h>
#include <string>

std::string make_connect_packet();
bool is_connect_packet(const char* data, size_t len);

std::string make_send_back_conv_packet(uint32_t conv);
bool is_send_back_conv_packet(const char* data, size_t len);
uint32_t get_conv_from_send_back_conv_packet(const char* data, size_t len);

std::string make_disconnect_packet(uint32_t conv);
bool is_disconnect_packet(const char* data, size_t len);
uint32_t get_conv_from_disconnect_packet(const char* data, size_t len);

#endif
