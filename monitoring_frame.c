#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// Estrutura para armazenar informações de conexão com o ClienteMestre
typedef struct {
    int socket_fd;
    struct sockaddr_in servidor_addr;
} ClienteMestre;

// Variáveis globais da interface
GtkWidget *lbNumeroNucleos;
ClienteMestre clienteMestre;

// Função para enviar mensagens ao ClienteMestre
void enviar_mensagem(const char *msg) {
    send(clienteMestre.socket_fd, msg, strlen(msg), 0);
}

// Função para receber mensagens do ClienteMestre
void* receber_mensagem(void* arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(clienteMestre.socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("Status recebido do ClienteMestre: %s\n", buffer);
        }
        sleep(1);
    }
    return NULL;
}

// Função chamada quando o botão "Start" é pressionado
void on_start_button_clicked(GtkButton *button, gpointer data) {
    enviar_mensagem("Start");
    g_print("Comando 'Start' enviado.\n");
}

// Função chamada quando o botão "Fechar Todos" é pressionado
void on_close_button_clicked(GtkButton *button, gpointer data) {
    enviar_mensagem("Close");
    g_print("Comando 'Close' enviado.\n");
}

// Função chamada quando o botão "Carregar" é pressionado
void on_load_button_clicked(GtkButton *button, gpointer data) {
    enviar_mensagem("Load");
    g_print("Comando 'Load' enviado.\n");
}

// Função para inicializar e mostrar a GUI
void activate_monitoring_frame(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *btnStart;
    GtkWidget *btnClose;
    GtkWidget *btnCarregar;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Monitoring Frame");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    label = gtk_label_new("Número de Núcleos Usados:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    lbNumeroNucleos = gtk_label_new("0");
    gtk_grid_attach(GTK_GRID(grid), lbNumeroNucleos, 1, 0, 1, 1);

    btnStart = gtk_button_new_with_label("Start");
    g_signal_connect(btnStart, "clicked", G_CALLBACK(on_start_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), btnStart, 2, 0, 1, 1);

    btnCarregar = gtk_button_new_with_label("Carregar");
    g_signal_connect(btnCarregar, "clicked", G_CALLBACK(on_load_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), btnCarregar, 3, 0, 1, 1);

    btnClose = gtk_button_new_with_label("Fechar Todos");
    g_signal_connect(btnClose, "clicked", G_CALLBACK(on_close_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), btnClose, 0, 2, 4, 1);

    gtk_widget_show_all(window);
}

// Função para iniciar a interface `monitoring_frame`
void iniciar_monitoring_frame(char *endereco, int porta, int quantidade_divisoes) {
    GtkApplication *app;
    int status;

    // Configura conexão com o ClienteMestre
    clienteMestre.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    clienteMestre.servidor_addr.sin_family = AF_INET;
    clienteMestre.servidor_addr.sin_port = htons(porta);
    clienteMestre.servidor_addr.sin_addr.s_addr = inet_addr(endereco);

    if (connect(clienteMestre.socket_fd, (struct sockaddr*)&clienteMestre.servidor_addr, sizeof(clienteMestre.servidor_addr)) < 0) {
        perror("Erro ao conectar ao ClienteMestre");
        return;
    }
    printf("Conectado ao ClienteMestre no endereço %s:%d\n", endereco, porta);

    // Cria uma thread para receber mensagens do ClienteMestre
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receber_mensagem, NULL);
    pthread_detach(thread_id);

    // Inicializa o GTK
    app = gtk_application_new("com.example.monitoring", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate_monitoring_frame), NULL);

    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    close(clienteMestre.socket_fd); // Fecha o socket ao final
}
