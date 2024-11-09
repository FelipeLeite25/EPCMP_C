#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_NUCLEOS 16
#define BUFFER_SIZE 1024

GtkWidget *tfCaminhoArquivo;
GtkWidget *cbQuantNucleos;
int PORTA = 1010;

// Funções auxiliares
void executar_comando(const char *cmd[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd[0], (char * const *)cmd);
        perror("Erro ao executar o comando");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}

// Função para escolher um arquivo
void on_btnEscolherArquivo_clicked(GtkButton *button, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Escolher Arquivo",
                                                    NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancelar", GTK_RESPONSE_CANCEL,
                                                    "_Abrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.cpp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(tfCaminhoArquivo), filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Função para iniciar a operação
void on_btnIniciar_clicked(GtkButton *button, gpointer data) {
    const char *caminhoArquivo = gtk_entry_get_text(GTK_ENTRY(tfCaminhoArquivo));
    int quantidadeNucleos = atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbQuantNucleos)));
    
    if (strlen(caminhoArquivo) == 0) {
        printf("Erro: Nenhum arquivo selecionado.\n");
        return;
    }

    // Comando para executar o "Clite" e gerar o arquivo .jas
    char cmd_rodar_clite[BUFFER_SIZE];
    snprintf(cmd_rodar_clite, BUFFER_SIZE, "java CodeGen %s", caminhoArquivo);
    const char *cmd[] = {"sh", "-c", cmd_rodar_clite, NULL};
    executar_comando(cmd);

    // Nome do arquivo .jas gerado
    char caminhoArquivoJas[BUFFER_SIZE];
    snprintf(caminhoArquivoJas, BUFFER_SIZE, "%s.jas", caminhoArquivo);
    printf("Arquivo .jas gerado: %s\n", caminhoArquivoJas);

    // Processamento dos arquivos
    int quantidadeArquivosGerados = dividir_arquivo(caminhoArquivoJas, quantidadeNucleos);

    // Configuração dos processadores simulados
    for (int i = 0; i < quantidadeArquivosGerados; i++) {
        char caminhoArquivoProcessador[BUFFER_SIZE];
        snprintf(caminhoArquivoProcessador, BUFFER_SIZE, "%s%d.jas", caminhoArquivoJas, i);
        printf("Executando Processador %d com arquivo %s\n", i, caminhoArquivoProcessador);
        // Aqui inicia cada processador
    }

    // Executar o EPCMP
    char comandoEPCMP[BUFFER_SIZE];
    snprintf(comandoEPCMP, BUFFER_SIZE, "java -jar EPCMP-1.0.jar parte %d %d", quantidadeArquivosGerados, PORTA);
    const char *cmdEPCMP[] = {"sh", "-c", comandoEPCMP, NULL};
    executar_comando(cmdEPCMP);

    gtk_main_quit();
}

// Função para dividir o arquivo .jas em múltiplos
int dividir_arquivo(const char *caminhoArquivoJas, int quantidadeNucleos) {
    FILE *file = fopen(caminhoArquivoJas, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    // Contar linhas
    int linhas = 0;
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file)) {
        linhas++;
    }
    rewind(file);

    // Ler conteúdo do arquivo
    char **conteudo = malloc(linhas * sizeof(char*));
    for (int i = 0; i < linhas; i++) {
        conteudo[i] = malloc(BUFFER_SIZE);
        fgets(conteudo[i], BUFFER_SIZE, file);
    }
    fclose(file);

    int quantidadeDivisoes = quantidadeNucleos;

    // Libera memória
    for (int i = 0; i < linhas; i++) {
        free(conteudo[i]);
    }
    free(conteudo);

    return quantidadeDivisoes;
}

// Função para inicializar a interface do start_main
void iniciar_start_main() {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.start_main", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_btnEscolherArquivo_clicked), NULL);

    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
}
