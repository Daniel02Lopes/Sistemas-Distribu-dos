#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "entry.h"
#include "tree-private.h"
#include "tree.h"
#include "data.h"

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create() // esta concluida
{
    struct tree_t *tree = (struct tree_t *)malloc(sizeof(struct tree_t));
    tree->root = NULL;
    if (tree == NULL )
    {
        printf("Falha ao criar a árvore");
        return NULL;
    }
    return tree;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree) // incompleta falta libertar os nos
{
    if (tree == NULL)
    {
        printf("Erro ao libertar toda a memória ocupada por uma árvore.");
    }
    aux_tree_destroy(tree->root);
    free(tree);
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro        free(data_to_return);.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value)
{
    if (tree == NULL || key == NULL || value == NULL)
    {
        printf("Erro ao adicionar um par chave-valor à árvore.");
        return -1;
    }
    if(tree->root == NULL)
    {
        struct data_t *data_to_add = data_dup(value);
        char *key_to_add = malloc(strlen(key) + 1);
        if (data_to_add == NULL || key_to_add == NULL)
        {
            return -1;
        }
        strcpy(key_to_add, key);
        struct entry_t *aux_entry = entry_create(key_to_add, data_to_add);
        tree->root = (struct node *)malloc(sizeof(struct node));
        if (tree->root == NULL)
        {
            entry_destroy(aux_entry);
            return -1;
        }
        tree->root->entry = entry_dup(aux_entry);
        tree->root->left = NULL;
        tree->root->right = NULL;
        entry_destroy(aux_entry);
        return 0;
    
    }
    return aux_tree_put(tree->root, key, value) == NULL ? -1 : 0;
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key)
{
    if (tree == NULL || key == NULL)
    {
        return NULL;
    }
    struct node *aux_node = getNodeOfTree(tree->root, key);
    if (aux_node == NULL)
    {
        return NULL;
    }
    struct data_t *data_toReturn = data_dup(aux_node->entry->value);
    if (data_toReturn == NULL)
    {
        data_destroy(data_toReturn);
        return NULL;
    }
    return data_toReturn;
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key)
{
    if (tree == NULL || tree->root == NULL || key == NULL)
    {
        return -1;
    }
    struct node *node_to_delete = getNodeOfTree(tree->root, key);
    if (node_to_delete == NULL) // nothing to delete
    {
        free(node_to_delete);
        return -1;
    }
    if (node_to_delete->left == NULL && node_to_delete->right == NULL) // has 0 children
    {
        printf("ola");
        if (node_to_delete->entry != NULL)
        {
            struct node *father_of_node = get_father(tree->root, key);
            if (father_of_node != NULL) // if has a father go put NULL
            {
                if (father_of_node->left == node_to_delete)
                {
                    father_of_node->left = NULL;
                }
                else
                {
                    father_of_node->right = NULL;
                }
            }
            else
            {
                tree->root = NULL;
            }
            entry_destroy(node_to_delete->entry);
        }
        free(node_to_delete);
        return 0;
    }
    else if (node_to_delete->right != NULL && node_to_delete->left != NULL) // has 2 children
    {
        struct node *successor = getLowerNode(node_to_delete->right);
        struct node *fathers_successor = get_father(node_to_delete, successor->entry->key);
        entry_destroy(node_to_delete->entry);
        node_to_delete->entry = entry_dup(successor->entry);
        if (successor->right == NULL && fathers_successor->left == successor)
        {
            fathers_successor->left = NULL;
        }
        else if (successor->right == NULL && fathers_successor->right == successor)
        {
            fathers_successor->right = NULL;
        }
        else if (successor->right != NULL && fathers_successor->left == successor)
        {
            fathers_successor->left = successor->right;
            successor->right = NULL;
        }
        else
        {
            fathers_successor->right = successor->right;
            successor->right = NULL;
        }

        entry_destroy(successor->entry);
        free(successor);
        return 0;
    }
    else if (node_to_delete->left != NULL && node_to_delete->right == NULL) // has 1 children (left)
    {
        struct node *father_of_node = get_father(tree->root, key);
        if (father_of_node != NULL)
        {
            if (father_of_node->right == node_to_delete)
            {
                father_of_node->right = node_to_delete->left;
            }
            else
            {
                father_of_node->left = node_to_delete->left;
            }
        }
        else
        {
            tree->root = node_to_delete->left;
        }
        entry_destroy(node_to_delete->entry);
        free(node_to_delete);
        return 0;
    }
    else // has 1 children (right)
    {
        struct node *father_of_node = get_father(tree->root, key);
        if (father_of_node != NULL)
        {
            if (father_of_node->right == node_to_delete)
            {
                father_of_node->right = node_to_delete->right;
            }
            else
            {
                father_of_node->left = node_to_delete->right;
            }
        }
        else
        {
            tree->root = node_to_delete->right;
        }
        entry_destroy(node_to_delete->entry);
        free(node_to_delete);
        return 0;
    }
    return 0;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree)
{
    if (tree == NULL)
    {
        printf("Erro ao calcular o número de elementos da árvore");
        return -1;
    }
    if (tree->root == NULL || tree->root->entry == NULL)
    {
        return 0;
    }
    return countNodesTree(tree->root);
}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree)
{
    if (tree == NULL)
    {
        printf("Erro ao calcular a altura da árvore");
        return -1;
    }
    else if (tree->root == NULL){
        return 0;
    }
    else if (tree->root->entry == NULL || (tree->root->left == NULL && tree->root->right == NULL))
    {
        return 0;
    }
    else if (tree_size(tree) == 2)
    {
        return 1;
    }
    return aux_tree_height(tree->root);
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree)
{
    if (tree == NULL)
    {
        return NULL;
    }
    char **array_key = (char **)calloc(tree_size(tree) + 1, sizeof(char *));
    if (array_key == NULL)
    {
        free(array_key);
        printf("Erro ao reservar memória no array");
        return NULL;
    }
    int index = aux_keys(tree->root, array_key, 0);
    array_key[index] = NULL;
    return array_key;
}

/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys)
{
    if (keys == NULL)
    {
        return;
    }
    int i = 0;
    while (keys[i] != NULL)
    {
        free(keys[i]);
        i++;
    }
    free(keys);
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree)
{
    if (tree == NULL)
    {
        return NULL;
    }
    void **array_values = (void **)calloc(tree_size(tree) + 1, sizeof(void *));
    if (array_values == NULL)
    {
        printf("Erro ao reservar memória no array");
        return NULL;
    }
    int index = aux_values(tree->root, array_values, 0);
    array_values[index] = NULL;
    return array_values;
}

/* Função que liberta toda a memória alocada por tree_get_values().
 */
void tree_free_values(void **values)
{
    int i = 0;
    if (values == NULL)
    {
        return;
    }
    while (values[i] != NULL)
    {
        free(values[i]);
        i++;
    }
    free(values);
}
