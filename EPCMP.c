#include <stdio.h>
#include <stdlib.h>

// Declaração das funções externas
void iniciar_start_main();
void iniciar_monitoring_frame(char *endereco, int porta, int quantidade_divisoes);

int main(int argc, char *argv[]) {
    // Verifica se há argumentos para decidir qual interface inicializar
    if (argc == 1) {
        // Nenhum argumento passado; inicia o `start_main`
        iniciar_start_main();
    } else if (argc == 4) {
        // Três argumentos: inicia o `monitoring_frame`
        char *endereco = argv[1];
        int porta = atoi(argv[2]);
        int quantidade_divisoes = atoi(argv[3]);
        iniciar_monitoring_frame(endereco, porta, quantidade_divisoes);
    } else {
        printf("Uso: \n");
        printf("  ./main                   -> Inicia start_main\n");
        printf("  ./main <endereco> <porta> <quantidade_divisoes> -> Inicia monitoring_frame\n");
        return 1;
    }

    return 0;
}
