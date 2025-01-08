# Trabalho 01

> Integrantes do Grupo: 
>
> - Guilherme Brito Vilas Boas (180108011)

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

Essa função recebe por parâmetro o ip e porta de envio do pacote UDP (**peer_ip** e **peer_port**), o pacote que será enviado (**payload**) e um parâmetro adicional de algumas flags possíveis para configuração do envio da mensagem:

```c
struct send_message_flags {
    int retries; # Define o número de tentativas que o cliente deve realizar caso o envio da mensagem ou o recebimento da resposta falhe. O valor padrão é 2, conforme especificado no relatório do Trabalho.
    bool wait_for_answer; # Indica se o cliente deve aguardar uma resposta após enviar o pacote UDP. Para o cenário deste trabalho, este valor deve ser configurado como true, uma vez que o cliente espera receber uma resposta contendo informações sobre o tempo.
    int timeout; # Define o tempo máximo (em segundos) que o cliente deve aguardar pela resposta do servidor antes de abortar a operação. O valor padrão é configurado para 20 segundos, conforme especificado no relatório do Trabalho.
};
```

#### Retorno:

A função retorna um ponteiro void*, que pode ser utilizado para acessar a resposta do servidor (se wait_for_answer for true). Caso não haja resposta ou o tempo de espera seja excedido, a função pode retornar NULL ou um valor indicativo de falha, dependendo da implementação interna.

##### Lógica de Funcionamento

### 1. **Criação do Socket UDP**
- A função inicia criando um socket UDP com a chamada à função `socket()`. 
- Se a criação do socket falhar, uma mensagem de erro é exibida, e a função retorna `NULL`.

```c
int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
if (udp_socket < 0) {
    perror("Socket creation failure.");
    return NULL;
}
```

### 2. **Configuração do Endereço de Destino**
- O endereço IP e a porta do servidor de destino são configurados na estrutura `sockaddr_in`.
  - O IP é convertido de formato string para binário usando a função `inet_aton()`.
  - A porta de destino é configurada com a função `htons()` para garantir que o valor seja interpretado corretamente pelo protocolo de rede.
- Caso o IP fornecido seja inválido, a função exibe uma mensagem de erro e retorna `NULL`.

```c
struct sockaddr_in peer_addr = {.sin_family = AF_INET, .sin_port = htons(peer_port)};

if (inet_aton(peer_ip, &peer_addr.sin_addr) <= 0) {
    perror("Invalid IP address.");
    return NULL;
}

int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
```

### 3. **Envio da Mensagem**
- O código tenta enviar o pacote de dados (payload) para o servidor utilizando a função `sendto()`.
  - O número de tentativas de envio é controlado pelo parâmetro `retries` da estrutura `flags`.
  - Caso o envio falhe, o socket é fechado e a função retorna `NULL`.

### 4. **Aguarda Resposta**
- Se a flag `wait_for_answer` for configurada como `true`, a função aguarda a resposta do servidor.
  - A função utiliza a chamada `select()` para aguardar por uma resposta dentro do período de tempo configurado em `timeout`.
  - A função verifica se o servidor respondeu dentro do tempo limite.
  - Caso o tempo expire sem resposta, o código tenta reenviar a mensagem até atingir o número máximo de tentativas configurado.
  - Se `select()` retornar um erro, o socket é fechado e a função retorna `NULL`.

### 5. **Recebimento da Resposta**
- Quando uma resposta do servidor é recebida, o código a processa com a função `recvfrom()`.
  - O tamanho dos dados recebidos é comparado com o tamanho esperado de um pacote SNTP.
  - Se a quantidade de dados recebidos for menor do que o esperado, um erro é gerado e o socket é fechado.

### 6. **Fechamento do Socket**
- Após o envio ou recebimento da mensagem, o socket é fechado com a função `close_socket()`.
  - A função `close_socket()` garante que o socket seja fechado corretamente, liberando os recursos do sistema.

---

### Comportamento em Caso de Falha:
- **Erro no Envio**: Se a função `sendto()` falhar, o socket é fechado e a função retorna `NULL`.
- **Sem Resposta**: Se a resposta não for recebida dentro do tempo limite, o código tenta reenviar a mensagem até o número máximo de tentativas.
- **Erro ao Receber**: Se houver erro ao receber a resposta ou se os dados recebidos não forem completos, o socket é fechado e a função retorna `NULL`.

---

### Mensagens de Log
- **`Sending Message`**: Indica que a mensagem está sendo enviada para o servidor.
- **`Esperando resposta do servidor...`**: Informa que o cliente está aguardando a resposta do servidor.
- **`O tempo expirou e não houve resposta do servidor.`**: Aparece caso o servidor não envie uma resposta dentro do tempo limite.
- **`Erro durante o select.`**: Aparece se ocorrer um erro na função `select()` ao aguardar pela resposta.
- **`Erro ao receber a mensagem.`**: Aparece se houver erro ao receber a resposta do servidor.

