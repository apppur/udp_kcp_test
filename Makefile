#CFLAGS = -g -O2 -Wall
CFLAGS = -g -O2

CORE_INC ?= ./

INC := -I$(CORE_INC)
SRC := $(wildcard *.cpp)

CC = g++ --std=c++11

all : udp_server udp_client

udp_server : udp_server.cpp server.cpp channel.cpp define.cpp ikcp.c make_packet.cpp
	$(CC) $(CFLAGS) -o $@ $^ $(INC)

udp_client : udp_client.cpp agent.cpp define.cpp make_packet.cpp ikcp.c
	$(CC) $(CFLAGS) -o $@ $^ $(INC)

clean :
	rm udp_client udp_server
