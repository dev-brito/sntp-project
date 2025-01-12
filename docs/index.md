<style>body {text-align: justify}</style>

> Integrantes do Grupo: 
>
> Guilherme Brito Vilas Boas (190108011)
> 
> Silas Neres de Souza (200043536)
> 
> Daniel Rocha Oliveira (190104821)


> Sistema Utilizado: Linux Ubuntu 22.04
> 
> Ambiente de Desenvolvimento: Visual Studio Code
> 
> Linguagem: C

---

# Build

Para a build desse projeto, é necessário rodar o comando `make project` para que o arquivo executável seja gerado.

---

# Execução do Programa

Para a execução do programa, basta, após a **build**, executar o comando `./project <IP SERVIDOR SNTP>`.

> exemplo: `./project 129.6.15.28`

> ![exemplo de execução](example.png)

---

# Introdução

Este trabalho tem como objetivo a criação de um cliente para o protocolo SNTP.

O SNTP (Simple Network Time Protocol) é um protocolo derivado do NTP (Network Time Protocol), especificado pela [RFC-1305](https://www.ntp.org/reflib/rfc/rfc1305/rfc1305b.pdf). Ambos os protocolos são utilizados para manter os relógios dos dispositivos sincronizados.

## Componentes

Para a realização deste trabalho, foi necessário o desenvolvimento de componentes adicionais, como, por exemplo um `Cliente Genérico UDP`.

### Cliente UDP

Neste tópico, será apresentada a descrição da pequena biblioteca de um cliente UDP genérico desenvolvida pelo grupo para enviar mensagens via protocolo UDP para um servidor. 

A função **principal** send_message é responsável por enviar uma mensagem via protocolo UDP para o servidor especificado, além de gerenciar a espera pela resposta. Ela pode ser configurada com diferentes parâmetros para ajustar o comportamento do envio e da espera pela resposta, como número de tentativas e tempo de espera.

```c
void *send_message(const char *peer_ip, int peer_port, void *payload, struct send_message_flags flags);
```

Essa função recebe por parâmetro o ip e porta de envio do pacote UDP (**peer_ip** e **peer_port**), o pacote que será enviado (**payload**) e um parâmetro adicional de algumas flags possíveis para configuração do comportamento da função:

```c
struct send_message_flags {
    int retries; # Define o número de tentativas que o cliente deve realizar caso o envio da mensagem ou o recebimento da resposta falhe. O valor padrão é 2 (1 + 1 tentativa), conforme especificado no relatório do Trabalho.
    bool wait_for_answer; # Indica se o cliente deve aguardar uma resposta após enviar o pacote UDP. Para o cenário deste trabalho, este valor deve ser configurado como true, uma vez que o cliente SNTP espera receber uma resposta do servidor contendo informações sobre o tempo.
    int timeout; # Define o tempo máximo (em segundos) que o cliente deve aguardar pela resposta do servidor antes de abortar a operação. O valor padrão é configurado para 20 segundos, conforme especificado no relatório do Trabalho.
```

---

#### Retorno

A função retorna um ponteiro void*, que pode ser utilizado para acessar a resposta do servidor (se `wait_for_answer` for true). Caso não haja resposta ou o tempo de espera seja excedido, a função retorna NULL.

---

#### Lógica de Funcionamento


##### 1. **Configuração do Endereço de Destino**

- O endereço IP e a porta do servidor de destino são configurados na estrutura [`sockaddr_in`](https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html).
- O IP é convertido de formato string para binário usando a função [`inet_aton()`](https://www.gta.ufrj.br/ensino/eel878/sockets/inet_ntoaman.html).
- A porta de destino é configurada com a função [`htons()`](https://www.gta.ufrj.br/ensino/eel878/sockets/htonsman.html) para garantir que o valor seja interpretado corretamente pelo protocolo de rede.
- Caso o IP fornecido seja inválido, a função exibe uma mensagem de erro e retorna `NULL`.

```c
struct sockaddr_in peer_addr = {.sin_family = AF_INET, .sin_port = htons(peer_port)};

if (inet_aton(peer_ip, &peer_addr.sin_addr) <= 0) {
    perror("Invalid IP address.");
    return NULL;
}
```

##### 2. **Criação do Socket UDP**

- A função segue tentando criar um socket UDP com a chamada à função [`socket()`](https://man7.org/linux/man-pages/man2/socket.2.html). 
- Se a criação do socket falhar, uma mensagem de erro é exibida, e a função retorna `NULL`.

```c
int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
if (udp_socket < 0) {
    perror("Socket creation failure.");
    return NULL;
}
```

##### 3. **Envio da Mensagem**

- O código tenta enviar o pacote de dados (payload) para o servidor utilizando a função [`sendto()`](https://pubs.opengroup.org/onlinepubs/009604499/functions/sendto.html).
  - Caso o envio falhe, o socket é fechado e a função retorna `NULL`.

```c
  if (sendto(udp_socket, payload, sizeof(struct ntp_packet), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
            perror("Sending message failure.");
            close_socket(udp_socket);
            return NULL;
        }
```

##### 4. **Aguarda Resposta**

- Se a flag `wait_for_answer` for configurada como `true`, a função aguarda a resposta do servidor.
- A função utiliza a chamada [`select()`](https://man7.org/linux/man-pages/man2/select.2.html) para aguardar por uma resposta dentro do período de tempo configurado na flag `timeout`.
- A função verifica se o servidor respondeu dentro do tempo limite.
- Caso o tempo expire sem resposta, o código tenta reenviar a mensagem até atingir o número máximo de tentativas configurado.
- Se `select()` retornar um erro, o socket é fechado e a função retorna `NULL`.

```c
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
```

##### 5. **Recebimento da Resposta**

- Quando uma resposta do servidor é recebida, o código a processa com a função [`recvfrom()`](https://www.gta.ufrj.br/ensino/eel878/sockets/recvman.html).
- O tamanho dos dados recebidos é comparado com o tamanho esperado de um pacote SNTP.
- Se a quantidade de dados recebidos for menor do que o esperado, um erro é gerado e o socket é fechado.

```c
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
```

##### 6. **Fechamento do Socket**

- Após o envio ou recebimento da mensagem, o socket é fechado com a função `close_socket()`.
- A função [`close_socket()`](https://man7.org/linux/man-pages/man2/close.2.html) garante que o socket seja fechado corretamente, liberando os recursos do sistema.

```c
int close_socket(int socket)
{
    if (close(socket) < 0)
    {
        perror("Error closing socket.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

---

##### Exemplo de uso da biblioteca UDP

```c
struct send_message_flags flags = {.wait_for_answer = true, .timeout = 20, .retries = 1};
struct ntp_packet ntp_packet;

memset(&ntp_packet, 0, sizeof(ntp_packet));
ntp_packet.li_vn_mode = 0x1B;

struct ntp_packet *response = (struct ntp_packet *)send_message(server_ip, 123, (void *)&ntp_packet, flags);
```

---

### Parseamento e Recebimento da Mensagem

Após o recebimento da mensagem enviada pelo servidor, a mensagem é processada e parseada. O objetivo é garantir que os dados retornados pelo servidor estejam no formato esperado do protocolo SNTP e, em caso de falha, tratar os erros adequadamente.

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
