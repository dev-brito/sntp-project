#ifndef __BASIC_CLIENT_UDP_H__
#define __BASIC_CLIENT_UDP_H__

#include <stdbool.h>

struct send_message_flags
{
    int retries;
    bool wait_for_answer;
    int timeout;
    bool received_message;
};

void * send_message(const char *peer_ip, int peer_port, void * payload, struct send_message_flags flags);

#endif