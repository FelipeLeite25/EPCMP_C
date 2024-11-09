#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* nome;
    char* tipo;
    int valor_int;
    char* valor_string;
} Variavel;

// Função para criar uma variável
Variavel* criar_variavel(char* nome, char* tipo) {
    Variavel* v = (Variavel*) malloc(sizeof(Variavel));
    v->nome = nome;
    v->tipo = tipo;
    v->valor_string = NULL;
    v->valor_int = 0;
    return v;
}

// Função para definir o nome de uma variável
void set_nome(Variavel* v, char* nome) {
    v->nome = nome;
}

// Função para obter o nome de uma variável
char* get_nome(Variavel* v) {
    return v->nome;
}

// Função para obter as variáveis do arquivo
Variavel** obter_variaveis_do_arquivo(char** leitura, int linhas, int* qtd_variaveis) {
    int inicio_declaracao_var = -1, fim_de_declaracao = -1, i, p = 0;

    // Encontrar o início e fim da declaração de variáveis
    for (i = 0; i < linhas; i++) {
        if (strstr(leitura[i], ".var")) {
            inicio_declaracao_var = i;
            p++;
        } else if (strstr(leitura[i], ".end-var")) {
            fim_de_declaracao = i;
            p++;
        }
        if (p == 2) break;
    }

    if (inicio_declaracao_var == -1 || fim_de_declaracao == -1) {
        *qtd_variaveis = 0;
        return NULL;
    }

    // Criar variáveis entre os delimitadores encontrados
    *qtd_variaveis = fim_de_declaracao - inicio_declaracao_var - 1;
    Variavel** variaveis = (Variavel**) malloc(*qtd_variaveis * sizeof(Variavel*));

    for (i = inicio_declaracao_var + 1, p = 0; i < fim_de_declaracao; i++, p++) {
        variaveis[p] = criar_variavel(leitura[i], "int");
    }
    return variaveis;
}

// Função para procurar uma letra em uma string
int procurar_letra(const char* palavra, char letra) {
    for (int i = 0; palavra[i] != '\0'; i++) {
        if (palavra[i] == letra) {
            return i;
        }
    }
    return -1;
}

// Função para definir o valor de uma variável com base na leitura do arquivo
void set_valor(Variavel* v, char** leitura, int linhas) {
    if (strcmp(v->tipo, "String") == 0) {
        v->valor_string = NULL;
    } else if (strcmp(v->tipo, "int") == 0) {
        int linha_de_atribuicao = -1;

        // Procurar a linha de atribuição
        for (int i = 0; i < linhas; i++) {
            if (strstr(leitura[i], "istore ") && strstr(leitura[i], v->nome)) {
                linha_de_atribuicao = i;
                break;
            }
        }

        if (linha_de_atribuicao > 0 && strstr(leitura[linha_de_atribuicao - 1], "bipush")) {
            int index_letra = procurar_letra(leitura[linha_de_atribuicao - 1], 'h');
            if (index_letra != -1) {
                char* valor_ini = &leitura[linha_de_atribuicao - 1][index_letra + 1];
                v->valor_int = atoi(valor_ini);
            }
        }
    } else {
        v->valor_string = "ERROR";
    }
}

// Função para verificar se uma variável é de condição de loop
int testar_se_e_variavel_de_condicao_de_loop(Variavel* v, char** leitura, int linhas) {
    int quantidade_de_loops = 0;
    int* linhas_do_loop = NULL;

    // Contar a quantidade de loops e suas linhas
    for (int i = 0; i < linhas; i++) {
        if (strstr(leitura[i], "LOOPTEST")) {
            quantidade_de_loops++;
            linhas_do_loop = (int*) realloc(linhas_do_loop, quantidade_de_loops * sizeof(int));
            linhas_do_loop[quantidade_de_loops - 1] = i;
        }
    }

    // Verificar se a variável é de loop
    for (int i = 0; i < quantidade_de_loops; i++) {
        char* linha_iload = leitura[linhas_do_loop[i] + 1];
        if (linha_iload != NULL) {
            char* nome_variavel_loop = &linha_iload[4];
            if (strcmp(v->nome, nome_variavel_loop) == 0) {
                free(linhas_do_loop);
                return 1;
            }
        }
    }
    free(linhas_do_loop);
    return 0;
}

// Função de exemplo para liberar a memória alocada para variáveis
void liberar_variaveis(Variavel** variaveis, int qtd) {
    for (int i = 0; i < qtd; i++) {
        free(variaveis[i]);
    }
    free(variaveis);
}

// Exemplo de uso
int main() {
    // Aqui, as leituras seriam passadas como argumento ou carregadas de um arquivo
    // Por exemplo:
    char* leitura[] = {".var", "variavel1", ".end-var", "istore variavel1"};
    int qtd_variaveis;

    // Obter as variáveis
    Variavel** variaveis = obter_variaveis_do_arquivo(leitura, 4, &qtd_variaveis);

    // Definir valor e verificar condições (exemplo de uso das funções)
    if (qtd_variaveis > 0) {
        set_valor(variaveis[0], leitura, 4);
        int is_loop_var = testar_se_e_variavel_de_condicao_de_loop(variaveis[0], leitura, 4);
        printf("Variavel '%s' e de loop: %s\n", get_nome(variaveis[0]), is_loop_var ? "Sim" : "Nao");
    }

    // Libera memória das variáveis
    liberar_variaveis(variaveis, qtd_variaveis);
    return 0;
}
