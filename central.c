#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "central.h"

typedef struct {
        int recebidos;
        int atribuidos;
        int concluidos;
        int expirados;
        int perdidos;
        float valor_total;
} Stats;

int main(int argc, char *argv[]) { // Vai receber estes dois argumentos para o "-m" ou "-p"

        char msg[100];
        char buf[100];
        int max_estafetas; // Variável para guardar o numero de estafetas
        int prazo_aceita;

        Stats estatisticas = {0,0,0,0,0,0.0}; // Inicializar a estrutura tudo a 0

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

        int opt;
        while ((opt = getopt(argc, argv, "m:p:")) != -1) { // getopt vai ler o que foi inserido no comando, neste caso ao iniciar o program "./central -p .... -m ....""
                                                           // ele vai ler e ver se existe essas letras (m, p), os ":" a separar é para ler logo o que está a seguir dessa letra
                                                           // guardando isso na variavel optarg (variável default e automática da biblioteca "unistd.h")
                                                           // A função getopt retorna a propria letra que leu se correr bem, quando acaba de ter tudo, devolve -1
                if (opt == 'm'){
                        max_estafetas = atoi(optarg); // Converte o valor que leu em string e guarda na respetiva variavel
                        if (max_estafetas == 0)
                                max_estafetas = MAXESTAFETAS;
                }

                if (opt == 'p') {
                        prazo_aceita = atoi(optarg);
                        if (prazo_aceita == 0)
                                prazo_aceita = PRAZOACEITA;
                }
        }


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

        char *bindir_temp = getenv("BINDIR"); // Guarda o valor da variavel ambiente "BINDIR", caso nao encontre nada, retorna NULL
        char caminho_tarifador[200];

        if (bindir_temp != NULL) {
                // Se a variável existir, junta o caminho com "/tarifador" (ex: /home/joao/tarifador)
                snprintf(caminho_tarifador, sizeof(caminho_tarifador), "%s/tarifador", bindir_temp);
        } 
        
        else {
                // Se não existir, assume que está na mesma pasta do central
                snprintf(caminho_tarifador, sizeof(caminho_tarifador), "./tarifador");
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

                execl(caminho_tarifador, "tarifador", NULL);
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

        while (1)
        {
        
                printf("Comando > " );
                if(fgets(msg, sizeof(msg), stdin)==NULL)//EOF
                        break;

                char comando[50];
                
                //Le a partir de um array, lê msg -> comando
                sscanf(msg, "%49s", comando); // Lẽ a primeira palavra para identificar qual o comando

                if(strcasecmp(comando, "encerra")==0) {
                        printf("A encerrar a central\n");
                        break;
                }
                else if (strcasecmp(comando, "stats")==0) {
                        printf("Estatísticas:\n");
                        printf("Encomendas recebidas: %d\n", estatisticas.recebidos);
                        printf("Encomendas atribuídas: %d\n", estatisticas.atribuidos);
                        printf("Encomendas concluídas: %d\n", estatisticas.concluidos);
                        printf("Encomendas expiradas: %d\n", estatisticas.expirados);
                        printf("Encomendas perdidas: %d\n", estatisticas.perdidos);
                        printf("Valor total das encomendas: %.2f\n", estatisticas.valor_total);
                }


                // Comandos ainda não existentes 
                else if (strcasecmp(comando, "estafetas") == 0 ||
                        strcasecmp(comando, "pedidos") == 0 ||
                        strcasecmp(comando, "prazo") == 0) {
                                printf("O comando '%s' ainda não existe\n", comando);
                }

                else if (strcasecmp(comando, "pedido") == 0){
                        char origem[15];
                        char destino[15];
                        float peso;

                        // Se conseguir ler os 3 elementos corretamente, o formato está correto
                        if ((sscanf(msg, "%*s %49s %49s %f", origem, destino, &peso) != 3) || peso <= 0 || strlen(origem) > 15 || strlen(destino) > 15) { // Entra no if se o sscanf ler algo diferente do que está a espera
                                // %*s -> serve para ignorar a primeira palavra
                                
                                if (strlen(origem) > 15 || strlen(destino) > 15) { 
                                        printf("\nInsira uma origem/destino com até 15 caracteres\n");
                                        continue;
                                }
                                else {
                                        printf("Erro: Formato inválido!\n");
                                        printf("Uso correto: <origem> <destino> <peso>(ex: Lisboa Porto 15.5)\n\n");
                                        continue; // Ignora o resto do código e volta ao início do ciclo
                                }
                        }
                        
                        estatisticas.recebidos++; // Incrementa o numero do pedido

                        char msg_tarifador[50];
                        // Isto serve para criar uma string nova com o que o sscanf leu, ou seja, apenas a origem, destino e peso
                        snprintf(msg_tarifador, sizeof(msg_tarifador), "%s %s %.2f\n", origem, destino, peso); // FUnciona como o printf, só que guarda o valor numa variavel

                        if(write(pipe_c2t[1], msg_tarifador, strlen(msg_tarifador))==-1){
                                perror("errro na escrita para o stdin tarifador");
                                break;
                        }

                        int n = read(pipe_t2c[0], buf, sizeof(buf)-1); //sizeof(buf)-1 para ter espaço para o \0

                        if(n == -1){
                                perror("errro na leitura stdout tarifador");
                                break;
                        }
                        if(n == 0){
                                printf("O tarifador terminou\n");
                                break;
                        }
                        buf[n]='\0';
                        printf("Recebi: %s\n", buf);

                }

                else {
                        printf("Comando desconhecido: %s\n", comando);
                }
        }
        close(pipe_c2t[1]);
        close(pipe_t2c[0]);
        
        wait(&retproc);
        close(f);
        unlink(FICHEIRO_TMP);

        return 0;
}