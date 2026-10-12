#ifndef SA_H
#define SA_H

#include <stdint.h>

#define QUANTIDADE_INODOS 12
#define TAMANHO_NOME 8
#define TAMANHO_BLOCO 32

#define STATUS_LIVRE 0
#define STATUS_OCUPADO 1

#define TAMANHO_CABECALHO 2
#define TAMANHO_INODO 45 // 8 + 1 + 2 + 32 + 2 = 45 bytes

typedef struct {
    uint16_t primeiro_bloco_livre;
}Tipo_CABEC;

typedef struct {
    char nome[TAMANHO_NOME];
    uint8_t status;
    uint16_t tamanho;
    uint8_t bloco_direto[TAMANHO_BLOCO];
    uint16_t ponteiro_indireto;
} Tipo_INODO;

void escreveCabec(Tipo_CABEC cabec);
void leiaCabec(Tipo_CABEC *cabec);

void escreveInodo(int numero, Tipo_INODO inodo);
void leiaInodo(int numero, Tipo_INODO *inodo);

#endif