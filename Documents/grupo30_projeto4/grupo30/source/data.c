#include "data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

/* Função que cria um novo elemento de dados data_t, reservando a memória
 * necessária para armazenar os dados, especificada pelo parâmetro size
 */
struct data_t *data_create(int size)
{

    if (size < 1)
        return NULL;

    struct data_t *dat = (struct data_t *)malloc(sizeof(struct data_t));

    if (dat == NULL)
    {
        free(dat);
        return NULL;
    }

    dat->datasize = size;
    dat->data = malloc(size);

    if (dat->data == NULL)
    {
        free(dat->data);
        free(dat);
        return NULL;
    }
    return dat;
}

/* Função que cria um novo elemento de dados data_t, inicializando o campo
 * data com o valor passado no parâmetro data, sem necessidade de reservar
 * memória para os dados.
 */
struct data_t *data_create2(int size, void *data)
{

    if (size < 1 || data == NULL)
        return NULL;

    struct data_t *dat = data_create(size);

    if (dat == NULL)
    {
        free(dat->data);
        free(dat);
        return NULL;
    }
    free(dat->data);
    dat->data = data;
    if (dat->data == NULL)
    {
        free(dat->data);
        free(dat);
        return NULL;
    }

    return dat;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data)
{
    if (data != NULL)
    {
        if (data->data != NULL)
        {
            free(data->data);
        }
        free(data);
    }
}

/* Função que duplica uma estrutura data_t, reservando toda a memória
 * necessária para a nova estrutura, inclusivamente dados.
 */
struct data_t *data_dup(struct data_t *data)
{
    if (data == NULL || data->data == NULL || data->datasize < 1)
        return NULL;

    struct data_t *dups = data_create(data->datasize);
    memcpy(dups->data, data->data, data->datasize);

    return dups;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
 *  Deve assegurar que destroi o conteúdo antigo do mesmo.
 */
void data_replace(struct data_t *data, int new_size, void *new_data)
{

    if (data != NULL && data->data != NULL && new_size > 0 && new_data != NULL)
    {
        free(data->data);
        data->datasize = new_size;
        data->data = new_data;
    }
}