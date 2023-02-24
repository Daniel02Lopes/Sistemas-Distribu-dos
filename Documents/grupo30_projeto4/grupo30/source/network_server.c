#include "network_server.h"
#include <stdio.h>
#include <stdlib.h>
#include "inet.h"
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "message-private.h"
#include "tree_skel.h"
#include <poll.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

// CONFIRMAR QUAL O VALOR CORRETO PARA NDFESC ???????

#define NFDESC 4 // Número de sockets (uma para listening)

// socker server number
int sockfd;

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */

int network_server_init(short port)
{
    struct sockaddr_in server;
    int option_value = 1;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro ao criar socket");
        return -1;
    }
   
    // Para ignorar sinais do tipo SIGPIPE, lançados quando uma das pontas comunicantes
    // fecha o socket de maneira inesperada
    signal(SIGPIPE, SIG_IGN);

    // Usar a função setsockopt(…, SO_REUSEADDR, …) para fazer com que o servidor
    // consiga fazer bind a um porto usado anteriormente e registado pelo kernel
    // como ainda ocupado.
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int)) < 0)
    {
        perror("Erro!");
        close(sockfd);
        return -1;
    }

    // Preenche estrutura server para bind
    server.sin_family = AF_INET;
    server.sin_port = htons(port); /*porta TCP */
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Faz bind
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    // Faz listen
    if (listen(sockfd, 0) < 0)
    {
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */

int network_main_loop(int listening_socket)
{

    struct pollfd connections[NFDESC]; // Estrutura para file descriptors das sockets das ligações
    struct sockaddr_in client;
    socklen_t size_client = sizeof(struct sockaddr);
    int kfds;
    int nfds = 1; // número de file descriptors

    // Para ignorar sinais do tipo SIGPIPE, lançados quando uma das pontas comunicantes
    // fecha o socket de maneira inesperada
    signal(SIGPIPE, SIG_IGN);

    if (listening_socket < 0)
    {
        return -1;
    }

    printf("----------------------------\n");
    printf("----BEM VINDO AO SERVIDOR---\n");
    printf("----------------------------\n");

    for (int i = 0; i < NFDESC; i++)
    {
        connections[i].fd = -1;
        connections[i].revents = 0;
    }

    connections[0].fd = sockfd;
    connections[0].events = POLLIN;

    while ((kfds = poll(connections, nfds, 10)) >= 0)
    { // kfds == 0 significa timeout sem eventos
        if (kfds > 0)
        { // número de descritores com evento ou erro
            if ((connections[0].revents & POLLIN) && (nfds < NFDESC))
            {
                if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *)&client, &size_client)) > 0)
                {
                    printf("CLIENTE CONECTOU-SE!\n");
                    connections[nfds].events = POLLIN;
                    nfds++;
                }
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (connections[i].revents & POLLIN)
            { // verifica se ha dados p ler

                struct message_t *message = network_receive(connections[i].fd);

                if (message == NULL)
                {

                    printf("CLIENTE DESCONECTOU-SE\n");
                    close(connections[i].fd);
                    connections[i].fd = -1;
                    nfds--;
                    continue;
                }

                invoke(message);

                int send = network_send(connections[i].fd, message);

                if (send == -1)
                {
                    close(connections[i].fd);
                    connections[i].fd = -1;
                    continue;
                }
            }

            if ((connections[i].revents & POLL_ERR) || (connections[i].revents & POLL_HUP))
            {
                close(connections[i].fd);
                connections[i].fd = -1;
                nfds--;
                continue;
            }
        }
    }

    close(listening_socket);
    return 0;
}
/*
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket)
{
    int nbytes = 0;
    if (client_socket < 0)
    {
        return NULL;
    }
    struct message_t *msg = create_message();

    int size_ntohl;
    int result = read(client_socket, &size_ntohl, sizeof(int));
    if (result == 0)
    {
        //message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }

    size_ntohl = ntohl(size_ntohl);

    uint8_t *buffer = malloc(size_ntohl);
    if (buffer == NULL)
    {
        //message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }

    if ((nbytes = read_all(client_socket, buffer, size_ntohl)) < 0)
    {
        perror("Erro ao receber dados do cliente");
        close(client_socket);
        //message_t__free_unpacked(&msg->message, NULL);
        free(buffer);
        return NULL;
    }

    MessageT *aux = message_t__unpack(NULL, size_ntohl, buffer);
    if (aux == NULL)
    {
        //message_t__free_unpacked(&msg->message, NULL);
        free(buffer);
        return NULL;
    }
    else
    {
        msg->message = *aux;
    }
    free(buffer);

    return msg;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t *msg)
{
    int nbytes;
    if (client_socket < 0 && msg == NULL)
    {
        return -1;
    }
    int packet_size = message_t__get_packed_size(&msg->message);
    int size_htonl = htonl(packet_size);

    uint8_t *buffer = malloc(packet_size);
    if (buffer == NULL)
    {
        printf("ERROR IN MALLOC\n");
        return -1;
    }
    message_t__pack(&msg->message, buffer);

    write(client_socket, &size_htonl, sizeof(int));
    nbytes = write_all(client_socket, buffer, packet_size);
    if (nbytes < 0)
    {
        perror("Erro ao enviar resposta ao cliente");
        close(client_socket);
        return -1;
    }
    free(buffer);
    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */

int network_server_close()
{
    return close(sockfd);
}