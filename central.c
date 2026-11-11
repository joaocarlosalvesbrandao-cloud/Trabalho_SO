#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "central.h"

int main() {

        char msg[100];
        char buf[100];
        int max_estafetas; // Variável para guardar o numero de estafetas
        int prazo_aceita;

        setbuf(stdout, NULL);

        if (access(FICHEIRO_TMP, F_OK) == 0){
		printf("A central já está em execucao\n");
		exit(0);
	}

        int f = open(FICHEIRO_TMP, O_CREAT | O_EXCL | O_RDWR, 0666);
        if(f == -1){
                printf("Erro na criação do ficheiro temporário\n");
                exit(1);
        }
        close(f);

    

        char *estafetas_temp = getenv("MAXESTAFETAS"); // "Capturar" o valor da variávle com o nome "MAXESTAFETAS" que está em memória
        char *prazo_temp = getenv("PRAZOACEITA"); // ----------- Igual ao de cima ------------

        if (estafetas_temp != NULL) {
                max_estafetas = atoi(estafetas_temp); // Se o valor que "capturou" for != NULL, converte o char para int, com a funçao atoi
        }

        else {
                max_estafetas = MAXESTAFETAS; // Senão (estafetas_temp == NULL), usa o valor definido por default, guardado no central.h
        }

        if (prazo_temp != NULL) {
                prazo_aceita = atoi(prazo_temp);
        }

        else {
                prazo_aceita = PRAZOACEITA;
        }

        printf("\n%d\n%d\n",max_estafetas, prazo_aceita);

        int pipe_c2t[2]; // Criar array para guardar a entrada/saida do tunel, [0] saída, [1] entrada
        int pipe_t2c[2]; // ------- Igual ao de cima -------

        if (pipe(pipe_c2t) == -1) { // pipe(.....), cria o pipe (central -> tarifador)
                perror("Erro ao criar o pipe para o tarifador");
                exit(1);
        }
        if (pipe(pipe_t2c)== -1) { // pipe(....), cria o pipe (tarifador -> central)
                perror("Erro ao criar o pipe de rececao do tarifador");
                exit(1);
        }

        int retproc;
        pid_t pid = fork();
        if(pid == -1){
                perror("erro no fork");
                exit(1);
        }
        if(pid ==0){
                //filho
                //substituição Stdin
                close(0);
                dup(pipe_c2t[0]);
                close(pipe_c2t[0]);
                close(pipe_c2t[1]);

                //substituição Stdout
                close(1);
                dup(pipe_t2c[1]);
                close(pipe_t2c[0]);
                close(pipe_t2c[1]);


                execl("./tarifador", "tarifador", NULL);
                perror("erro no exec do tarifador");
                exit(1);
        }
        //Fechar os pipes criados para o filho
        close(pipe_c2t[0]); //fechar porque o pai não vai ler
        close(pipe_t2c[1]); //Fechar porque o pai não vai escrever

        //Num pipe
        /*
                [0] é a ponta de leitura
                [1] é a ponta de escrita
                //mas como criamos os 
        */

        int nbytes_w = write(pipe_c2t[1], msg, strlen(msg));
        if(nbytes_w==-1){
                perror("errro na escrita para o stdin tarifador");
                close(f);
                unlink(FICHEIRO_TMP);
                exit(1);
        }
        //close(pipe_c2t[1]);

        //read não acrescenta '\0'
        int nbytes_r = read(pipe_t2c[0], buf, sizeof(buf)-1); //sizeof(buf)-1 para ter espaço para o \0
        if(nbytes_r == -1){
                perror("errro na leitura stdout tarifador");
                close(f);
                unlink(FICHEIRO_TMP);
                exit(1);
        }
        buf[nbytes_r]='\0';
        printf("Recebi do buffer: %s\n", buf);
        //close(pipe_t2c[0]);
        wait(&retproc);

        close(f);
        unlink(FICHEIRO_TMP);



        return 0;
}
