#include "network_server.h"
#include "tree_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * PROJETO CRIADO POR :GRUPO30
 * Daniel Lopes - fc56357
 * Miguel Ramos - fc56377
 * Tomás Piteira - fc56303
 */

int main(int argc, char *argv[])
{

  /* Testar os argumentos de entrada */

  if (argc != 3)
  {
    printf("ERROR\n");
    return -1;
  }

  /* inicialização da camada de rede */
  int socket_de_escuta = network_server_init(atoi(argv[1]));
  creat_universal_tree();
  if (tree_skel_init(argv[1], argv[2]) == -1)
  {
    printf("Erro ao inicializar o servidor\n");
    return -1;
  }
  if (network_main_loop(socket_de_escuta) == -1)
  {
    return -1;
  }

  network_server_close();
  tree_skel_destroy();
  return 0;
}
