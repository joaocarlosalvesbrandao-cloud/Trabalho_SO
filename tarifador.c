#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINHA 128

/* Le uma linha do fd 0 para buf (sem o '\n').
   Devolve: 1 = linha lida, 0 = EOF (stdin fechado), -1 = erro */
static int le_linha(char *buf, int max) {
    int n = 0;
    char c;
    while (n < max - 1) {
        ssize_t r = read(STDIN_FILENO, &c, 1);
        if (r == 0) return (n > 0) ? 1 : 0;   // EOF
        if (r < 0)  return -1;
        if (c == '\n') break;
        buf[n++] = c;
    }
    buf[n] = '\0';
    return 1;
}

/* TODO: a formula e livre. Ex.: distancia derivada do tamanho dos
   nomes, valor = base + km * taxa + peso * taxa_peso */
static void calcula(const char *orig, const char *dest, double peso,
                    double *km, double *euros) {
    *km = 0.0;     // TODO
    *euros = 0.0;  // TODO
}

int main(void) {
    char linha[MAX_LINHA], orig[16], dest[16], resp[MAX_LINHA];
    double peso, km, euros;
    int r;

    while ((r = le_linha(linha, sizeof linha)) == 1) {
        if (sscanf(linha, "%15s %15s %lf", orig, dest, &peso) != 3) {
            // TODO: decidir o que responder a uma linha invalida
            // (ex.: "-1 -1" para a central detetar o erro)
            snprintf(resp, sizeof resp, "-1 -1\n");
        } else {
            calcula(orig, dest, peso, &km, &euros);
            snprintf(resp, sizeof resp, "%.1f %.2f\n", km, euros);
        }
        write(STDOUT_FILENO, resp, strlen(resp));  // sem buffer: sai logo
    }
    return 0;   // EOF => termina
}
