#include "tree-private.h"
#include "entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

/**
 * Função auxiliar do tree_destroy em que irá remover todas as entrys de todos os nós e os próprios nós
 */
void aux_tree_destroy(struct node *root)
{
    if (root != NULL)
    {
        aux_tree_destroy(root->left);
        aux_tree_destroy(root->right);
        entry_destroy(root->entry);
        free(root);
    }
}

/**
 * Função que através de um nó vai percorrer o próprio e os seus filhos
 * até encontrar um nó com a chave. Caso não encontre retorna NULL
 */
struct node *getNodeOfTree(struct node *root, char *key)
{
    if (root == NULL || root->entry == NULL || root->entry->key == NULL || key == NULL)
    {
        return NULL;
    }
    char *key_to_add = malloc(strlen(key) + 1);
    strcpy(key_to_add, key);
    struct entry_t *aux_entry = entry_create(key_to_add, NULL);
    int comparation = entry_compare(root->entry, aux_entry);

    if (comparation == 0) // keys are the same
    {
        entry_destroy(aux_entry);
        return root;
    }
    else if (comparation == 1) // if keys are diferent and the key to add is lower then the key's root so go left
    {
        entry_destroy(aux_entry);
        return getNodeOfTree(root->left, key);
    }
    else
    {
        entry_destroy(aux_entry);
        return getNodeOfTree(root->right, key);
    }
}

/*Função auxiliar do tree_size, que vai contar o número de filhos
 * a partir de um dado nó incluindo o próprio
 */
int countNodesTree(struct node *root)
{
    if (root == NULL)
    {
        return 0;
    }
    return countNodesTree(root->left) + countNodesTree(root->right) + 1;
}

/*Função auxiliar do tree_height que calcula a altura da tree apartir de um dado nó
 */
int aux_tree_height(struct node *root)
{
    if (root == NULL)
    {
        return 0;
    }
    int leftHeight = aux_tree_height(root->left);
    int rightHeight = aux_tree_height(root->right);
    return leftHeight > rightHeight ? leftHeight + 1 : rightHeight + 1;
}

/*
 * Função auxiliar do tree_put que dado um nó (root) vai uma chave e um value.
 * Se a função correr corretamente irá retornar nó criado ou substituido, caso contrário retorna NULL
 */
struct node *aux_tree_put(struct node *root, char *key, struct data_t *value)
{
    struct data_t *data_to_add = data_dup(value);
    char *key_to_add = malloc(strlen(key) + 1);
    if (data_to_add == NULL || key_to_add == NULL)
    {
        return NULL;
    }
    strcpy(key_to_add, key);
    struct entry_t *aux_entry = entry_create(key_to_add, data_to_add);

    // if Node is NULL
    if (root == NULL)
    {
        root = (struct node *)malloc(sizeof(struct node));
        if (root == NULL)
        {
            entry_destroy(aux_entry);
            return NULL;
        }
        root->entry = entry_dup(aux_entry);
        root->left = NULL;
        root->right = NULL;
        entry_destroy(aux_entry);
        return root;
    }

    else if (root->entry == NULL)
    {
        root->entry = entry_dup(aux_entry);
        entry_destroy(aux_entry);
        return root;
    }
    else
    {
        int comparation = entry_compare(root->entry, aux_entry);
        // if keys are the same
        if (comparation == 0)
        {
            entry_replace(root->entry, key_to_add, data_to_add);
            free(aux_entry);
            return root;
        }
        // if keys are diferent and the key is lower then the key's root so go left
        else if (comparation == 1)
        {
            root->left = aux_tree_put(root->left, key_to_add, data_to_add);
        }
        else
        {
            root->right = aux_tree_put(root->right, key_to_add, data_to_add);
        }
        entry_destroy(aux_entry);
        return root;
    }
}

/**
 * Função que dado um nó e uma chave, irá percorrer os nós até que encontre o nó.
 * Caso encontre devolve o pai caso contrário devolve NULL
 */
struct node *get_father(struct node *root, char *key)
{
    int right_comparation, left_comparation = -10;
    if (root == NULL || root->entry == NULL || root->entry->key == NULL || key == NULL)
    {
        return NULL;
    }
    int comparation = strcmp(root->entry->key, key);
    if (root->right == NULL && root->left == NULL && comparation == 0)
    {
        return NULL;
    }
    if (root->right != NULL)
    {
        right_comparation = strcmp(root->right->entry->key, key);
    }
    if (root->left != NULL)
    {
        left_comparation = strcmp(root->left->entry->key, key);
    }

    if (right_comparation == 0 || left_comparation == 0)
    {
        return root;
    }
    else if (comparation == 1)
    {
        return get_father(root->left, key);
    }
    else
    {
        return get_father(root->right, key);
    }
}

/**
 * Função que vai retornar o menor nó (à esquerda) existente dado um nó.
 * Caso contrário retorna NULL
 * Função usada no tree_del.
 */
struct node *getLowerNode(struct node *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    while (node->left != NULL)
    {
        node = node->left;
    }
    return node;
}

/**
 * Função auxiliar do tree_get_keys. Retorna o número de keys adicionadas ao array
 */
int aux_keys(struct node *node, char **array, int i)
{
    if (node == NULL)
    {
        return i;
    }
    char *key_to_add = malloc(strlen(node->entry->key) + 1);
    strcpy(key_to_add, node->entry->key);
    array[i] = key_to_add;
    i++;
    if (node->left != NULL)
        i = aux_keys(node->left, array, i);
    if (node->right != NULL)
        i = aux_keys(node->right, array, i);
    return i;
}

/**
 * Função auxiliar do tree_get_values Retorna o número de values adicionados ao array
 */
int aux_values(struct node *node, void **array_values, int i)
{

    if (node == NULL)
    {
        return i;
    }
    int size = node->entry->value->datasize;
    array_values[i] = malloc(size*sizeof(int));
    memcpy(array_values[i], node->entry->value->data, size);
    i++;
    if (node->left != NULL)
        i = aux_values(node->left, array_values, i);
    if (node->right != NULL)
        i = aux_values(node->right, array_values, i);
    return i;
}
