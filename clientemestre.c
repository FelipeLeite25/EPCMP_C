#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Variáveis de controle compartilhadas
int Start = 0, Close = 0, Load = 0;

// Estrutura ClienteMestre para armazenar as informações do cliente e conexão
typedef struct {
    int porta_inicial;
    int quantidade_servidores;
    struct sockaddr_in servidor_addr;
} ClienteMestre;

// Funções de controle de fase
void setFaseStart(int b) { Start = b; }
void setFaseClose(int b) { Close = b; }
void setLoad(int b) { Load = b; }

// Função para enviar mensagens ao servidor
void enviar_mensagem(int socket_fd, const char *msg) {
    send(socket_fd, msg, strlen(msg), 0);
}

// Função para receber mensagens do servidor
void* receber_mensagem(void* arg) {
    int socket_fd = *((int*) arg);
    char buffer[BUFFER_SIZE];

    while (!Close) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("Mensagem recebida: %s\n", buffer);

            if (strcmp(buffer, "Load Iniciado") == 0) { setLoad(0); }
            if (strcmp(buffer, "Start Iniciado") == 0) { setFaseStart(0); }
        }
        sleep(1);
    }

    close(socket_fd);
    free(arg);  // Libera a memória alocada para o socket_fd
    return NULL;
}

// Função para conectar a vários servidores e iniciar threads para cada um
void* iniciar_conexao(void* args) {
    ClienteMestre* cliente = (ClienteMestre*) args;
    int iniPort = cliente->porta_inicial;
    int quantidade_servidores = cliente->quantidade_servidores;

    for (int i = 0; i < quantidade_servidores; i++) {
        int* socket_fd = malloc(sizeof(int));
        *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (*socket_fd < 0) {
            perror("Erro ao criar o socket");
            free(socket_fd);
            continue;
        }

        cliente->servidor_addr.sin_family = AF_INET;
        cliente->servidor_addr.sin_port = htons(iniPort + i);
        cliente->servidor_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(*socket_fd, (struct sockaddr*)&cliente->servidor_addr, sizeof(cliente->servidor_addr)) < 0) {
            perror("Erro ao conectar ao servidor");
            close(*socket_fd);
            free(socket_fd);
            continue;
        }

        printf("Conectado ao servidor %d na porta %d\n", i, iniPort + i);
        
        if (i == 0) {
            enviar_mensagem(*socket_fd, "Inicializar");
        }

        // Criar uma thread para cada conexão e passar uma cópia do socket_fd
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, receber_mensagem, (void*) socket_fd);
        pthread_detach(thread_id);  // Permite que a thread limpe seus recursos automaticamente ao terminar
    }

    return NULL;
}

// Função que define o comportamento da thread principal de comunicação
void* thread_run(void* args) {
    ClienteMestre* cliente = (ClienteMestre*) args;
    int i_Start = 0, i_Load = 0;

    while (!Close) {
        // Verifique o estado e envie mensagens apropriadas
        if (Load && i_Load == 0) {
            enviar_mensagem(cliente->socket_fd, "Load");
            i_Load = 1;
        } else if (Start && i_Start == 0) {
            enviar_mensagem(cliente->socket_fd, "Start");
            i_Start = 1;
        } else {
            enviar_mensagem(cliente->socket_fd, "Default");
        }

        if (!Load) {
            i_Load = 0;
        }
        if (!Start) {
            i_Start = 0;
        }

        sleep(1);  // Pausa para evitar uso excessivo de CPU
    }

    return NULL;
}

// Função principal para iniciar o cliente mestre
int main() {
    int porta_inicial = 8080;
    int quantidade_servidores = 3;

    ClienteMestre cliente;
    cliente.porta_inicial = porta_inicial;
    cliente.quantidade_servidores = quantidade_servidores;
    cliente.servidor_addr.sin_family = AF_INET;

    // Conecta e inicia a comunicação com os servidores
    pthread_t conn_thread;
    pthread_create(&conn_thread, NULL, iniciar_conexao, (void*)&cliente);
    pthread_join(conn_thread, NULL);

    return 0;
}
