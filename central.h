#ifndef CENTRAL_H
#define CENTRAL_H

#include "comum.h"

/* Constantes específicas da central */
#define FICHEIRO_TMP   "/tmp/central"   /* garante instância única */
#define NOME_TARIFADOR "tarifador"
#define TAM_LINHA      256              /* tamanho máximo de um comando do administrador */
#define TAM_CAMINHO    512

typedef struct {
    int  max_estafetas;                 /* MAXESTAFETAS / -m */
    int  prazo_aceita;                  /* PRAZOACEITA  / -p (segundos) */
    char bindir[TAM_CAMINHO];           /* BINDIR: diretoria do tarifador */
} Config;

typedef struct {
    int    recebidos, atribuidos, concluidos, expirados, perdidos;
    double valor_concluidos;            /* valor total das entregas concluídas */
} Stats;

#endif
