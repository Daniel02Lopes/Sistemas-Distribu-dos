#
#  PROJETO CRIADO POR :GRUPO30
#   Miguel Ramos - fc56377
#   Daniel Lopes - fc56357
#   TomÃ¡s Piteira - fc56303
#

CC = gcc
INC_DIR = include
OBJ_DIR = object
BIN_DIR = binary
SRC_DIR = source
LIB_DIR = lib


OBJETOS = sdmessage.pb-c.o message.o data.o entry.o tree-private.o tree.o tree_skel.o client_stub.o network_client.o network_server.o client-lib.o tree_client.o tree_server.o

tree-private.o = tree-private.h
data.o = data.h 
entry.o = data.h entry.h
tree.o = entry.h data.h tree-private.h
sdmessage.pb-c.o = sdmessage.pb-c.h
tree_skel.o = tree.h sdmessage.pb-c.h client_stub-private.h client_stub.h
message.o = message-private.h inet.h sdmessage.pb-c.h
client_stub.o = data.h entry.h client_stub.h client_stub-private.h sdmessage.pb-c.h message-private.h network_client.h
network_client.o = network_client.h sdmessage.pb-c.h client_stub-private.h message-private.h
network_server.o = network_server.h tree_skel.h inet.h message-private.h 
client-lib.o = client_stub.h network_client.h data.h entry.h sdmessage.pb-c.h
tree_client.o = data.h entry.h client_stub.h client_stub-private.h
tree_server.o = tree_skel.h network_server.h

CFLAGS = -Wall -g -I
LIBS= -lrt -lpthread -lzookeeper_mt

run : 
	make protoc
	make all

all: $(OBJETOS) client-lib.o tree-client tree-server

protoc: sdmessage.proto
	protoc --c_out=. sdmessage.proto
	mv sdmessage.pb-c.h $(INC_DIR)/
	mv sdmessage.pb-c.c $(SRC_DIR)/

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC_DIR) -o $(OBJ_DIR)/$@ -c $<

client-lib.o: client_stub.o network_client.o data.o entry.o message.o sdmessage.pb-c.o
	ld -r $(addprefix $(OBJ_DIR)/,$^) -o $(LIB_DIR)/$@

tree-client: tree_client.o data.o entry.o message.o client_stub.o network_client.o network_server.o tree_skel.o tree.o tree-private.o sdmessage.pb-c.o
	$(CC) $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c $(LIBS) -o $(BIN_DIR)/$@

tree-server: tree_server.o data.o entry.o tree.o message.o client_stub.o network_client.o tree_skel.o network_server.o sdmessage.pb-c.o tree-private.o client_stub.o
	$(CC) $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c $(LIBS) -o $(BIN_DIR)/$@

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*
	rm -f $(LIB_DIR)/*
	rm -f $(SRC_DIR)/sdmessage.pb-c.c
	rm -f $(INC_DIR)/sdmessage.pb-c.h