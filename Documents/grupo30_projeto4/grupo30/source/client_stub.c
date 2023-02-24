#include <string.h>
#include <stdlib.h>

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include <signal.h>
#include "client_stub-private.h"
#include "network_client.h"
#include "message-private.h"
#include <zookeeper/zookeeper.h>

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

typedef struct String_vector zoo_string;
static char *watcher_ctx = "ZooKeeper Data Watcher";
static char *root_path = "/chain";
static zhandle_t *zh;
int closed = 0;
static struct rtree_t *rtree_head;
static struct rtree_t *rtree_tail;
static struct rtree_t *rtree;
static int is_connected;
static int connected;
/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

/**
 * Watcher function for connection state change events
 */
void connection_watcher_client(zhandle_t *zzh, int type, int state, const char *path, void *context)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            is_connected = 1;
        }
        else
        {
            is_connected = 0;
        }
    }
}

void child_watcher_client(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    char *lowerId;
    char *hieghtsId;
    char *address_port_tail = NULL;
    char *address_port_head = NULL;
    char lower_rooth_path [120];
    char heights_rooth_path [120];
    zoo_string *children_list = NULL;
    int zoo_data_len = ZDATALEN;
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            children_list = (zoo_string *)malloc(sizeof(zoo_string));
            if (ZOK != zoo_wget_children(zh, root_path, child_watcher_client, watcher_ctx, children_list))
            {
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }
            hieghtsId = children_list->data[0];
            lowerId = children_list->data[0];
            fprintf(stderr, "\n=== znode listing === [ %s ]", root_path);
            hieghtsId = children_list->data[0];
            hieghtsId = strtok(hieghtsId, "node");
            lowerId = children_list->data[0];
            lowerId = strtok(lowerId, "node");
            char *node = NULL;
            for (int i = 0; i < children_list->count; i++)
            {
                node = children_list->data[i];
                char *id = strtok(node, "node");
                if (id < lowerId)
                    lowerId = node;
                if (id > hieghtsId)
                    hieghtsId = node;
            }
            fprintf(stderr, "\n=== done ===\n");

            //---------------------------HEAD--------------------------------
            address_port_head = malloc(ZDATALEN * sizeof(char));
            if (address_port_head == NULL)
            {
                exit(-1);
            }
            strcpy(lower_rooth_path, root_path);
            strcat(lower_rooth_path, "/");
            strcat(lower_rooth_path, lowerId);
            if (zoo_get(zh, lower_rooth_path, 0, address_port_head, &zoo_data_len, NULL) != ZOK)
            {
                exit(-1);
            }
            if (rtree_head == NULL)
            {
                rtree_head = rtree_connect_aux(address_port_head);
            }
            else
            {
                if (strcmp(address_port_head, rtree_head->address_port) != 0)
                {
                    rtree_disconnect(rtree_head);
                    rtree_head = rtree_connect_aux(address_port_head);
                }
            }
            if (rtree_head == NULL)
            {
                exit(-1);
            }
            //---------------------------TAIL---------------------------------
            address_port_tail = malloc(zoo_data_len);
            if (address_port_tail == NULL)
            {
                exit(-1);
            }
            strcpy(heights_rooth_path, root_path);
            strcat(heights_rooth_path, "/");
            strcpy(heights_rooth_path, hieghtsId);
            if (zoo_get(zh, heights_rooth_path, 0, address_port_tail, &zoo_data_len, NULL) != ZOK)
            {
                exit(-1);
            }
            if (rtree_tail == NULL)
            {
                rtree_tail = rtree_connect_aux(address_port_tail);
            }
            else
            {
                if (strcmp(address_port_tail, rtree_tail->address_port) != 0)
                {
                    rtree_disconnect(rtree_tail);
                    rtree_tail = rtree_connect_aux(address_port_tail);
                }
            }
            if (rtree_tail == NULL)
            {
                exit(-1);
            }
        }
    }
}

struct rtree_t *rtree_connect(const char *address_port)
{
    char *lowerId;
    char *hieghtsId;
    char lower_rooth_path[120];
    char heights_rooth_path[120];
    zoo_string *children_list = NULL;
    char *address_port_head = NULL;
    char *address_port_tail = NULL;
    int zoo_data_len = ZDATALEN;
    if (address_port == NULL)
    {
        return NULL;
    }
    zh = zookeeper_init(address_port, connection_watcher_client, 2000, 0, 0, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error connecting to ZooKeeper server[%d]!\n", errno);
        return NULL;
    }
    char *ip = strtok((char *)address_port, ":"); //<hostname>
    char *port = strtok(NULL, ":");               //<port>
    if (ip == NULL || port == NULL)
        return NULL;
    sleep(3);
    rtree = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    if (is_connected)
    {
        if (ZNONODE == zoo_exists(zh, root_path, 0, NULL))
        {
            fprintf(stderr, "Error %s!\n", root_path);
            return NULL;
        }
        children_list = (zoo_string *)malloc(sizeof(zoo_string));
        printf("--------------------------------------\n");
        fprintf(stderr, "\n=== znode listing === [ %s ]", root_path);
        if (ZOK != zoo_wget_children(zh, root_path, &child_watcher_client, watcher_ctx, children_list))
        {
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
        }
        if (children_list->count < 1)
        {
            printf("NENHUM SERVIDOR DISPONIVEL\n");
            return NULL;
        }

        hieghtsId = children_list->data[0];
        hieghtsId = strtok(hieghtsId, "node");
        lowerId = children_list->data[0];
        lowerId = strtok(lowerId, "node");
        char *node = NULL;
        char *id = NULL;
        printf("--------------------------------------\n");
        printf("Number of nodes:%d\n", children_list->count);
        printf("--------------------------------------\n");
        for (int i = 0; i < children_list->count; i++)
        {
            node = children_list->data[i];
            printf("Node:%s\n", node);
            id = strtok(node, "node");
            if (atoi(id) < atoi(lowerId))
                lowerId = id;
            if (atoi(id) > atoi(hieghtsId))
                hieghtsId = id;
        }
        fprintf(stderr, "\n=== done ===\n");
        printf("--------------------------------------\n");
        printf("Lower ID:%s\n", lowerId);
        printf("Heighst ID:%s\n", hieghtsId);
        printf("--------------------------------------\n");
        //---------------------------HEAD--------------------------------
        address_port_head = malloc(ZDATALEN * sizeof(char));
        if (address_port_head == NULL)
        {
            return NULL;
        }
        strcpy(lower_rooth_path, root_path);
        strcat(lower_rooth_path, "/node");
        strcat(lower_rooth_path, lowerId);
        if (zoo_get(zh, lower_rooth_path, 0, address_port_head, &zoo_data_len, NULL) != ZOK)
        {
            printf("erro\n");
            return NULL;
            ;
        }
        if (address_port_head == NULL)
        {
            return NULL;
        }
        printf("--------------------------------------\n");
        printf("ADDRESS_PORT_HEAD:%s\n", address_port_head);
        if (rtree_head == NULL)
        {
            rtree_head = rtree_connect_aux(address_port_head);
            printf("SERVIDOR HEAD ESTÁ DISPONIVEL!\n");
            printf("--------------------------------------\n");
        }
        else
        {
            if (strcmp(address_port_head, rtree_head->address_port) != 0)
            {
                rtree_disconnect(rtree_head);
                rtree_head = rtree_connect_aux(address_port_head);
                printf("SERVIDOR HEAD ESTÁ DISPONIVEL!\n");
                printf("--------------------------------------\n");
            }
        }
        if (rtree_head == NULL)
        {
            return NULL;
        } //---------------------------TAIL---------------------------------
        if (children_list->count > 1)
        {
            address_port_tail = malloc(zoo_data_len);
            if (address_port_tail == NULL)
            {
                return NULL;
            }
            strcpy(heights_rooth_path, root_path);
            strcat(heights_rooth_path, "/node");
            strcat(heights_rooth_path, hieghtsId);
            if (zoo_get(zh, heights_rooth_path, 0, address_port_tail, &zoo_data_len, NULL) != ZOK)
            {
                return NULL;
            }
            if (address_port_tail == NULL)
            {
                return NULL;
            }
            printf("--------------------------------------\n");
            printf("address_port_tail:%s\n", address_port_tail);
            if (rtree_tail == NULL)
            {
                rtree_tail = rtree_connect_aux(address_port_tail);
                printf("SERVIDOR TAIL ESTÁ DISPONIVEL!\n");
                printf("--------------------------------------\n");
            }
            else
            {
                if (strcmp(address_port_tail, rtree_tail->address_port) != 0)
                {
                    rtree_disconnect(rtree_tail);
                    rtree_tail = rtree_connect_aux(address_port_tail);
                    printf("SERVIDOR TAIL ESTÁ DISPONIVEL!\n");
                    printf("--------------------------------------\n");
                }
            }
            if (rtree_tail == NULL)
            {
                return NULL;
            }
        }
    }
    free(children_list);
    free(address_port_head);
    free(address_port_tail);
    signal(SIGPIPE, SIG_IGN);
    connected = 1;
    return rtree_head;
}
/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect_aux(const char *address_port)
{
    if (address_port == NULL)
    {
        return NULL;
    }
    struct rtree_t *rtree = (struct rtree_t *)malloc(sizeof(struct rtree_t));
    if (rtree == NULL)
    {
        free(rtree);
        return NULL;
    }
    char *hostname = strtok((char *)address_port, ":"); //<hostname>
    int port = atoi(strtok(NULL, ":"));                 //<port>
    rtree->server.sin_family = AF_INET;                 // família de endereços
    rtree->server.sin_port = htons(port);               // Porta TCP
    rtree->address_port = strdup((char *)address_port);
    if (inet_pton(AF_INET, hostname, &rtree->server.sin_addr) < 1)
    { // Endereço IP
        printf("Erro ao converter IP\n");
        free(rtree);
        return NULL;
    }

    if (network_connect(rtree) == -1)
    {
        if (network_close(rtree) == -1)
        {
            printf("Erro ao terminar a associação entre o cliente e o servidor\n");
        }
        perror("Erro ao conectar-se ao servidor");
        free(rtree);
        return NULL;
    }
    signal(SIGPIPE, SIG_IGN);
    return rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree)
{
    if (rtree == NULL)
    {
        return -1;
    }
    if (network_close(rtree) == -1)
    {
        return -1;
    }
    free(rtree);
    return 0;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry)
{

    struct message_t *msg = create_message();

    if (msg == NULL)
    {
        return -1;
    }
    msg->message.entry->data->datasize = entry->value->datasize;
    msg->message.entry->data->data = entry->value->data;
    msg->message.entry->key = entry->key;
    msg->message.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    msg = network_send_receive(rtree, msg);

    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (put,entry) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }
    printf("OPERAÇÃO DE ESCRITA OP:%d\n", msg->message.value);
    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_PUT + 1)
    {
        message_t__free_unpacked(&msg->message, NULL);
        return 0;
    }
    printf("Erro ao enviar a mensagem (put,entry) para o servidor\n");
    message_t__free_unpacked(&msg->message, NULL);
    return -1;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key)
{
    struct message_t *msg = create_message();
    if (msg == NULL)
    {
        return NULL;
    }
    msg->message.entry->key = key;
    msg->message.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg = network_send_receive(rtree, msg);
    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (get,key) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }
    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_GET + 1)
    {
        int size = msg->message.entry->data->datasize;
        void *data = msg->message.entry->data->data;
        struct data_t *data_to_return = data_create2(size, data);

        void *value = msg->message.entry->data->data;
        printf("---KEY ENCONTRADA----\n");
        printf("-->KEY:%s -----------\n", msg->message.entry->key);
        printf("-->DATASIZE:%d -------\n", msg->message.entry->data->datasize);
        printf("-->DATA:%s -----------\n", strtok(value, "\n"));
        printf("-----------------------\n");
        message_t__free_unpacked(&msg->message, NULL);
        return data_to_return;
    }
    printf("Erro ao enviar a mensagem (get,key) para o servidor\n");
    message_t__free_unpacked(&msg->message, NULL);
    return NULL;
}

/* Função para remover um elemento da árvore. Vai libertar
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key)
{
    struct message_t *msg = create_message();
    if (msg == NULL)
    {
        return -1;
    }
    msg->message.entry->key = key;
    msg->message.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg = network_send_receive(rtree, msg);
    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (del,key) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }
    printf("OPERAÇÃO DE ESCRITA OP:%d\n", msg->message.value);
    if (msg->message.opcode != MESSAGE_T__OPCODE__OP_DEL + 1)
    {
        printf("Erro ao enviar a mensagem (del,key) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }
    message_t__free_unpacked(&msg->message, NULL);
    return 0;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree)
{
    struct message_t *msg = create_message();
    int size = 0;
    if (msg == NULL)
    {
        return -1;
    }
    msg->message.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg = network_send_receive(rtree, msg);
    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (size) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }
    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_SIZE + 1)
    {
        size = msg->message.value;
        return size;
    }
    printf("Erro ao enviar a mensagem (size) para o servidor\n");
    message_t__free_unpacked(&msg->message, NULL);
    return -1;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree)
{
    struct message_t *msg = create_message();
    int height = 0;
    if (msg == NULL)
    {
        return -1;
    }
    msg->message.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg = network_send_receive(rtree, msg);
    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (height) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }
    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_HEIGHT + 1)
    {
        height = msg->message.value;
        message_t__free_unpacked(&msg->message, NULL);
        return height;
    }
    printf("Erro ao enviar a mensagem (height) para o servidor\n");
    message_t__free_unpacked(&msg->message, NULL);
    return -1;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree)
{
    struct message_t *msg = create_message();
    if (msg == NULL)
    {
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }
    msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->message.n_keys_values = 0;
    msg->message.keys_values = NULL;
    msg = network_send_receive(rtree, msg);
    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (getkeys) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }

    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_GETKEYS + 1)
    {
        int size_of_buffer = msg->message.n_keys_values + 1;
        char **keys = malloc(size_of_buffer * sizeof(char *) + 1);
        for (int i = 0; i < size_of_buffer - 1; i++)
        {
            char *key_to_add = malloc(strlen(msg->message.keys_values[i]) + 1);
            strcpy(key_to_add, msg->message.keys_values[i]);
            keys[i] = key_to_add;
        }
        keys[size_of_buffer - 1] = NULL; // ultimo elemento fica a null
        message_t__free_unpacked(&msg->message, NULL);
        return keys;
    }
    printf("Erro ao enviar a mensagem (getkeys) para o servidor\n");
    message_t__free_unpacked(&msg->message, NULL);
    return NULL;
}
/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree)
{

    struct message_t *msg = create_message();
    if (msg == NULL)
    {
        return NULL;
    }
    msg->message.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->message.n_keys_values = 0;
    msg->message.keys_values = NULL;
    msg = network_send_receive(rtree, msg);
    if (msg == NULL)
    {
        printf("Erro ao enviar a mensagem (getvalues) para o servidor\n");
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }
    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_GETVALUES + 1)
    {
        void *value = NULL;
        int size_of_buffer = msg->message.n_keys_values + 1;
        void **values = malloc(size_of_buffer * sizeof(void *) + 1);
        printf("-------VALUES----------\n");
        for (int i = 0; i < size_of_buffer - 1; i++)
        {
            int size = strlen((char *)msg->message.keys_values[i]) + 1;
            values[i] = malloc(size);
            memcpy(values[i], (char *)msg->message.keys_values[i], size);
            value = values[i];
            printf("-->VALUE[%2i]:%s\n", i + 1, strtok(value, "\n"));
        }
        values[size_of_buffer - 1] = NULL; // ultimo elemento fica a null
        message_t__free_unpacked(&msg->message, NULL);
        return values;
    }
    printf("Erro ao enviar a mensagem (getvalues) para o servidor\n");
    message_t__free_unpacked(&msg->message, NULL);
    return NULL;
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int rtree_verify(struct rtree_t *rtree, int op_n)
{
    struct message_t *msg = create_message();
    if (msg == NULL)
    {
        message_t__free_unpacked(&msg->message, NULL);
        return -1;
    }
    int toReturn = 0;
    msg->message.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->message.value = op_n;

    msg = network_send_receive(rtree, msg);

    if (msg->message.opcode == MESSAGE_T__OPCODE__OP_VERIFY + 1)
    {
        toReturn = msg->message.value;
        message_t__free_unpacked(&msg->message, NULL);
        return toReturn;
    }
    message_t__free_unpacked(&msg->message, NULL);
    return -1;
}

/**
 * zk disconect
*/
int zk_disconnect()
{
    if (rtree_disconnect(rtree_head) == 0)
    {
        return -1;
    }
    if (rtree_tail != NULL)
    {

        if (rtree_disconnect(rtree_tail) == 0)
        {
            return -1;
        }
    }
    zookeeper_close(zh);
    return 0;
}

/**
 * funcao auxiliar para put no rtree_head
*/
int zk_put(struct entry_t *entry)
{
    if (rtree_tail != NULL)
    {
        rtree_put(rtree_tail, entry);
    }
    return rtree_put(rtree_head, entry);
}

/**
 * funcao auxiliar para get no rtree_tail
*/
struct data_t *zk_get(char *key)
{
    if (rtree_tail != NULL)
        return rtree_get(rtree_tail, key);
    else
    {
        return rtree_get(rtree_head, key);
    }
}

/**
 * funcao auxiliar para del no rtree_head
*/
int zk_del(char *key)
{
    if (rtree_tail != NULL)
    {
        rtree_del(rtree_tail, key);
    }
    return rtree_del(rtree_head, key);
}
/**
 * funcao auxiliar para size no rtree_tail
*/
int zk_size()
{
    if (rtree_tail != NULL)
        return rtree_size(rtree_tail);
    else
        return rtree_size(rtree_head);
}

/**
 * funcao auxiliar para height no rtree_tail
*/
int zk_height()
{
    if (rtree_tail != NULL)
        return rtree_height(rtree_tail);
    else
        return rtree_height(rtree_head);
}

/**
 * funcao auxiliar para getkeys no rtree_tail
*/
char **zk_getKeys()
{
    if (rtree_tail != NULL)
        return rtree_get_keys(rtree_tail);
    else
        return rtree_get_keys(rtree_head);
}
/**
 * funcao auxiliar para getvalues no rtree_tail
*/
void **zk_getValues()
{
    if (rtree_tail != NULL)
        return rtree_get_values(rtree_tail);
    else
    {
        return rtree_get_values(rtree_head);
    }
}

/**
 * funcao auxiliar para verify no rtree_tail
*/
int zk_verify(int op_n)
{
    if (rtree_tail != NULL)
    {
        return rtree_verify(rtree_tail, op_n);
    }
    else
    {
        return rtree_verify(rtree_head, op_n);
    }
}
