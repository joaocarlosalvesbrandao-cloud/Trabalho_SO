#ifndef COMUM_H
#define COMUM_H
#include <sys/types.h>

/* Limites do enunciado */
#define TAM_TIPO         12
#define TAM_CORPO        50
#define TAM_NOME         16      // origem/destino: 15 chars + '\0' 
#define TAM_USERNAME     32
#define TAM_PIPE         64
#define LIMITE_ESTAFETAS 20
#define LIMITE_PEDIDOS   30

/* Valores por omissão (7.2) */
#define DEF_INTMIN       2
#define DEF_INTMAX       10
#define DEF_MAXESTAFETAS 10
#define DEF_PRAZOACEITA  30
#define DEF_BINDIR       "."

/* Nomes das variáveis de ambiente */
#define ENV_INTMIN       "INTMIN"
#define ENV_INTMAX       "INTMAX"
#define ENV_MAXESTAFETAS "MAXESTAFETAS"
#define ENV_PRAZOACEITA  "PRAZOACEITA"
#define ENV_BINDIR       "BINDIR"
#define ENV_CENTRAL_PIPE "CENTRAL_PIPE"

/* Identificadores de tipo (primeiros 12 bytes de TODAS as mensagens) */
#define TIPO_PEDIDO      "pedido"       /* gerador  -> central  */
#define TIPO_REGISTO     "RegistoEst"   /* estafeta -> central  */
#define TIPO_ACEITA      "aceita"       /* estafeta -> central  */
#define TIPO_ENTREGUE    "entregue"
#define TIPO_ESTADO      "estado"
#define TIPO_GANHOS      "ganhos"
#define TIPO_SAI         "sai"
#define TIPO_ANUNCIO     "anuncio"      /* central  -> estafeta */
#define TIPO_RESPOSTA    "resposta"
#define TIPO_ENCERRA     "encerra"

/* Todas as structs começam por char tipo[TAM_TIPO] */
typedef struct {                /* formato OBRIGATÓRIO (4.1) */
    char tipo[TAM_TIPO];
    char corpo[TAM_CORPO];
} MsgPedido;

typedef struct {
    char  tipo[TAM_TIPO];
    char  username[TAM_USERNAME];
    pid_t pid;
    char  pipe_cmd[TAM_PIPE];   /* estafeta -> central */
    char  pipe_resp[TAM_PIPE];  /* central  -> estafeta */
} MsgRegistoEst;

typedef struct {                //aceita / entregue / estado / ganhos / sai xx
    char tipo[TAM_TIPO];
    int  id_pedido;             //só usado em "aceita" 
} MsgComandoEst;

typedef struct {
    char   tipo[TAM_TIPO];
    int    id;
    char   origem[TAM_NOME], destino[TAM_NOME];
    double peso, valor;
    int    segundos_restantes;
} MsgAnuncio;

typedef struct {
    char tipo[TAM_TIPO];
    int  codigo;                
    char texto[128];
} MsgResposta;

#endif