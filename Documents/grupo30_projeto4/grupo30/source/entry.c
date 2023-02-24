#include "entry.h"
#include "data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.
 */
struct entry_t *entry_create(char *key, struct data_t *data)
{
    if (key == NULL)
    {
        return NULL;
    }
    struct entry_t *entry;
    entry = (struct entry_t *)malloc(sizeof(struct entry_t));

    if (entry == NULL)
    {
        free(entry);
        return NULL;
    }

    entry->key = key;
    entry->value = data;

    return entry;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry)
{

    if (entry != NULL)
    {
        if (entry->value != NULL)
        {
            data_destroy(entry->value);
            free(entry->key);
            free(entry);
        }
        else
        {
            free(entry->key);
            free(entry);
        }
    }
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry)
{
    if (entry == NULL)
    {
        return NULL;
    }
    else
    {
        char *key = strdup(entry->key);
        struct data_t *value = data_dup(entry->value);
        return entry_create(key, value);
    }
}

/* Função que substitui o conteúdo de uma entrada entry_t.
 *  Deve assegurar que destroi o conteúdo antigo da mesma.
 */
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value)
{
    if (entry != NULL)
    {
        data_destroy(entry->value);
        entry->value = new_value;
        free(entry->key);
        entry->key = new_key;
    }
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
 *  Ordem das entradas é definida pela ordem das suas chaves.
 *  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
 */
int entry_compare(struct entry_t *entry1, struct entry_t *entry2)
{

    int result = strcmp(entry1->key, entry2->key);

    if (result < 0)
    {
        return -1;
    }

    if (result > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}