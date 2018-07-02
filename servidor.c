#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdint.h>

struct clients  {
    int exists;
    struct sockaddr_in addr;
    char tags[50][20];
};

int find_client(struct clients *subs, struct sockaddr_in client);
int add_client(struct clients *subs, struct sockaddr_in client);
void add_tag(struct clients *subs, const int index, char *tag);
void del_tag(struct clients *subs, const int index, char *tag);
int tags_counter(char *msg);
char **tag_retriever(char *msg, const int qtdTags);

int
main(int argc, char **argv)
{
    //  Verifica numero de argumentos   //
    if(argc != 2)   {                                                           // Se o numero de argumentos esta incorreto.
        fprintf(stderr, "Utilizacao:\t./servidor <local-port>\n");              // Imprime o erro.
        return EXIT_FAILURE;                                                    // Encerra o programa com sinal de erro.
    }

    //  Cria o endpoint de comunicação do servidor  //
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);                                // Socket criado como IPv4 e UDP.
    if(sockfd == -1)    {                                                       // Verifica erro na criaçao do socket.
        perror("socket");                                                       // Imprime o erro.
        return EXIT_FAILURE;                                                    // Encerra o programa com sinal de erro.
    }

    //  Cria a estrutura de endereçamento do servidor   //
    struct sockaddr_in serv;
    socklen_t slen = sizeof(struct sockaddr_in);                                // Guarda o tamanho da struct do servidor.
    memset(&serv, 0, (size_t) slen);                                            // Inicializa zerada a estrutura.

    serv.sin_family = AF_INET;                                                  // IPv4.
    serv.sin_port = htons(atoi(argv[1]));                                       // Porta do socket.
    serv.sin_addr.s_addr = INADDR_ANY;                                          // Endereços locais.

    //  Liga o socket à porta e endereço    //
    if(bind(sockfd, (struct sockaddr *) &serv, slen) == -1) {                   // Verifica erro em bind.
        perror("bind");                                                         // Imprime o erro.
        return EXIT_FAILURE;                                                    // Encerra o programa com sinal de erro.
    }

    //  Recebe mensagens e comandos e envia mensagens e confirmações    //
    struct sockaddr_in client;                                                  // Estrutura para receber info de clientes.
    socklen_t clen = sizeof(client);                                            // Guarda o tamanho da estrutura do cliente.
    char msg[500] = {0};                                                        // Buffer para recebimento das mensagens.

    struct clients subs[50];                                                    // Vetor que guarda as informações dos assinantes.
    for(int i = 0; i < 50; i++) {
        memset(&(subs[i].addr), 0, sizeof(struct sockaddr_in));
        subs[i].exists = 0;
        for(int j = 0; j < 50; j++)
            strcpy(subs[i].tags[j], "vazio");
    }

    while(1)    {
        memset(&client, 0, clen);                                               // Zera a estrutura do cliente.

        //  Recebe mensagens dos clientes   //
        ssize_t recvd = recvfrom(sockfd, (char *) msg, (500 * sizeof(char)), 0, (struct sockaddr *) &client, &clen);
        if(recvd == 0)  {                                                       // Verifica se o cliente foi fechado.
            fprintf(stdout, "O cliente %hu foi fechado.\n", ntohs(client.sin_port));
            int index = find_client(subs, client);
            if(index != -1)
                subs[index].exists = 0;
        }
        else if(recvd == -1)    {                                               // Verifica erro no recebimento da mensagem.
            perror("recvfrom");                                                 // Imprime o erro.
            return EXIT_FAILURE;                                                // Encerra o programa com sinal de erro.
        }

        //  Verifica o tipo de mensagem //
        if(msg[0] == '+')   {
            char *addTag = &msg[1];
            fprintf(stdout, "Adicionar tag ao cliente.\n");
            int index = find_client(subs, client);
            if(index == -1) {
                index = add_client(subs, client);
            }
            add_tag(subs, index, addTag);
        }
        else if(msg[0] == '-')  {
            char *delTag = &msg[1];
            fprintf(stdout, "Remover tag do cliente.\n");
            int index = find_client(subs, client);
            if(index == -1) {
                index = add_client(subs, client);
            }
            del_tag(subs, index, delTag);
        }
        else    {
            fprintf(stdout, "Mensagem.\n");

            //  Verifica a quantidade de tags na mensagem   //
            int qtdTags = tags_counter(msg);


            //  Recupera as tags da mensagem    //
            char **tags = tag_retriever(msg, qtdTags);

            //  Envia as mensagens recebidas para os clientes assinantes    //
            for(int i = 0; i < qtdTags; i++)    {
                for(int j = 0; j < 50; j++) {
                    if(subs[j].exists)  {
                        for(int k = 0; k < 50; k++) {
                            if(!strcmp(subs[j].tags[k], tags[i]))   {
                                socklen_t sublen = sizeof(subs[j].addr);
                                if(sendto(sockfd, (char *) msg, (recvd * sizeof(char)), 0, (struct sockaddr *) &(subs[j].addr), sublen) == -1) {
                                    perror("sendto");
                                    return EXIT_FAILURE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    close(sockfd);                                                              // Fecha o endpoint do servidor.
    return EXIT_SUCCESS;                                                        // Encerra o programa com sinal de sucesso.
}

int
find_client(struct clients *subs, struct sockaddr_in client)
{
    for(int i = 0; i < 50; i++) {
        if(subs[i].addr.sin_port == client.sin_port)    {
            fprintf(stdout, "Cliente [%hu] encontrado.\n", ntohs(subs[i].addr.sin_port));
            return i;
        }
    }

    fprintf(stdout, "Cliente [%hu] não encontrado.\n", ntohs(client.sin_port));

    return -1;
}

int
add_client(struct clients *subs, struct sockaddr_in client)
{
    for(int i = 0; i < 50; i++) {
        if(subs[i].exists == 0) {
            subs[i].addr = client;
            subs[i].exists = 1;
            fprintf(stdout, "Cliente [%hu] adicionado.\n", ntohs(subs[i].addr.sin_port));
            //i = 50;
            return i;
        }
    }

    return 0;
}

void
add_tag(struct clients *subs, const int index, char *tag)
{
    for(int i = 0; i < 50; i++) {
        if(!strcmp(subs[index].tags[i], "vazio"))  {
            strcpy(subs[index].tags[i], tag);
            fprintf(stdout, "Adicionada tag [%s] ao cliente [%hu].\n", tag, ntohs(subs[index].addr.sin_port));
            i = 50;
        }
    }
}

void
del_tag(struct clients *subs, const int index, char *tag)
{
    for(int i = 0; i < 50; i++) {
        if(!strcmp(subs[index].tags[i], tag))   {
            strcpy(subs[index].tags[i], "vazio");
            fprintf(stdout, "Removida tag [%s] do cliente [%hu].\n", tag, ntohs(subs[index].addr.sin_port));
            i = 50;
        }
    }
}

int
tags_counter(char *msg)
{
    int counter = 0;
    for(size_t i = 0; i < strlen(msg); i++)    {
        if((msg[i] == '#') && (msg[i + 1] != ' '))
            counter++;
    }

    return counter;
}

char **
tag_retriever(char *msg, const int qtdTags)
{            
    //  Cria vetor com as tags  //
    char **tags = malloc(qtdTags * sizeof(char *));
    for(int i = 0; i < qtdTags; i++)
        tags[i] = malloc(20 * sizeof(char));

    int it = 0;
    for(size_t i = 0; i < strlen(msg); i++)    {
        if((msg[i] == '#') && (msg[i + 1] != ' '))  {
            int ii = 0;
            char c = msg[++i];
            while((c != ' ') && (c != '\0'))    {
                tags[it][ii++] = c;
                c = msg[++i];
            }
            it++;
        }
    }

    return tags;
}

