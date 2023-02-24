#include "inet.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

struct message_t *create_message()
{
    
    struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));

    message_t__init(&msg->message);
    if (msg == NULL)
    {
        printf("Erro ao inicializar a Mensagem");
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }
    msg->message.entry = (EntryT *)malloc(sizeof(EntryT));
    entry_t__init(msg->message.entry);
    
    if (msg->message.entry == NULL)
    {
        printf("Erro ao inicializar o Entry da Mensagem");
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }
    msg->message.entry->data = (DataT *)malloc(sizeof(DataT));
    data_t__init(msg->message.entry->data);
    
    if (msg->message.entry->data == NULL)
    {
        printf("Erro ao inicializar o Data da Entry da Mensagem");
        message_t__free_unpacked(&msg->message, NULL);
        return NULL;
    }

    return msg;
}

int write_all(int sock, uint8_t *buf, int len)
{
    int bufsize = len;

    while (len > 0)
    {
        int res = write(sock, buf, len);
        if (res < 0)
        {
            if (errno == EINTR)
                continue;
            perror("write failed!");
            return res;
        }
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, uint8_t *buf, int len)
{
    int aux_read = 0;
    int result;
    while (aux_read < len)
    {
        result = read(sock, buf + aux_read, len - aux_read);
        if (result < 1)
        {
            perror("Read failed!");
            return result;
        }
        else if (result == 0)
        {
            return 0;
        }
        aux_read += result;
    }
    return aux_read;
}
