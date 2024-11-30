#include "../libraries/basic_client_udp.h"
#include <stdint.h>

struct ntp_packet
{
    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li. Two bits. Leap indicator.
                             // vn. Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.
    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.
    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.
    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.
    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.
    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.
    uint32_t txTm_s;         // 32 bits and the most important field the client cares
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
};

int main(int argc, char const *argv[])
{
    struct send_message_flags flags = {.wait_for_answer = true, .timeout = 20, .retries = 1};
    struct ntp_packet ntp_packet;
    send_message("127.0.0.1", 9800, (void *)&ntp_packet, flags);
    return 0;
}
