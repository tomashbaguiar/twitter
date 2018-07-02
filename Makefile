PROGRAMS := servidor cliente
CC := gcc
FLAGS := -Wall -Werror -pedantic -Wextra -fno-stack-protector -std=c11

all: $(PROGRAMS)

servidor: servidor.c 
	$(CC) $(FLAGS) servidor.c -o servidor
cliente: cliente.c 
	$(CC) $(FLAGS) cliente.c -o cliente

clean: 
	rm -rf cliente servidor
