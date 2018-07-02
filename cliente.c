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
#include <sys/select.h>

int
main(int argc, char **argv)
{
    //  Verifica numero de argumentos   //
    if(argc != 4)   {                                                           // Se o numero de argumentos esta incorreto.
        fprintf(stderr, "Utilizacao:\t./cliente <local-port> <server-ip> <server-port>\n"); // Imprime o erro.
        return EXIT_FAILURE;                                                    // Encerra o programa com sinal de erro.
    }

    //  Cria o endpoint de comunicação do cliente  //
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);                                // Socket criado como IPv4 e UDP.
    if(sockfd == -1)    {                                                       // Verifica erro na criaçao do socket.
        perror("socket");                                                       // Imprime o erro.
        return EXIT_FAILURE;                                                    // Encerra o programa com sinal de erro.
    }

    //  Cria a estrutura de endereçamento do cliente   //
    struct sockaddr_in clnt;
    socklen_t clen = sizeof(struct sockaddr_in);                                // Guarda o tamanho da struct do cliente.
    memset(&clnt, 0, (size_t) clen);                                            // Inicializa zerada a estrutura.

    clnt.sin_family = AF_INET;                                                  // IPv4.
    clnt.sin_port = htons(atoi(argv[1]));                                       // Porta do socket.
    clnt.sin_addr.s_addr = INADDR_ANY;                                          // Endereços locais.

    //  Liga o socket à porta e endereço    //
    if(bind(sockfd, (struct sockaddr *) &clnt, clen) == -1) {                   // Verifica erro em bind.
        perror("bind");                                                         // Imprime o erro.
        return EXIT_FAILURE;                                                    // Encerra o programa com sinal de erro.
    }

    //  Recebe mensagens e comandos e envia mensagens e confirmações    //
    struct sockaddr_in serv;                                                    // Estrutura para guardar o endereço do servidor.
    socklen_t slen = sizeof(struct sockaddr_in);                                // Guarda o tamanho da estrutura do servidor.
    memset(&serv, 0, slen);                                                     // Zera a estrutura do servidor.
    serv.sin_family = AF_INET;                                                  // IPv4.
    serv.sin_port = htons(atoi(argv[3]));                                       // Porta do socket do servidor.
    serv.sin_addr.s_addr = inet_addr(argv[2]);                                  // Endereço do servidor.

    //  Variáveis de select //
    fd_set rfd;                                                                 // Leitor de select.
    FD_ZERO(&rfd);                                                              // Zera o leitor.

    char msg[500] = {0};                                                        // Buffer para recebimento das mensagens.
    while(1)    {

        //  Adiciona descritores de arquivos a select   //
        FD_SET(0, &rfd);                                                        // Coloca stdin no leitor.
        FD_SET(sockfd, &rfd);                                                   // Coloca o socket no leitor.

        //  Espera entradas de dados com select //
        int maxfd = sockfd + 1;
        if(select(maxfd, &rfd, NULL, NULL, NULL) == -1) {                       // Verifica erro em select.
            perror("select");                                                   // Imprime o erro.
            return EXIT_FAILURE;                                                // Encerra o programa com sinal de erro.
        }

        if(FD_ISSET(0, &rfd))   {                                               // Verifica se houve ação no teclado (stdin).
            //  Recebe do teclado a mensagem a ser enviada  //
            char sMsg[500] = {0};
            memset(sMsg, 0, (500 * sizeof(char)));
            //size_t read = fread(sMsg, (size_t) sizeof(char), 500, stdin);
            fgets(sMsg, 500, stdin);
            size_t read = strlen(sMsg);
            sMsg[read - 1] = '\0';
            read--;

            //  Envia as mensagens recebidas para os clientes assinantes    //
            if(sendto(sockfd, (char *) sMsg, (read * sizeof(char)), 0, (struct sockaddr *) &serv, slen) == -1) {
                perror("sendto");
                return EXIT_FAILURE;
            }
        }

        else if(FD_ISSET(sockfd, &rfd)) {                                       // Verifica se houve ação no socket.
            //  Recebe mensagens dos clientes   //
            ssize_t recvd = recvfrom(sockfd, (char *) msg, (500 * sizeof(char)), 0, (struct sockaddr *) &serv, &slen);
            if(recvd == 0)  {                                                   // Verifica se o cliente foi fechado.
                fprintf(stdout, "O servidor foi fechado.\n");

            }
            else if(recvd == -1)    {                                           // Verifica erro no recebimento da mensagem.
                perror("recvfrom");                                             // Imprime o erro.
                return EXIT_FAILURE;                                            // Encerra o programa com sinal de erro.
            }

            fprintf(stdout, ">> %s.\n", msg);                                   // Imprime a mensagem recebida.
        }
    }

    close(sockfd);                                                              // Fecha o endpoint do cliente.
    return EXIT_SUCCESS;                                                        // Encerra o programa com sinal de sucesso.
}

