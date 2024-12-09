#include "../libraries/basic_client_udp.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// Definição do pacote NTP
struct ntp_packet {
    uint8_t li_vn_mode;      // Leap Indicator, Version Number e Mode
    uint8_t stratum;         // Stratum
    uint8_t poll;            // Poll Interval
    uint8_t precision;       // Precision
    uint32_t rootDelay;      // Root Delay
    uint32_t rootDispersion; // Root Dispersion
    uint32_t refId;          // Reference ID
    uint32_t refTm_s;        // Reference Timestamp (segundos)
    uint32_t refTm_f;        // Reference Timestamp (fração de segundo)
    uint32_t origTm_s;       // Originate Timestamp (segundos)
    uint32_t origTm_f;       // Originate Timestamp (fração de segundo)
    uint32_t rxTm_s;         // Receive Timestamp (segundos)
    uint32_t rxTm_f;         // Receive Timestamp (fração de segundo)
    uint32_t txTm_s;         // Transmit Timestamp (segundos)
    uint32_t txTm_f;         // Transmit Timestamp (fração de segundo)
};

// Função para interpretar os dados do pacote NTP
void parse_ntp_packet(struct ntp_packet *packet) {
    // Extração dos valores de li, vn e mode
    uint8_t li = (packet->li_vn_mode >> 6) & 0x03;  // Leap Indicator
    uint8_t vn = (packet->li_vn_mode >> 3) & 0x07;  // Version Number
    uint8_t mode = packet->li_vn_mode & 0x07;       // Mode

    printf("Leap Indicator: %u\n", li);
    printf("Version Number: %u\n", vn);
    printf("Mode: %u\n", mode);

    // Conversão do timestamp de 1900 para 1970 (Unix Epoch)
    const uint32_t epoch_offset = 2208988800U; // Segundos entre 1900 e 1970
    time_t tx_time = ntohl(packet->txTm_s) - epoch_offset;
    printf("Transmit Timestamp: %s", ctime(&tx_time));
}

// Função principal
int main(int argc, char const *argv[]) {
    struct send_message_flags flags = {.wait_for_answer = true, .timeout = 20, .retries = 1};
    struct ntp_packet ntp_packet;

    // Enviar mensagem e receber resposta
    struct ntp_packet *response = (struct ntp_packet *)send_message("127.0.0.1", 9800, (void *)&ntp_packet, flags);

    // Processar resposta
    if (response) {
        parse_ntp_packet(response);
    } else {
        printf("Failed to receive response from server.\n");
    }

    return 0;
}
