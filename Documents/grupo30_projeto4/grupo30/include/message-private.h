#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "inet.h"
#include "sdmessage.pb-c.h"
#include <errno.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

struct message_t{
    MessageT message;
};

struct message_t *create_message();

int write_all(int sock, uint8_t *buf, int len);

int read_all(int sock, uint8_t *buf, int len);

#endif