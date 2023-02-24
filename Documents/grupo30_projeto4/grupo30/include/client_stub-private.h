#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "inet.h"
#include "data.h"
#include <zookeeper/zookeeper.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

struct rtree_t {
    char * address_port;
    int sockfd;
    struct sockaddr_in server;
    char * node_; //identificador node deste servidor 
};

struct rtree_t *rtree_connect_aux(const char *address_port);
int zk_disconnect();
int zk_put(struct entry_t *entry);
struct data_t *zk_get(char *key);
int zk_del(char *key);
int zk_size();
int zk_height();
char **zk_getKeys();
void **zk_getValues();
int zk_verify(int op_n);
#endif