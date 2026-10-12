#include "sa.h"
#include "memoria.h"

void escreveCabec(Tipo_CABEC cabec)
{
    memoria_fwrite(
        (void *)&cabec,
        0,
        TAMANHO_CABECALHO
    );
}

void leiaCabec(Tipo_CABEC *cabec)
{
    memoria_fread(
        (void *)cabec,
        0,
        TAMANHO_CABECALHO
    );
}

void escreveInodo(int numero, Tipo_INODO inodo)
{
    int posicao = TAMANHO_CABECALHO
                 + numero * TAMANHO_INODO;

    memoria_fwrite(
        inodo.nome,
        posicao,
        TAMANHO_NOME
    );
    posicao += TAMANHO_NOME;

    memoria_fwrite(
        &inodo.status,
        posicao,
        sizeof(uint8_t)
    );
    posicao += sizeof(uint8_t);

    memoria_fwrite(
        &inodo.tamanho,
        posicao,
        sizeof(uint16_t)
    );
    posicao += sizeof(uint16_t);

    memoria_fwrite(
        inodo.bloco_direto,
        posicao,
        TAMANHO_BLOCO
    );
    posicao += TAMANHO_BLOCO;

    memoria_fwrite(
        &inodo.ponteiro_indireto,
        posicao,
        sizeof(uint16_t)
    );
}

void leiaInodo(int numero, Tipo_INODO *inodo)
{
    int posicao = TAMANHO_CABECALHO
                 + numero * TAMANHO_INODO;

    memoria_fread(
        inodo->nome,
        posicao,
        TAMANHO_NOME
    );
    posicao += TAMANHO_NOME;

    memoria_fread(
        &inodo->status,
        posicao,
        sizeof(uint8_t)
    );
    posicao += sizeof(uint8_t);

    memoria_fread(
        &inodo->tamanho,
        posicao,
        sizeof(uint16_t)
    );
    posicao += sizeof(uint16_t);

    memoria_fread(
        inodo->bloco_direto,
        posicao,
        TAMANHO_BLOCO
    );
    posicao += TAMANHO_BLOCO;

    memoria_fread(
        &inodo->ponteiro_indireto,
        posicao,
        sizeof(uint16_t)
    );
}