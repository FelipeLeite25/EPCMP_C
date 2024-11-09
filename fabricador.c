#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* tipo_de_condicao;
    char** conteudo_loop;
    int inicio_loop;
    int fim_loop;
} Loop;

typedef struct {
    char* nome_dos_arquivos;
    char** conteudo_completo_arquivo;
    Loop** loops;
    char* destino_do_arquivo;
    int linha_de_inicio_loop;
    int linha_de_fim_loop;
} Fabricador;

// Função para criar um novo Fabricador
Fabricador* criar_fabricador(char* nome_do_arquivo, char** conteudo, Loop** loops, char* destino_do_arquivo) {
    Fabricador* f = (Fabricador*) malloc(sizeof(Fabricador));
    f->nome_dos_arquivos = nome_do_arquivo;
    f->conteudo_completo_arquivo = conteudo;
    f->loops = loops;
    f->destino_do_arquivo = destino_do_arquivo;
    f->linha_de_inicio_loop = 0;
    f->linha_de_fim_loop = 0;
    return f;
}

// Função para definir o início e o fim do loop
void set_inicio_e_fim_do_loop(Fabricador* f) {
    for (int i = 0; f->conteudo_completo_arquivo[i] != NULL; i++) {
        if (strcmp(f->conteudo_completo_arquivo[i], "LOOPTEST0:") == 0) {
            f->linha_de_inicio_loop = i;
        } else if (strstr(f->conteudo_completo_arquivo[i], "LOOPEXIT") != NULL) {
            f->linha_de_fim_loop = i;
        }
    }
}

// Função para criar arquivos
void criar_arquivos(Fabricador* f) {
    set_inicio_e_fim_do_loop(f);
    int quantidade_de_linhas_antes_loop = f->linha_de_inicio_loop + 1;
    int quantidade_de_linhas_depois_loop = 0;
    
    if ((f->conteudo_completo_arquivo[f->linha_de_fim_loop] != NULL) &&
        (f->conteudo_completo_arquivo[f->linha_de_fim_loop + 1] == NULL)) {
        quantidade_de_linhas_depois_loop = 1;
    } else {
        quantidade_de_linhas_depois_loop = 0;
        for (int i = f->linha_de_fim_loop + 1; f->conteudo_completo_arquivo[i] != NULL; i++) {
            quantidade_de_linhas_depois_loop++;
        }
    }
    
    for (int i = 0; f->loops[i] != NULL; i++) {
        int total_linhas = quantidade_de_linhas_antes_loop + quantidade_de_linhas_depois_loop;
        for (int j = 0; f->loops[i]->conteudo_loop[j] != NULL; j++) {
            total_linhas++;
        }
        
        char** novo_conteudo = (char**) malloc((total_linhas + 1) * sizeof(char*));
        
        int p = 0;
        for (int j = 0; j < quantidade_de_linhas_antes_loop - 1; j++) {
            novo_conteudo[p] = strdup(f->conteudo_completo_arquivo[j]);
            p++;
        }

        for (int j = 0; f->loops[i]->conteudo_loop[j] != NULL; j++) {
            novo_conteudo[p] = strdup(f->loops[i]->conteudo_loop[j]);
            p++;
        }

        for (int j = f->linha_de_fim_loop + 1; f->conteudo_completo_arquivo[j] != NULL; j++) {
            novo_conteudo[p] = strdup(f->conteudo_completo_arquivo[j]);
            p++;
        }
        novo_conteudo[p] = NULL; // Marca o final do conteúdo

        char caminho_absoluto[256];
        snprintf(caminho_absoluto, sizeof(caminho_absoluto), "%s%s%d.jas", f->destino_do_arquivo, f->nome_dos_arquivos, i);
        
        FILE* file = fopen(caminho_absoluto, "w");
        if (file == NULL) {
            perror("Erro ao criar arquivo");
            continue;
        }
        
        for (int j = 0; novo_conteudo[j] != NULL; j++) {
            fprintf(file, "%s\n", novo_conteudo[j]);
            free(novo_conteudo[j]);
        }
        
        printf("Arquivo %d Pronto\n", i);
        fclose(file);
        free(novo_conteudo);
    }
}

// Função para liberar memória alocada para Fabricador
void liberar_fabricador(Fabricador* f) {
    free(f->nome_dos_arquivos);
    free(f->destino_do_arquivo);
    free(f);
}

int main() {
    // Exemplo de conteúdo e loops
    char* conteudo[] = {"Linha1", "LOOPTEST0:", "Linha2", "LOOPEXIT", NULL};
    char* conteudo_loop1[] = {"LoopLine1", "LoopLine2", NULL};
    
    Loop* loop1 = (Loop*) malloc(sizeof(Loop));
    loop1->conteudo_loop = conteudo_loop1;
    
    Loop* loops[] = {loop1, NULL};
    Fabricador* fabricador = criar_fabricador("arquivo", conteudo, loops, "./destino/");
    
    criar_arquivos(fabricador);
    liberar_fabricador(fabricador);
    free(loop1);
    
    return 0;
}
