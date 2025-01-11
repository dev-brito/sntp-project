# Trabalho 01

> Integrantes do Grupo:

> - Silas Neres de Souza (200043536)
> - Guilherme Brito Vilas Boas (180108011)
> - Daniel Rocha Oliveira (190104821)

O trabalho foi construído na linguagem C

para rodar o projeto utilize o make run

Este trabalho consiste na criação de um Cliente para o protocolo SNTP.

O protocolo SNTP é um protocolo derivado do protocolo NTP (especificado na RFC-1305) e ambos são usados para manter os relógios de dispositivos sincronizados.

## Componentes

Para a concepção desse trabalho, foi necessário implementar componentes adicionais, como por exemplo:

### Cliente UDP

Para o cliente UDP, foi criado uma função principal chamada `send_message`.
A função `send_message` é responsável por criar e enviar uma mensagem via protocolo UDP para o servidor especificado, além de gerenciar a espera pela resposta. Ela pode ser configurada com diferentes parâmetros para ajustar o comportamento do envio e da espera pela resposta, como número de tentativas e tempo de espera.

```c
void *send_message(const char *peer_ip, int peer_port, void *payload, struct send_message_flags flags);
```

Essa função recebe por parâmetro o IP e porta de envio do pacote UDP (**peer_ip** e **peer_port**), o pacote que será enviado (**payload**) e um parâmetro adicional de algumas flags possíveis para configuração do envio da mensagem:

```c
struct send_message_flags {
    int retries; // Define o número de tentativas que o cliente deve realizar caso o envio da mensagem ou o recebimento da resposta falhe. O valor padrão é 2, conforme especificado no relatório do Trabalho.
    bool wait_for_answer; // Indica se o cliente deve aguardar uma resposta após enviar o pacote UDP. Para o cenário deste trabalho, este valor deve ser configurado como true, uma vez que o cliente espera receber uma resposta contendo informações sobre o tempo.
    int timeout; // Define o tempo máximo (em segundos) que o cliente deve aguardar pela resposta do servidor antes de abortar a operação. O valor padrão é configurado para 20 segundos, conforme especificado no relatório do Trabalho.
};
```

#### Retorno:

A função retorna um ponteiro `void*`, que pode ser utilizado para acessar a resposta do servidor (se `wait_for_answer` for `true`). Caso não haja resposta ou o tempo de espera seja excedido, a função pode retornar `NULL` ou um valor indicativo de falha, dependendo da implementação interna.

---

### Parseamento e Recebimento da Mensagem

Após o recebimento da mensagem enviada pelo servidor, o código implementa uma lógica robusta para parsear e processar a resposta. O objetivo é garantir que os dados retornados pelo servidor estejam no formato esperado do protocolo SNTP e, em caso de falha, tratar os erros adequadamente.

#### 1. **Recebendo a Resposta do Servidor**
- A função utiliza `recvfrom()` para capturar os dados enviados pelo servidor.
- Antes de processar a mensagem, o tamanho dos dados recebidos é validado para garantir conformidade com o tamanho mínimo esperado para um pacote SNTP.

```c
int received_bytes = recvfrom(udp_socket, buffer, sizeof(buffer), 0, NULL, NULL);
if (received_bytes < sizeof(struct ntp_packet)) {
    fprintf(stderr, "Pacote SNTP inválido ou incompleto recebido.\n");
    close(udp_socket);
    return NULL;
}
```

#### 2. **Validando o Formato do Pacote**
- O pacote recebido é interpretado como uma estrutura `ntp_packet`.
- Os campos da estrutura são validados para verificar se os dados correspondem ao esperado:
  - O campo **`li_vn_mode`** é usado para verificar a versão do protocolo e o modo de operação.
  - Outros campos, como **`stratum`** e **`precision`**, são analisados para determinar se o servidor está sincronizado corretamente.

```c
struct ntp_packet *response = (struct ntp_packet *)buffer;
if ((response->li_vn_mode & 0x07) != 4) { // Verifica se o modo é Server (4)
    fprintf(stderr, "Resposta recebida não está no modo esperado (Server).\n");
    close(udp_socket);
    return NULL;
}
```

#### 3. **Extração de Dados de Tempo**
- A partir do pacote SNTP, os timestamps relevantes são extraídos:
  - **`rxTm_s`** (Receive Timestamp): Indica o horário em que o servidor recebeu a requisição do cliente.
  - **`txTm_s`** (Transmit Timestamp): Indica o horário em que o servidor enviou a resposta.

- Os timestamps são convertidos para um formato legível (e.g., UNIX Epoch) para cálculos adicionais:

```c
uint32_t server_time = ntohl(response->txTm_s); // Converte para a ordem de bytes do host
server_time -= 2208988800U; // Ajusta para o Epoch UNIX (1970)
printf("Horário recebido do servidor: %u\n", server_time);
```
#### 4.**Conversão de Data e Hora em Português**

- Foi feita uma tradução da data e hora para o português, através do código a seguir que converte a "data/hora" do formato struct tm para uma string com nomes de dias da semana e meses traduzidos para português.

```c
struct tm *tm_info = localtime(&tx_time);
const char *dias_da_semana_pt[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sáb"};
const char *meses_pt[] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};

char dia_semana[4];
strftime(dia_semana, sizeof(dia_semana), "%a", tm_info);
for (int i = 0; i < 7; i++) {
    if (strcmp(dia_semana, "Sun") == 0) strcpy(dia_semana, dias_da_semana_pt[0]);
    // Outras comparações semelhantes...
}
```

#### 5. **Tratamento de Erros**
- Em caso de erro no parseamento ou validação:
  - Uma mensagem de erro é registrada no log.
  - O socket é fechado e a função retorna `NULL`.

Exemplo de erro ao processar um campo inválido:
```c
if (response->stratum == 0 || response->stratum > 16) {
    fprintf(stderr, "Servidor não sincronizado ou resposta inválida.\n");
    close(udp_socket);
    return NULL;
}
```

---

### Comportamento em Caso de Falha

- **Recebimento Incompleto:** Se o tamanho dos dados recebidos for menor que o esperado, o socket é fechado e uma mensagem de erro é exibida.
- **Pacote Inválido:** Caso os campos do pacote não estejam no formato esperado, o cliente trata como erro e retorna `NULL`.
- **Servidor Não Sincronizado:** Se o servidor indicar que não está sincronizado (e.g., Stratum 0), o cliente registra o problema e encerra a conexão.

---

### Mensagens de Log
- **`Pacote SNTP inválido ou incompleto recebido.`**: Indica que os dados recebidos não têm o tamanho mínimo esperado para um pacote SNTP.
- **`Resposta recebida não está no modo esperado (Server).`**: Informa que o pacote não está no modo correto para respostas SNTP.
- **`Servidor não sincronizado ou resposta inválida.`**: Indica que o servidor está fora de sincronização ou que os campos do pacote estão incorretos.
- **`Horário recebido do servidor:`**: Exibe o horário recebido do servidor em formato UNIX Epoch.

