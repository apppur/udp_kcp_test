#include <unistd.h>
#include "define.h"
#include "agent.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1234
#define CLINET_PORT 8888

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("useage udp_client server_ip server_port client_port\n");
        return 0;
    }
    const char* server_ip = argv[1];
    unsigned short server_port = atoi(argv[2]);
    unsigned short client_port = atoi(argv[3]);
    //Agent agent(SERVER_IP, SERVER_PORT, CLINET_PORT);
    Agent agent(server_ip, server_port, client_port);
    agent.Connect();


    unsigned long long clock = GetCurrentTime();
    while (true)
    {
        if (clock + 100 < GetCurrentTime())
        {
            agent.SendMsg("Hello World");
            clock = GetCurrentTime();
        }
        agent.Update();
    }
}
