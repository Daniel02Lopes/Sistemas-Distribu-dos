#ifndef _TREE_SKEL_H
#define _TREE_SKEL_H

#include "sdmessage.pb-c.h"
#include "tree.h"
#include "message-private.h"


struct op_proc
{
    int max_proc;
    int *in_progress;
};


struct request_t
{
    int op_n;            // o número da operação
    int op;              // a operação a executar. op=0 se for um delete, op=1 se for um put
    char *key;           // a chave a remover ou adicionar
    struct data_t *data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    struct request_t * next_request;// adicionar campo(s) necessário(s) para implementar fila do tipo produtor/consumidor
};
int creat_universal_tree();
/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke().
* A função deve lanchar* hostname threads secundárias responsáveis por atender
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(char* port, char *host_name);

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy();

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void * process_request (void *params);

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct message_t *msg);


/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n);

/**
 * Função que vai criar a request e colocar dentro da Queue
*/
int putInQueue(int op_n, int op, char *key, struct data_t *data);

/**
 * Criar uma request
*/
struct request_t *creat_request(int op_n, int op, char *key, struct data_t *data);

/**
 * Destroi uma dada request
*/
int destroy_request(struct request_t *request);

/**
 * Adiciona o op_n de uma dada request ao array In_progess 
*/
int addToInProgess(struct request_t *request);

/**
 * Remove o op_n de uma dada request ao array In_progess 
*/
int removeToInProgess(struct request_t *request);

/**
 * Função executar por uma thread a request que tem como operação PUT
*/
int execute_put_op(struct request_t *request);

/**
 * Função executar por uma thread a request que tem como operação DELETE
*/
int execute_del_op(struct request_t *request);

/**
 * Print do array de op_n de requests
*/
void printfInProgress();

#endif
