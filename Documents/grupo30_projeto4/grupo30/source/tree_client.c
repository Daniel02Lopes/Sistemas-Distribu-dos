#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entry.h"
#include "data.h"
#include <ctype.h>
#include "client_stub-private.h"
#include "client_stub.h"

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

int main(int argc, char const *argv[])
{
    printf("-----------------------\n");
    printf("----BEM VINDO CLIENTE--\n");
    printf("-----------------------\n");
    
    char *address_port = (char *)argv[1];
    printf("-TENTAR ESTABELECER LIGAÇÃO-\n");
    struct rtree_t * rtree = rtree_connect(address_port);
    if (rtree == NULL)
    {
        printf("Erro ao estabelecer associação entre o cliente e o servidor\n");
        return -1;
    }
    else
    {
        printf("-CONEXÃO ESTABELECIDA COM SUCESSO-\n");
        printf("-----------------------\n");
    }
    printf("----COMANDOS VÁLIDOS---\n");
    printf("--> put <key> <data> --\n");
    printf("--> get <key> ---------\n");
    printf("--> del <key> ---------\n");
    printf("--> size --------------\n");
    printf("--> height ------------\n");
    printf("--> getkeys -----------\n");
    printf("--> getvalues ---------\n");
    printf("--> quit --------------\n");
    printf("--> verify <op_n>------\n");
    printf("-----------------------\n");
    while (1)
    {
        const int size = 200;
        char *comandline;
        printf("-->COMANDO:");
        char aux_comandline[size];
        fgets(aux_comandline, size, stdin);
        comandline = strtok(aux_comandline, " \n");
        printf("-----------------------\n");
        if (comandline == NULL)
        {
            printf("COMANDO INVÁLIDO!-TENTE OUTRA VEZ\n");
            continue;
        }
        if (strcmp(comandline, "put") == 0)
        {
            char *key = strtok(NULL, " ");
            if (key == NULL)
            {
                printf("Formato do put inválido!\n");
                continue;
            }
            void *data = strtok(NULL, "\0");
            if (data == NULL)
            {
                printf("Formato do put inválido!\n");
                continue;
            }
            struct data_t *dataToAdd = data_create2(strlen(data), data);
            struct entry_t *entryToAdd = entry_create(key, dataToAdd);
            if (zk_put(entryToAdd) == -1)
            {
                printf("-ERRO AO ADICIONAR UM ELEMENTO NA ÁRVORE-\n");
                printf("-----------------------\n");
                continue;
            }
            else
            {
                printf("-ADICIONADO COM SUCESSO-\n");
                printf("-----------------------\n");
            }
        }
        else if (strcmp(comandline, "del") == 0)
        {
            char *key = strtok(NULL, " ");
            key = strtok(key, "\n");
            if (key == NULL)
            {
                printf("Formato do del inválido!\n");
                continue;
            }
            if (zk_del(key) == -1)
            {
                printf("ERRO AO REMOVER A ENTRY\n");
                printf("KEY NÃO EXISTE!\n");
            }
            else
            {
                printf("REMOÇÃO BEM SUCEDIDA\n");
            }
        }
        else if (strcmp(comandline, "size") == 0)
        {
            int size = zk_size();
            if (size == -1)
            {
                printf("Não foi possivel obter o size da tree\n");

                continue;
            }
            else
            {
                printf("-->SIZE:%d  -----------\n", size);
                printf("-----------------------\n");
            }
        }
        else if (strcmp(comandline, "get") == 0)
        {
            char *key = strtok(NULL, " ");
            key = strtok(key, "\n");
            if (key == NULL)
            {
                printf("FORMATO INVÁLIDO DO GET!\n");
                continue;
            }
            struct data_t *dataToAdd = zk_get(key);
            if (dataToAdd == NULL)
            {
                printf("-Erro ao devolver o value da key\n");
                printf("-KEY NAO EXISTE!\n");
            }
            free(dataToAdd);
        }
        else if (strcmp(comandline, "height") == 0)
        {
            int height = zk_height();
            if (height == -1)
            {
                printf("Não foi possivel obter o height da tree \n");
            }
            else
            {
                printf("-->HEIGHT:%d ---------\n", height);
                printf("-----------------------\n");
            }
        }
        else if (strcmp(comandline, "getkeys") == 0)
        {
            char **keys = zk_getKeys();
            if (keys == NULL)
            {
                printf("Nao foi possivel obter as keys da arvore\n");
                printf("POSSIVELMENTE A TREE ESTA VAZIA!\n");
                printf("--------------------------------\n");
                continue;
            }
            else
            {

                for (int i = 0; keys[i] != NULL; ++i)
                {
                    printf("-->KEY:%s\n", keys[i]);
                }
                printf("-------------\n");

                int i = 0;
                while (keys[i] != NULL)
                {
                    free(keys[i]);
                    i++;
                }
                free(keys);
            }
        }
        else if (strcmp(comandline, "getvalues") == 0)
        {
            void **values = zk_getValues();
            if (values == NULL)
            {
                printf("Nao foi possivel obter os values da arvore\n");
                printf("POSSIVELMENTE A TREE ESTA VAZIA!\n");
                printf("--------------------------------\n");
                continue;
            }

            int i = 0;

            while (values[i] != NULL)
            {
                free(values[i]);
                i++;
            }
            free(values);
        }

        else if (strcmp(comandline, "verify") == 0)
        {
            char *aux_op_n = strtok(NULL, " ");
            aux_op_n = strtok(aux_op_n, "\n");
            int isnumber = 0;
            if (aux_op_n == NULL)
            {
                printf("Formato do verify inválido!\n");
                continue;
            }
            for (int i = 0; aux_op_n[i] != '\0'; i++)
            {
                if (isdigit(aux_op_n[i]) == 0)
                {
                    printf("Formato do verify inválido!\n");
                    printf("Introduza um valor inteiro\n");
                    isnumber = -1;
                    break;
                }
            }
            if (isnumber == -1)
                continue;

            int op_n = atoi(aux_op_n);
            if (op_n < 1)
            {
                printf("Por favor introduza um valor maior que 0\n");
                continue;
            }
            int verify = zk_verify(op_n);
            if (verify == 0)
            {
                printf("Operação ainda não foi executada!\n");
            }
            else if (verify == -1)
            {
                printf("Operação não fui criada!\n");
            }
            else
            {
                printf("Operação foi executada!\n");
            }
        }
        else if (strcmp(comandline, "quit") == 0)
        {
            if (zk_disconnect() == -1)
            {
                printf("--Erro ao terminar a associação entre o cliente e o servidor\n");
                return -1;
            }
            else
            {
                printf("--TERMINAÇÃO BEM SUCESSIDA\n");
                printf("--------------------------\n");
            }
            return 0;
        }
        else
        {
            printf("COMANDO INVÁLIDO!-TENTE OUTRA VEZ\n");
        }
    }
    printf("--------------------------\n");
    if (zk_disconnect() == 0)
    {
        printf("--DESCONEXÃO BEM SUCEDIDA--\n");
        printf("---------VOLTE SEMPRE------\n");
    }

    return 0;
}