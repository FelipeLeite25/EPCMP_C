#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* nome;
    char* tipo;
    int valor_int;
    char* valor_string;
} Variavel;

typedef struct {
    char* tipo_de_condicao;
    Variavel* variavel_de_condicao;
    Variavel** variaveis_de_condicao;
    char** conteudo_loop;
    int inicio_loop;
    int fim_loop;
} Loop;

// Função para criar um novo loop
Loop* criar_loop(char* tipo_de_condicao, Variavel* variavel, char** conteudo_loop, int inicio, int fim) {
    Loop* l = (Loop*) malloc(sizeof(Loop));
    l->tipo_de_condicao = tipo_de_condicao;
    l->variavel_de_condicao = variavel;
    l->conteudo_loop = conteudo_loop;
    l->inicio_loop = inicio;
    l->fim_loop = fim;
    return l;
}

// Função para definir os limites do loop
void set_limites(Loop* l) {
    if (strcmp(l->tipo_de_condicao, "Default") == 0) {
        if (strcmp(l->variavel_de_condicao->tipo, "int") == 0) {
            l->inicio_loop = l->variavel_de_condicao->valor_int;
            l->fim_loop = obter_valor_de_condicao(l);
        }
    }
}

// Função para obter o valor de condição do loop
int obter_valor_de_condicao(Loop* l) {
    int i, linhaif = -1;

    // Procurar linha com "if_icmpeq" e capturar o valor na linha anterior
    for (i = 0; l->conteudo_loop[i] != NULL; i++) {
        if (strstr(l->conteudo_loop[i], "if_icmpeq")) {
            linhaif = i;
            break;
        }
    }
    
    if (linhaif > 0) {
        char* linha_valor = l->conteudo_loop[linhaif - 1];
        int valor = atoi(&linha_valor[6]);
        return valor + 1;
    }
    return 0;
}

// Função para testar se o número é par
int testar_se_o_numero_e_par(int numero) {
    return (numero % 2 == 0);
}

// Função para criar um novo conteúdo com base em alterações nos limites
char** criar_novo_conteudo(Loop* l, int acrescimos, int numero_loop) {
    int i = 0;
    int novo_tamanho = (numero_loop == 0) ? l->fim_loop : l->fim_loop + 2;
    char** newconteudo = (char**) malloc(novo_tamanho * sizeof(char*));

    int novo_termino = l->inicio_loop + (acrescimos * (numero_loop + 1));
    int novo_inicio = l->inicio_loop + (acrescimos * numero_loop);

    for (i = 0; l->conteudo_loop[i] != NULL; i++) {
        if (numero_loop == 0 && strcmp(l->conteudo_loop[i], "bipush ") == 0) {
            sprintf(newconteudo[i], "bipush %d", novo_termino);
        } else {
            newconteudo[i] = strdup(l->conteudo_loop[i]);
        }
    }

    return newconteudo;
}

// Função para determinar o fator de divisão
int determinar_fator_de_divisao(int max_n, int intervalo_l) {
    if ((intervalo_l % max_n) == 0) {
        return intervalo_l / max_n;
    } else {
        return (intervalo_l / max_n) + 1;
    }
}

// Função para dividir loops
Loop** dividir_loops(Loop* l, int max_nucleos, int* qtd_loops) {
    int intervalo_loop = l->fim_loop - l->inicio_loop;
    int fator_de_divisao = determinar_fator_de_divisao(max_nucleos, intervalo_loop);
    int quantidade_loops = (intervalo_loop < 0) ? -intervalo_loop : intervalo_loop;

    if (testar_se_o_numero_e_par(fator_de_divisao) == testar_se_o_numero_e_par(intervalo_loop)) {
        quantidade_loops /= fator_de_divisao;
    } else {
        quantidade_loops = (intervalo_loop / fator_de_divisao) + 1;
    }

    *qtd_loops = quantidade_loops;
    Loop** novosloops = (Loop**) malloc(quantidade_loops * sizeof(Loop*));
    int acrescimo_no_limitador = intervalo_loop / quantidade_loops;

    for (int i = 0; i < quantidade_loops; i++) {
        char** novo_conteudo = criar_novo_conteudo(l, acrescimo_no_limitador, i);
        int novo_inicio = l->inicio_loop + (acrescimo_no_limitador * i);
        int novo_final = l->inicio_loop + (acrescimo_no_limitador * (i + 1));
        novosloops[i] = criar_loop("Default", l->variavel_de_condicao, novo_conteudo, novo_inicio, novo_final);
    }
    
    return novosloops;
}

// Funções auxiliares para imprimir valores de teste
void print_loop(Loop* l) {
    printf("Loop Tipo: %s, Inicio: %d, Fim: %d\n", l->tipo_de_condicao, l->inicio_loop, l->fim_loop);
    for (int i = 0; l->conteudo_loop[i] != NULL; i++) {
        printf("Conteudo: %s\n", l->conteudo_loop[i]);
    }
}

void liberar_loop(Loop* l) {
    free(l->tipo_de_condicao);
    free(l->conteudo_loop);
    free(l);
}

int main() {
    // Exemplo de conteúdo do loop
    char* conteudo_loop[] = {"bipush 0", "istore variavel", "if_icmpeq", NULL};

    Variavel var = {"variavel", "int", 0, NULL};
    Loop* loop_principal = criar_loop("Default", &var, conteudo_loop, 0, 10);

    set_limites(loop_principal);
    int qtd_loops;
    Loop** loops_divididos = dividir_loops(loop_principal, 4, &qtd_loops);

    for (int i = 0; i < qtd_loops; i++) {
        print_loop(loops_divididos[i]);
        liberar_loop(loops_divididos[i]);
    }

    free(loops_divididos);
    liberar_loop(loop_principal);

    return 0;
}
