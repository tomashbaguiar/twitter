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

    while(1)    {
        memset(&client, 0, clen);                                               // Zera a estrutura do cliente.

        //  Recebe mensagens dos clientes   //
        ssize_t recvd = recvfrom(sockfd, (char *) msg, (500 * sizeof(char)), 0, (struct sockaddr *) &client, &clen);
        if(recvd == 0)  {                                                       // Verifica se o cliente foi fechado.
            fprintf(stdout, "O cliente %hu foi fechado.\n", ntohs(client.sin_port));

        }
        else if(recvd == -1)    {                                               // Verifica erro no recebimento da mensagem.
            perror("recvfrom");                                                 // Imprime o erro.
            return EXIT_FAILURE;                                                // Encerra o programa com sinal de erro.
        }
printf("Recebido %s.\n", msg);

        //  Verifica o tipo de mensagem //
        if(msg[0] == '+')   {
            fprintf(stdout, "Adicionar cliente.\n");
        }
        else if(msg[0] == '-')  {
            fprintf(stdout, "Remover cliente.\n");
        }
        else    {
            fprintf(stdout, "Mensagem.\n");
            //  Envia as mensagens recebidas para os clientes assinantes    //
            if(sendto(sockfd, (char *) msg, (recvd * sizeof(char)), 0, (struct sockaddr *) &client, clen) == -1) {
                perror("sendto");
                return EXIT_FAILURE;
            }
        }
    }

    close(sockfd);                                                              // Fecha o endpoint do servidor.
    return EXIT_SUCCESS;                                                        // Encerra o programa com sinal de sucesso.
}

