#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
*/

struct tree_t
{
	struct node *root;
};

struct node
{
	struct node *left;
	struct node *right;
	struct entry_t *entry;
};

/**
 * Função auxiliar do tree_destroy em que irá remover todas as entrys de todos os nós e os próprios nós
 */
void aux_tree_destroy(struct node *root);

/**
 * Função que através de um nó vai percorrer o próprio e os seus filhos
 * até encontrar um nó com a chave. Caso não encontre retorna NULL
 */
struct node *getNodeOfTree(struct node *root, char *key);

/*Função auxiliar do tree_size, que vai contar o número de filhos
 * a partir de um dado nó incluindo o próprio
 */
int countNodesTree(struct node *root);


/*Função auxiliar do tree_height que calcula a altura da tree apartir de um dado nó
 */
int aux_tree_height(struct node *root);

/*
 * Função auxiliar do tree_put que dado um nó (root) vai uma chave e um value.
 * Se a função correr corretamente irá retornar nó criado ou substituido, caso contrário retorna NULL
 */
struct node *aux_tree_put(struct node *root, char *key, struct data_t *value);

/**
 * Função que dado um nó e uma chave, irá percorrer os nós até que encontre o nó.
 * Caso encontre devolve o pai caso contrário devolve NULL
 */
struct node *get_father(struct node *root, char *key);

/**
 * Função que vai retornar o menor nó (à esquerda) existente dado um nó.
 * Caso contrário retorna NULL
 * Função usada no tree_del.
 */
struct node* getLowerNode(struct node* node);

/**
 * Função auxiliar do tree_get_keys. Retorna o número de keys adicionadas ao array
 */
int aux_keys(struct node *node, char **array, int i);

/**
 * Função auxiliar do tree_get_values Retorna o número de values adicionados ao array
 */
int aux_values(struct node *node, void **array_values, int i);

#endif