#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int id;
    char* caminho_binario;  // Caminho para o binário RISC-V
    char* endereco;
    int porta;
} ProcessadorV;

// Função que executa o simulador RISC-V no QEMU (equivalente ao método `executar_Mic` em Java)
void executar_riscv(ProcessadorV* proc) {
    char comando_de_chamada[256];
    snprintf(comando_de_chamada, sizeof(comando_de_chamada), 
             "qemu-system-riscv64 -nographic -machine virt -kernel %s -m 256M", proc->caminho_binario);
    
    printf("Executando comando: %s\n", comando_de_chamada);
    system(comando_de_chamada);
}

// Função que será executada pela thread (equivalente ao método `run` em Java)
void* thread_run(void* args) {
    ProcessadorV* proc = (ProcessadorV*) args;
    executar_riscv(proc);
    return NULL;
}

// Função para criar e inicializar um ProcessadorV (equivalente ao construtor em Java)
ProcessadorV* criar_processador(int id, char* caminho_binario, char* endereco, int porta) {
    ProcessadorV* novo_proc = (ProcessadorV*) malloc(sizeof(ProcessadorV));
    novo_proc->id = id;
    novo_proc->caminho_binario = caminho_binario;
    novo_proc->endereco = endereco;
    novo_proc->porta = porta;
    return novo_proc;
}

int main() {
    // Cria e inicializa o processador com o id, caminho do binário, endereço e porta
    ProcessadorV* proc = criar_processador(1, "programa_riscv.bin", "localhost", 8080);

    // Cria a thread para executar o comando
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_run, (void*) proc);

    // Aguarda a conclusão da thread
    pthread_join(thread_id, NULL);

    // Libera a memória alocada
    free(proc);

    return 0;
}
