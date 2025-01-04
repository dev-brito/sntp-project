#include "basic_client_udp.h"
#include "ntp_packet.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>




int close_socket(int socket)
{
    if (close(socket) < 0)
    {
        perror("Error closing socket.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void *send_message(const char *peer_ip, int peer_port, void *payload, struct send_message_flags flags) {
    struct sockaddr_in peer_addr = {.sin_family = AF_INET, .sin_port = htons(peer_port)};
    
    if (inet_aton(peer_ip, &peer_addr.sin_addr) <= 0) {
        perror("Invalid IP address.");
        return NULL;
    }

    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("Socket creation failure.");
        return NULL;
    }

    int try = 1;
    int total_tries = flags.retries + 1;
    do {
        flags.received_message = false;
        printf("Sending Message\n\tTry: %d\n\tTotal tries:%d.\n", try, total_tries);

        if (sendto(udp_socket, payload, sizeof(struct ntp_packet), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
            perror("Sending message failure.");
            close_socket(udp_socket);
            return NULL;
        }

        if (flags.wait_for_answer) {
            int ret;
            fd_set readfds;
            struct timeval timeout;
            timeout.tv_sec = flags.timeout;
            timeout.tv_usec = 0;

            FD_ZERO(&readfds);
            FD_SET(udp_socket, &readfds);

            printf("\tEsperando resposta do servidor...\n");
            ret = select(udp_socket + 1, &readfds, NULL, NULL, &timeout);

            if (ret < 0) {
                perror("Error during select.");
                close_socket(udp_socket);
                return NULL;
            } else if (ret == 0) {
                printf("\tOtempo expirou e não houve reposta do servidor.\n");
                try++;
            } else {
                flags.received_message = true;
            }
        }
    } while (try <= total_tries && !flags.received_message);

    if (flags.received_message) {
        socklen_t addr_len = sizeof(peer_addr);
        ssize_t received_bytes = recvfrom(udp_socket, payload, sizeof(struct ntp_packet), 0, (struct sockaddr *)&peer_addr, &addr_len);

    if (received_bytes < 0) {
        perror("Error receiving message.");
        close_socket(udp_socket);
        return NULL;
    } else if (received_bytes < (ssize_t)sizeof(struct ntp_packet)) {
        fprintf(stderr, "Incomplete SNTP packet received (%zd bytes).\n", received_bytes);
        close_socket(udp_socket);
        return NULL;
    }

    }

    close_socket(udp_socket);
    return payload;
}
