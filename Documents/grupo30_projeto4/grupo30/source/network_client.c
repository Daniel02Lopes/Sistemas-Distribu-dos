#include "client_stub.h"
#include <sys/socket.h>
#include <signal.h>
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "network_server.h"
#include "inet.h"
#include "sdmessage.pb-c.h"
#include "message-private.h"

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303rtree_connect
 */

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree)
{

    if ((rtree->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro a criar o socket");
        return -1;
    }
    if (connect(rtree->sockfd, (struct sockaddr *)&rtree->server, sizeof(struct sockaddr)) < 0)
    {
        perror("Erro ao connectar com o servidor");
        return -1;
    }
    signal(SIGPIPE, SIG_IGN);
    return 0;
}
/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtree_t *rtree, struct message_t *msg)
{

    if (network_send(rtree->sockfd, msg) == -1)
    {
        message_t__free_unpacked(&msg->message, NULL);
        printf("Erro ao enviar mensagem\n");
        return NULL;
    }
    msg = network_receive(rtree->sockfd);
    if (msg == NULL)
    {
        message_t__free_unpacked(&msg->message, NULL);
        printf("Erro ao receber mensagem\n");
        return NULL;
    }

    return msg;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t *rtree)
{
    return close(rtree->sockfd);
}