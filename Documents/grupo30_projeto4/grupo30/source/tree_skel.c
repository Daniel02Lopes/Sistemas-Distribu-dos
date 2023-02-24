#include "sdmessage.pb-c.h"
#include "message-private.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "tree.h"
#include "tree_skel.h"
#include <pthread.h>
#include "zookeeper/zookeeper.h"
#include <netdb.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

typedef struct String_vector zoo_string;
static zhandle_t *zh;
static int is_connected;
static const char *root_path = "/chain";
static char *watcher_ctx = "ZooKeeper Data Watcher";
static int is_connected;

struct rtree_t *next_server = NULL;

int id_node;
struct tree_t *tree;
int last_assigned;
int size_in_progress;
int result_put = 0;
int result_del = 0;
struct op_proc this_op;

pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
struct request_t *queue_head;

/**
 * Watcher function for connection state change events
 */
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void *context)
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
/**
 * Data Watcher function for /chain node
 */
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
    // int zoo_data_len = ZDATALEN;
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {

            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list))
            {
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }

            // ir buscar no a seguir ao atual (ou NULL)
            char *next_node = NULL;
            int next_id = -1;

            printf("=== znode listing === [ %s ]", root_path);

            char *path = "/chain/node";

            for (int i = 0; i < children_list->count; i++)
            {
                char *node = children_list->data[i];
                int id = atoi(strtok(node, path));

                // condicoes para obter next_id
                if (next_id == -1 || id < next_id)
                {
                    if (id > id_node)
                    {
                        next_id = id;
                        next_node = node;
                    }
                }
            }

            // nó seguinte
            char *final_next_node = next_node;

            // ligar ao proximo servidor
            if (next_server != NULL)
            {
                rtree_disconnect(next_server);
                next_server = NULL;
            }

            if (final_next_node != NULL)
            {
                char *node_path = NULL;
                strcat(node_path, root_path);
                strcat(node_path, "/");
                strcat(node_path, final_next_node);

                int data_size = ZDATALEN;
                char *node_data = malloc(data_size);

                if (ZOK != zoo_get(zh, node_path, 0, node_data, &data_size, NULL))
                {
                    printf("ERRO");
                    exit(-1);
                }

                next_server = rtree_connect(node_data);
                next_server->node_ = final_next_node;
            }
        }
    }
    free(children_list);
}
/*
* cria a arvore universal para head e tail
*/
int creat_universal_tree(){
    tree = tree_create();
    if (tree == NULL)
        return -1;
    return 0;
}
/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * A função deve lançar N threads secundárias responsáveis por atender
 * pedidos de escrita na árvore.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(char* port, char *host_name)
{
    char hostname_buffer[200];
    char *IP;
    struct hostent *hostent;
    int hostname = gethostname(hostname_buffer, sizeof(hostname_buffer));
    if (hostname == -1)
    {
        exit(-1);
    }
    hostent = gethostbyname(hostname_buffer);
    if (hostent == NULL)
    {

        exit(-1);
    }
    IP = inet_ntoa(*((struct in_addr *)hostent->h_addr_list[0]));

    char *next_id = NULL;
    int zoo_data_len = ZDATALEN;
    int N = 1;
    size_in_progress = N;
    last_assigned = 1;
    pthread_t threads[N];
    pthread_attr_t detached;
    this_op.in_progress = malloc(N * sizeof(int));
    queue_head = NULL;
    pthread_attr_init(&detached);
    pthread_attr_setdetachstate(&detached, PTHREAD_CREATE_DETACHED);
    zoo_string *children_list = NULL;
    zh = zookeeper_init(host_name, connection_watcher, 2000, 0, NULL, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
        return -1;
    }
    strcat(IP,":");
    strcat(IP,port);
    sleep(2);
    if (is_connected)
    {
        if (ZNONODE == zoo_exists(zh, root_path, 0, NULL))
        {
            printf("----------------------------------\n");
            fprintf(stderr, "%s doesn't exist!\n", root_path);
            printf("----------------------------------\n");
            if (ZOK == zoo_create(zh, root_path, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0))
            {
                printf("--------------------------------------\n");
                printf("Node Chain foi criado!\n");
                printf("--------------------------------------\n");
            }
            else
            {
                printf("--------------------------------------\n");
                printf("Erro ao criar o Node Chain\n");
                printf("--------------------------------------\n");
                exit(-1);
            }
        }
        else
        {
            printf("---------------------------------\n");
            fprintf(stderr, "THE NODE%s exist!\n", root_path);
            printf("---------------------------------\n");
        }
        char node_path[120] = "";
        strcat(node_path, root_path);
        strcat(node_path, "/node");
        int new_path_len = 1024;
        char *new_path = malloc(new_path_len);
        printf("-----IP:PORT->%s-----\n",IP);
        if (ZOK != zoo_create(zh, node_path, IP, strlen(IP) + 1, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len))
        {
            perror("Error creating server znode %s!\n");
            return -1;
        }
        printf("-----------------------------\n");
        fprintf(stderr, "Ephemeral Sequencial ZNode created! \n ZNode path: %s\n", new_path);
        children_list = (zoo_string *)malloc(sizeof(zoo_string));
        int retval = zoo_get_children(zh, root_path, 0, children_list);
        if (retval != ZOK)
        {
            fprintf(stderr, "Error retrieving znode from path %s!\n", root_path);
            exit(EXIT_FAILURE);
        }
        printf("------------------------------\n");
        fprintf(stderr, "=== znode listing === [ %s ]\n", root_path);
        next_id = malloc(ZDATALEN);
        char *current_node = children_list->data[0];
        char *heights_id_node = children_list->data[0];
        next_id = strtok(current_node, "node");
        for (int i = 0; i < children_list->count; i++)
        {
            current_node = children_list->data[i];
            char *current_id = strtok(current_node, "node");
            if (atoi(current_id) > atoi(next_id))
            {
                next_id = current_id;
                heights_id_node = current_node;
            }
        }
        fprintf(stderr, "=== done === \n");
        // ligar ao proximo servidor
        if (next_server != NULL)
        {
            rtree_disconnect(next_server);
            next_server = NULL;
        }

        if (heights_id_node != NULL)
        {
            char node_path [120];
            strcpy(node_path, root_path);
            strcat(node_path, "/");
            strcat(node_path, heights_id_node);
            char *node_data = malloc(zoo_data_len);
            if (ZOK != zoo_get(zh, node_path, 0, node_data, &zoo_data_len, NULL))
            {
                printf("ERRO");
                exit(-1);
            }
            next_server = rtree_connect_aux(node_data);
            next_server->node_ = heights_id_node;
        }

        for (int i = 0; i < N; i++)
        {
            this_op.in_progress[i] = 0;
            if (pthread_create(&threads[i], &detached, &process_request, NULL) != 0)
            {
                printf("\nThread %d não criada.\n", i);
                return -1;
            }
        }
    }
    return 0;
}

/* Função da thread secundária que vai processar pedidos de escrita.
 */
void *process_request(void *params)
{

    while (1)
    {

        pthread_mutex_lock(&queue_lock);

        while (queue_head == NULL)
        {
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }
        struct request_t *request = queue_head;
        pthread_mutex_unlock(&queue_lock);
        if (addToInProgess(request) == -1)
        {
            printf("Out of memory!");
            continue;
        }
        printfInProgress();
        switch (request->op)
        {
        case 0: // DEL
            if (execute_del_op(request) == -1)
            {
                result_del = -1;
                printf("Erro ao executar DEL da OP:%d\n", request->op_n);
            }
            else
            {
                result_del = 1;
                printf("A OP:%d foi executada com sucesso!\n", request->op_n);
                printf("------------------------------------------\n");
            }
            break;
        case 1: // PUT
            if (execute_put_op(request) == -1)
            {
                result_put = -1;
                printf("Erro ao executar PUT da OP:%d\n", request->op_n);
            }
            else
            {
                result_put = 1;
                printf("A OP:%d foi executada com sucesso!\n", request->op_n);
                printf("------------------------------------------\n");
            }
            break;
        }
        if (request->next_request == NULL)
        {
            queue_head = NULL;
        }
        else
        {
            queue_head = queue_head->next_request;
        }
        if (removeToInProgess(request) == -1)
        {
            printf("ERROR TO REMOVE FROM INPROGESS");
            continue;
        }
        printfInProgress();
        if (destroy_request(request) == -1)
        {
            printf("Erro ao destruir a request!\n");
        }
        // sleep(30); // testing the threads for multiples request
    }
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy()
{

    pthread_exit(0);
    pthread_mutex_destroy(&tree_lock);
    pthread_mutex_destroy(&queue_lock);
    pthread_cond_destroy(&queue_not_empty);
    free(this_op.in_progress);
    tree_destroy(tree);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
 */
int invoke(struct message_t *msg)
{
    struct data_t *newdata;
    switch (msg->message.opcode)
    {
    case MESSAGE_T__OPCODE__OP_SIZE:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        else if (tree_size(tree) != -1)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            msg->message.value = tree_size(tree);
            printf("-->OP:SIZE\n");
            return 0;
        }
        else
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        break;
    case MESSAGE_T__OPCODE__OP_HEIGHT:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        else if (tree_height(tree) != -1)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
            msg->message.value = tree_height(tree);
            printf("-->OP:HEIGHT\n");
            return 0;
        }
        else
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        break;
    case MESSAGE_T__OPCODE__OP_DEL:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_KEY)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        else
        {
            pthread_mutex_lock(&queue_lock);
            printf("------------------------------------------\n");
            printf("-REQUEST FOR DELETE WITH OP:%d IS IN QUEUE-\n", last_assigned);
            printf("------------------------------------------\n");
            putInQueue(last_assigned, 0, msg->message.entry->key, NULL);
            pthread_cond_signal(&queue_not_empty);
            pthread_mutex_unlock(&queue_lock);
            while (result_del == 0)
            {
                // wait from response of thread
            }
            if (result_del == 1)
            {
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->message.opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
                msg->message.value = last_assigned;
                last_assigned++;
                result_del = 0;
                return 0;
            }
            else
            {

                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                result_del = 0;
                return -1;
            }
        }
        break;
    case MESSAGE_T__OPCODE__OP_GET:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_KEY)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        else
        {
            newdata = tree_get(tree, msg->message.entry->key);
            if (newdata != NULL)
            {
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->message.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
                msg->message.entry->data->data = newdata->data;
                msg->message.entry->data->datasize = newdata->datasize;
                printf("-->OP:GET\n");
                printf("-->KEY:%s\n", msg->message.entry->key);
                return 0;
            }
            else
            {
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                return -1;
            }
        }
        break;
    case MESSAGE_T__OPCODE__OP_PUT:

        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_ENTRY)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        else
        {
            newdata = data_create2(msg->message.entry->data->datasize, msg->message.entry->data->data);
            if (newdata != NULL)
            {
                pthread_mutex_lock(&queue_lock);
                printf("------------------------------------------\n");
                printf("-REQUEST FOR PUT WITH OP:%d IS IN QUEUE-\n", last_assigned);
                printf("------------------------------------------\n");
                putInQueue(last_assigned, 1, msg->message.entry->key, newdata);
                pthread_cond_signal(&queue_not_empty);
                pthread_mutex_unlock(&queue_lock);
                while (result_put == 0)
                {
                    // wait for response from thread
                }
                if (result_put == 1)
                {
                    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    msg->message.opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
                    msg->message.value = last_assigned;
                    last_assigned++;
                    result_put = 0;
                    return 0;
                }
                else
                {
                    msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                    msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                    result_put = 0;
                    return -1;
                }
            }
            else
            {
                msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
                msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
                return -1;
            }
        }
        break;
    case MESSAGE_T__OPCODE__OP_GETKEYS:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_NONE || tree_size(tree) < 1)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        char **keys_to_add = tree_get_keys(tree);
        if (keys_to_add != NULL)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            msg->message.n_keys_values = tree_size(tree);
            msg->message.keys_values = keys_to_add;
            printf("-->OP:GETKEYS\n");
            return 0;
        }
        else
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }

        break;
    case MESSAGE_T__OPCODE__OP_GETVALUES:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_NONE || tree_size(tree) < 1)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        void **values = tree_get_values(tree);
        if (values != NULL)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_VALUES;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
            msg->message.n_keys_values = tree_size(tree);
            msg->message.keys_values = (char **)values;

            printf("-->OP:GETVALUES\n");
            return 0;
        }
        else
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        break;
    case MESSAGE_T__OPCODE__OP_VERIFY:
        if (msg->message.c_type != MESSAGE_T__C_TYPE__CT_RESULT)
        {
            msg->message.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            msg->message.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            return -1;
        }
        msg->message.opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
        msg->message.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        printf("-->OP:VERIFY %d\n", msg->message.value);
        msg->message.value = verify(msg->message.value);
        return 0;

        break;

    case MESSAGE_T__OPCODE__OP_ERROR:
        return -1;
        break;
    default:
        return -1;
        break;
    }
    return -1;
}

/**
 * Função que vai criar a request e colocar dentro da Queue
 */
int putInQueue(int op_n, int op, char *key, struct data_t *data)
{
    struct request_t *request_to_return = NULL;
    if (queue_head == NULL)
    {
        request_to_return = creat_request(op_n, op, key, data);
        queue_head = request_to_return;
    }
    else
    {
        struct request_t *aux = queue_head;
        while (aux->next_request != NULL)
        {
            aux = aux->next_request;
        }

        request_to_return = creat_request(op_n, op, key, data);
        aux->next_request = request_to_return;
        return 0;
    }
    return -1;
}

/**
 * Função executar por uma thread a request que tem como operação DELETE
 */
int execute_del_op(struct request_t *request)
{
    pthread_mutex_lock(&tree_lock);
    int aux = tree_del(tree, request->key);
    pthread_mutex_unlock(&tree_lock);
    if (aux == 0)
    {
        printf("REQUEST WITH OP:%d DONE!\n", request->op_n);
        printf("-->OP:DEL\n");
        printf("-->KEY:%s\n", request->key);
        return 0;
    }

    return -1;
}

/**
 * Função executar por uma thread a request que tem como operação PUT
 */
int execute_put_op(struct request_t *request)
{
    pthread_mutex_lock(&tree_lock);
    int aux = tree_put(tree, request->key, request->data);
    pthread_mutex_unlock(&tree_lock);
    if (aux == 0)
    {
        printf("REQUEST WITH OP:%d DONE!\n", request->op_n);
        printf("-->OP:PUT\n");
        printf("-->KEY:%s\n", request->key);
        printf("-->VALUE:%s", (char *)request->data);
        printf("-->DATASIZE:%d\n", request->data->datasize);
        return 0;
    }
    return -1;
}

/*
 * Verifica se a operação identificada por op_n foi executada.
 * retorna: 1 caso for executada
 *          0 caso esteja em execução
 *          -1 caso não tenha sido criada
 */
int verify(int op_n)
{
    for (int i = 0; i < (size_in_progress); i++)
    {
        if (this_op.in_progress[i] == op_n)
        {
            return 0; // in progress
        }
    }
    if (op_n >= last_assigned)
        return -1; // Nr op ainda nem foi criada
    return 1;      // executed
}

/**
 * Print do array de op_n de requests
 */
void printfInProgress()
{
    printf("----------------IN PROGRESS---------------\n");
    for (int i = 0; i < (size_in_progress); i++)
    {
        printf("->OP:%d\n", this_op.in_progress[i]);
    }
    printf("------------------------------------------\n");
}

/**
 * Criar uma request
 */
struct request_t *creat_request(int op_n, int op, char *key, struct data_t *data)
{
    struct request_t *request = (struct request_t *)malloc(sizeof(struct request_t));
    if (request == NULL)
    {
        printf("ERRO AO FAZER MALLOC DA REQUEST\n");
        return NULL;
    }
    request->op_n = op_n;
    request->op = op;
    request->key = key;
    request->data = data;
    request->next_request = NULL;
    return request;
}
/**
 * Destroi uma dada request
 */
int destroy_request(struct request_t *request)
{
    if (request != NULL)
    {
        free(request);
        return 0;
    }
    return -1;
}

/**
 * Adiciona o op_n de uma dada request ao array In_progess
 */
int addToInProgess(struct request_t *request)
{
    for (int i = 0; i < size_in_progress; i++)
    {
        if (this_op.in_progress[i] == 0)
        {
            this_op.in_progress[i] = request->op_n;
            return 0;
        }
    }
    printf("OUT OF MEMORY!\n");
    return -1;
}

/**
 * Remove o op_n de uma dada request ao array In_progess
 */
int removeToInProgess(struct request_t *request)
{
    for (int i = 0; i < size_in_progress; i++)
    {

        if (this_op.in_progress[i] == request->op_n)
        {
            if (this_op.in_progress[i] > this_op.max_proc)
            {
                this_op.max_proc = this_op.in_progress[i];
            }

            this_op.in_progress[i] = 0;
            return 0;
        }
    }

    printf("REQUEST NOT FOUND!\n");
    return -1;
}
