#include "../libraries/basic_client_udp.h"
#include "../libraries/ntp_packet.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include <string.h>
#include <locale.h> 



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
    
    // Usando localtime para obter a estrutura tm com a data
    struct tm *tm_info = localtime(&tx_time);

    // Mapeando os nomes dos dias da semana para português
    const char *dias_da_semana_pt[] = {
        "Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sáb"
    };

    // Mapeando os nomes dos meses para português
    const char *meses_pt[] = {
        "Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
        "Jul", "Ago", "Set", "Out", "Nov", "Dez"
    };

    // Obtendo o nome do dia e do mês em inglês
    char dia_semana[4];
    strftime(dia_semana, sizeof(dia_semana), "%a", tm_info);

    char mes[4];
    strftime(mes, sizeof(mes), "%b", tm_info);

    // Traduzindo o nome do dia da semana
    for (int i = 0; i < 7; i++) {
        if (strcmp(dia_semana, "Sun") == 0) strcpy(dia_semana, dias_da_semana_pt[0]);
        if (strcmp(dia_semana, "Mon") == 0) strcpy(dia_semana, dias_da_semana_pt[1]);
        if (strcmp(dia_semana, "Tue") == 0) strcpy(dia_semana, dias_da_semana_pt[2]);
        if (strcmp(dia_semana, "Wed") == 0) strcpy(dia_semana, dias_da_semana_pt[3]);
        if (strcmp(dia_semana, "Thu") == 0) strcpy(dia_semana, dias_da_semana_pt[4]);
        if (strcmp(dia_semana, "Fri") == 0) strcpy(dia_semana, dias_da_semana_pt[5]);
        if (strcmp(dia_semana, "Sat") == 0) strcpy(dia_semana, dias_da_semana_pt[6]);
    }

    // Traduzindo o nome do mês
    for (int i = 0; i < 12; i++) {
        if (strcmp(mes, "Jan") == 0) strcpy(mes, meses_pt[0]);
        if (strcmp(mes, "Feb") == 0) strcpy(mes, meses_pt[1]);
        if (strcmp(mes, "Mar") == 0) strcpy(mes, meses_pt[2]);
        if (strcmp(mes, "Apr") == 0) strcpy(mes, meses_pt[3]);
        if (strcmp(mes, "May") == 0) strcpy(mes, meses_pt[4]);
        if (strcmp(mes, "Jun") == 0) strcpy(mes, meses_pt[5]);
        if (strcmp(mes, "Jul") == 0) strcpy(mes, meses_pt[6]);
        if (strcmp(mes, "Aug") == 0) strcpy(mes, meses_pt[7]);
        if (strcmp(mes, "Sep") == 0) strcpy(mes, meses_pt[8]);
        if (strcmp(mes, "Oct") == 0) strcpy(mes, meses_pt[9]);
        if (strcmp(mes, "Nov") == 0) strcpy(mes, meses_pt[10]);
        if (strcmp(mes, "Dec") == 0) strcpy(mes, meses_pt[11]);
    }

    // Formatando a data e hora no formato solicitado
    char data_hora[100];
    snprintf(data_hora, sizeof(data_hora), "Data/hora: %s %s %02d %02d:%02d:%02d %d",
             dia_semana, mes, tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, 1900 + tm_info->tm_year);
    
    // Imprimindo a data/hora no formato desejado
    printf("%s\n", data_hora);
}

// Função principal
int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;
    struct send_message_flags flags = {.wait_for_answer = true, .timeout = 20, .retries = 1};
    struct ntp_packet ntp_packet;
    
    memset(&ntp_packet, 0, sizeof(ntp_packet));
    ntp_packet.li_vn_mode = 0x1B; // li = 0, vn = 3, mode = 3

    // Enviar mensagem e receber resposta
    struct ntp_packet *response = (struct ntp_packet *)send_message("129.6.15.28", 123, (void *)&ntp_packet, flags);

    // Processar resposta
    if (response) {
        parse_ntp_packet(response);
    } else {
        printf("Failed to receive response from server.\n");
    }

    return 0;
}
