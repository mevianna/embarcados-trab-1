#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "sa.h"
#include "memoria.h"

#define INICIO_PONTEIROS (TAMANHO_CABECALHO + QUANTIDADE_INODOS * TAMANHO_INODO)
#define INICIO_DADOS (INICIO_PONTEIROS + QUANTIDADE_BLOCOS * sizeof(uint16_t))
#define TAMANHO_MEMORIA_USADA (INICIO_DADOS + QUANTIDADE_BLOCOS * TAMANHO_BLOCO)

static int endereco_ponteiro(uint16_t bloco)
{
    return INICIO_PONTEIROS + bloco * sizeof(uint16_t);
}

static int endereco_dados(uint16_t bloco)
{
    return INICIO_DADOS + bloco * TAMANHO_BLOCO;
}

void sa_init(void)
{
    memoria_init();
}

void escreveCabec(Tipo_CABEC cabec)
{
    memoria_fwrite(&cabec, 0, TAMANHO_CABECALHO);
}

void leiaCabec(Tipo_CABEC *cabec)
{
    if (cabec != NULL) memoria_fread(cabec, 0, TAMANHO_CABECALHO);
}

void escreveInodo(int numero, Tipo_INODO inodo)
{
    if (numero < 0 || numero >= QUANTIDADE_INODOS) return;

    uint16_t posicao = TAMANHO_CABECALHO + numero * TAMANHO_INODO;

    memoria_fwrite(&inodo.status, posicao, sizeof(inodo.status));
    posicao += sizeof(inodo.status);

    memoria_fwrite(inodo.nome, posicao, TAMANHO_NOME);
    posicao += TAMANHO_NOME;

    memoria_fwrite(&inodo.tamanho, posicao, sizeof(inodo.tamanho));
    posicao += sizeof(inodo.tamanho);

    memoria_fwrite(inodo.bloco_direto, posicao, TAMANHO_BLOCO);
    posicao += TAMANHO_BLOCO;

    memoria_fwrite(&inodo.ponteiro_indireto, posicao, sizeof(inodo.ponteiro_indireto));
}

void leiaInodo(int numero, Tipo_INODO *inodo)
{
    if (numero < 0 || numero >= QUANTIDADE_INODOS || inodo == NULL) return;

    uint16_t posicao = TAMANHO_CABECALHO + numero * TAMANHO_INODO;

    memoria_fread(&inodo->status, posicao, sizeof(inodo->status));
    posicao += sizeof(inodo->status);

    memoria_fread(inodo->nome, posicao, TAMANHO_NOME);
    posicao += TAMANHO_NOME;

    memoria_fread(&inodo->tamanho, posicao, sizeof(inodo->tamanho));
    posicao += sizeof(inodo->tamanho);

    memoria_fread(inodo->bloco_direto, posicao, TAMANHO_BLOCO);
    posicao += TAMANHO_BLOCO;
    
    memoria_fread(&inodo->ponteiro_indireto, posicao, sizeof(inodo->ponteiro_indireto));
}

int sa_bloco_valido(uint16_t bloco)
{
    return bloco < QUANTIDADE_BLOCOS;
}

static void leia_proximo_bloco(uint16_t bloco, uint16_t *proximo)
{
    if (proximo == NULL || !sa_bloco_valido(bloco)) return;
    memoria_fread(proximo, endereco_ponteiro(bloco), sizeof(*proximo));
}

int sa_aloca_bloco(uint16_t *bloco)
{
    if (bloco == NULL) return -1;
    Tipo_CABEC cabec;
    uint16_t proximo;
    leiaCabec(&cabec);
    if (cabec.primeiro_bloco_livre == MARCADOR_FIM || !sa_bloco_valido(cabec.primeiro_bloco_livre)) return -1;
    *bloco = cabec.primeiro_bloco_livre;
    leia_proximo_bloco(*bloco, &proximo);
    cabec.primeiro_bloco_livre = proximo;
    escreveCabec(cabec);
    return 0;
}

int sa_libera_bloco(uint16_t bloco)
{
    if (!sa_bloco_valido(bloco)) return -1;
    Tipo_CABEC cabec;
    leiaCabec(&cabec);
    memoria_fwrite(&cabec.primeiro_bloco_livre, endereco_ponteiro(bloco), sizeof(cabec.primeiro_bloco_livre));
    cabec.primeiro_bloco_livre = bloco;
    escreveCabec(cabec);
    return 0;
}

void sa_leia_ponteiros_indiretos(uint16_t bloco, uint16_t ponteiros[PONTEIROS_POR_BLOCO])
{
    if (ponteiros == NULL) return;
    for (size_t i = 0; i < PONTEIROS_POR_BLOCO; i++) ponteiros[i] = MARCADOR_FIM;
    if (sa_bloco_valido(bloco)) memoria_fread(ponteiros, endereco_dados(bloco), TAMANHO_BLOCO);
}

void sa_escreva_ponteiros_indiretos(uint16_t bloco, const uint16_t ponteiros[PONTEIROS_POR_BLOCO])
{
    if (!sa_bloco_valido(bloco) || ponteiros == NULL) return;
    memoria_fwrite((void *)ponteiros, endereco_dados(bloco), TAMANHO_BLOCO);
}

void sa_leia_dados(uint16_t bloco, uint16_t deslocamento, void *buffer, uint16_t quantidade)
{
    if (!sa_bloco_valido(bloco) || buffer == NULL || deslocamento >= TAMANHO_BLOCO) return;
    if (quantidade > TAMANHO_BLOCO - deslocamento) quantidade = TAMANHO_BLOCO - deslocamento;
    memoria_fread(buffer, endereco_dados(bloco) + deslocamento, quantidade);
}

void sa_escreva_dados(uint16_t bloco, uint16_t deslocamento, const void *buffer, uint16_t quantidade)
{
    if (!sa_bloco_valido(bloco) || buffer == NULL || deslocamento >= TAMANHO_BLOCO) return;
    if (quantidade > TAMANHO_BLOCO - deslocamento) quantidade = TAMANHO_BLOCO - deslocamento;
    memoria_fwrite((void *)buffer, endereco_dados(bloco) + deslocamento, quantidade);
}

void sa_libera_conteudo(Tipo_INODO *inodo)
{
    if (inodo == NULL) return;
    uint16_t ponteiros[PONTEIROS_POR_BLOCO];
    if (sa_bloco_valido(inodo->ponteiro_indireto)) {
        sa_leia_ponteiros_indiretos(inodo->ponteiro_indireto, ponteiros);
        for (size_t i = 0; i < PONTEIROS_POR_BLOCO; i++) {
            if (ponteiros[i] == MARCADOR_FIM) break;
            if (sa_bloco_valido(ponteiros[i])) sa_libera_bloco(ponteiros[i]);
        }
        sa_libera_bloco(inodo->ponteiro_indireto);
    }
    inodo->ponteiro_indireto = MARCADOR_FIM;
    inodo->tamanho = 0;
    memset(inodo->bloco_direto, 0, TAMANHO_BLOCO);
}

void sa_formata(void)
{
    if (TAMANHO_MEMORIA_USADA > 2048) {
        printf("Erro: estrutura do sistema de arquivos excede a memoria configurada.\n");
        return;
    }
    Tipo_CABEC cabec = {};
    cabec.primeiro_bloco_livre = 0;
    escreveCabec(cabec);
    Tipo_INODO inodo = {};
    for (int i = 0; i < QUANTIDADE_INODOS; i++) {
        inodo.status = STATUS_LIVRE;
        memset(inodo.nome, 0, TAMANHO_NOME);
        inodo.tamanho = 0;
        memset(inodo.bloco_direto, 0, TAMANHO_BLOCO);
        inodo.ponteiro_indireto = MARCADOR_FIM;
        escreveInodo(i, inodo);
    }
    for (uint16_t i = 0; i < QUANTIDADE_BLOCOS; i++) {
        uint16_t proximo = (i == QUANTIDADE_BLOCOS - 1) ? MARCADOR_FIM : i + 1;
        memoria_fwrite(&proximo, endereco_ponteiro(i), sizeof(proximo));
    }
    printf("Formatacao concluida!\n");
}