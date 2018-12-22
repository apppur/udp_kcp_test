#include "channel.h"

struct Call
{
    int (*dump)(void* user);
};

int main(int argc, char** argv)
{
    Channel channel("APPPUR");
    Call call;
    call.dump = &Channel::Dump;
    call.dump(&channel);

    return 0;
}
