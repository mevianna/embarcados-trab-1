#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <new>

#include "sa.h"
#include "sa_api.h"

#define TAMANHO_MAXIMO_ARQUIVO (TAMANHO_BLOCO + PONTEIROS_POR_BLOCO * TAMANHO_BLOCO)

void api_init(void)
{
    sa_init();
}

void api_formata(void)
{
    sa_formata();
}

ARQUIVO *api_fopen(const char *nome, const char *modo)
{
    if (nome == NULL || modo == NULL) return NULL;

    size_t tamanho_nome = strlen(nome);
    if (tamanho_nome == 0 || tamanho_nome > TAMANHO_NOME) return NULL;

    int modo_escrita = (strcmp(modo, "w") == 0);
    int modo_leitura_escrita = (strcmp(modo, "r+") == 0);
    if (!modo_escrita && !modo_leitura_escrita) return NULL;

    int indice_livre = -1;
    int indice_encontrado = -1;
    Tipo_INODO inodo;

    for (int i = 0; i < QUANTIDADE_INODOS; i++) {
        leiaInodo(i, &inodo);
        if (inodo.status == STATUS_OCUPADO &&
            strncmp(inodo.nome, nome, TAMANHO_NOME) == 0) {
            indice_encontrado = i;
            break;
        }
        if (inodo.status == STATUS_LIVRE && indice_livre == -1) indice_livre = i;
    }

    if (modo_escrita) {
        if (indice_encontrado >= 0) {
            leiaInodo(indice_encontrado, &inodo);
            sa_libera_conteudo(&inodo);
            memset(inodo.nome, 0, TAMANHO_NOME);
            memcpy(inodo.nome, nome, tamanho_nome);
            inodo.status = STATUS_OCUPADO;
            inodo.tamanho = 0;
            inodo.ponteiro_indireto = MARCADOR_FIM;
            escreveInodo(indice_encontrado, inodo);
        } else {
            if (indice_livre < 0) return NULL;
            memset(&inodo, 0, sizeof(inodo));
            memcpy(inodo.nome, nome, tamanho_nome);
            inodo.status = STATUS_OCUPADO;
            inodo.tamanho = 0;
            inodo.ponteiro_indireto = MARCADOR_FIM;
            escreveInodo(indice_livre, inodo);
            indice_encontrado = indice_livre;
        }
    } else if (indice_encontrado < 0) {
        return NULL;
    }

    ARQUIVO *arquivo = new (std::nothrow) ARQUIVO;
    if (arquivo == NULL) return NULL;
    arquivo->numero_inodo = indice_encontrado;
    arquivo->posicao_atual = 0;
    return arquivo;
}

int api_fclose(ARQUIVO *arquivo)
{
    if (arquivo == NULL) return -1;
    delete arquivo;
    return 0;
}

size_t api_fread(ARQUIVO *arquivo, uint8_t *buffer, size_t quantidade)
{
    if (arquivo == NULL || buffer == NULL) return 0;

    Tipo_INODO inodo;
    leiaInodo(arquivo->numero_inodo, &inodo);
    if (inodo.status != STATUS_OCUPADO || arquivo->posicao_atual >= inodo.tamanho) return 0;

    size_t disponivel = inodo.tamanho - arquivo->posicao_atual;
    if (quantidade > disponivel) quantidade = disponivel;

    size_t lidos = 0;
    while (lidos < quantidade) {
        uint16_t posicao = arquivo->posicao_atual;
        size_t quantidade_ler;

        if (posicao < TAMANHO_BLOCO) {
            quantidade_ler = TAMANHO_BLOCO - posicao;
            if (quantidade_ler > quantidade - lidos) quantidade_ler = quantidade - lidos;
            memcpy(buffer + lidos, inodo.bloco_direto + posicao, quantidade_ler);
        } else {
            uint16_t posicao_indireta = posicao - TAMANHO_BLOCO;
            uint16_t indice = posicao_indireta / TAMANHO_BLOCO;
            uint16_t deslocamento = posicao_indireta % TAMANHO_BLOCO;
            uint16_t ponteiros[PONTEIROS_POR_BLOCO];

            if (!sa_bloco_valido(inodo.ponteiro_indireto) || indice >= PONTEIROS_POR_BLOCO) break;
            sa_leia_ponteiros_indiretos(inodo.ponteiro_indireto, ponteiros);
            if (!sa_bloco_valido(ponteiros[indice])) break;

            quantidade_ler = TAMANHO_BLOCO - deslocamento;
            if (quantidade_ler > quantidade - lidos) quantidade_ler = quantidade - lidos;
            sa_leia_dados(ponteiros[indice], deslocamento, buffer + lidos, quantidade_ler);
        }

        arquivo->posicao_atual += quantidade_ler;
        lidos += quantidade_ler;
    }
    return lidos;
}

size_t api_fwrite(ARQUIVO *arquivo, const uint8_t *buffer, size_t quantidade)
{
    if (arquivo == NULL || buffer == NULL) return 0;
    if (arquivo->posicao_atual > TAMANHO_MAXIMO_ARQUIVO) return 0;
    if (quantidade > TAMANHO_MAXIMO_ARQUIVO - arquivo->posicao_atual) return 0;

    Tipo_INODO inodo;
    leiaInodo(arquivo->numero_inodo, &inodo);
    if (inodo.status != STATUS_OCUPADO) return 0;

    size_t escritos = 0;
    while (escritos < quantidade) {
        uint16_t posicao = arquivo->posicao_atual;
        size_t quantidade_escrever;

        if (posicao < TAMANHO_BLOCO) {
            quantidade_escrever = TAMANHO_BLOCO - posicao;
            if (quantidade_escrever > quantidade - escritos) quantidade_escrever = quantidade - escritos;
            memcpy(inodo.bloco_direto + posicao, buffer + escritos, quantidade_escrever);
        } else {
            uint16_t posicao_indireta = posicao - TAMANHO_BLOCO;
            uint16_t indice = posicao_indireta / TAMANHO_BLOCO;
            uint16_t deslocamento = posicao_indireta % TAMANHO_BLOCO;
            uint16_t ponteiros[PONTEIROS_POR_BLOCO];

            if (indice >= PONTEIROS_POR_BLOCO) break;

            if (!sa_bloco_valido(inodo.ponteiro_indireto)) {
                uint16_t novo_bloco;
                if (sa_aloca_bloco(&novo_bloco) != 0) break;
                inodo.ponteiro_indireto = novo_bloco;
                for (size_t i = 0; i < PONTEIROS_POR_BLOCO; i++) ponteiros[i] = MARCADOR_FIM;
                sa_escreva_ponteiros_indiretos(inodo.ponteiro_indireto, ponteiros);
            }

            sa_leia_ponteiros_indiretos(inodo.ponteiro_indireto, ponteiros);
            if (!sa_bloco_valido(ponteiros[indice])) {
                uint16_t novo_bloco;
                if (sa_aloca_bloco(&novo_bloco) != 0) break;
                ponteiros[indice] = novo_bloco;
                sa_escreva_ponteiros_indiretos(inodo.ponteiro_indireto, ponteiros);
            }

            quantidade_escrever = TAMANHO_BLOCO - deslocamento;
            if (quantidade_escrever > quantidade - escritos) quantidade_escrever = quantidade - escritos;
            sa_escreva_dados(ponteiros[indice], deslocamento, buffer + escritos, quantidade_escrever);
        }

        arquivo->posicao_atual += quantidade_escrever;
        escritos += quantidade_escrever;
    }

    if (arquivo->posicao_atual > inodo.tamanho) inodo.tamanho = arquivo->posicao_atual;
    escreveInodo(arquivo->numero_inodo, inodo);
    return escritos;
}

int api_fseek(ARQUIVO *arquivo, long deslocamento, int origem)
{
    if (arquivo == NULL) return -1;
    Tipo_INODO inodo;
    leiaInodo(arquivo->numero_inodo, &inodo);

    long nova_posicao;
    if (origem == SEEK_SET) nova_posicao = deslocamento;
    else if (origem == SEEK_CUR) nova_posicao = arquivo->posicao_atual + deslocamento;
    else if (origem == SEEK_END) nova_posicao = inodo.tamanho + deslocamento;
    else return -1;

    if (nova_posicao < 0 || nova_posicao > inodo.tamanho || nova_posicao > (long)TAMANHO_MAXIMO_ARQUIVO) return -1;
    arquivo->posicao_atual = nova_posicao;
    return 0;
}

long api_ftell(ARQUIVO *arquivo)
{
    if (arquivo == NULL) return -1;
    return arquivo->posicao_atual;
}

int api_feof(ARQUIVO *arquivo)
{
    if (arquivo == NULL) return 1;
    Tipo_INODO inodo;
    leiaInodo(arquivo->numero_inodo, &inodo);
    return arquivo->posicao_atual >= inodo.tamanho;
}

void lista_todos(void)
{
    Tipo_INODO inodo;
    printf("\nArquivos armazenados:\n");
    for (int i = 0; i < QUANTIDADE_INODOS; i++) {
        leiaInodo(i, &inodo);
        if (inodo.status == STATUS_OCUPADO) {
            char nome[TAMANHO_NOME + 1];
            memcpy(nome, inodo.nome, TAMANHO_NOME);
            nome[TAMANHO_NOME] = '\0';
            printf("Inode %d: nome=%s, tamanho=%u bytes\n", i, nome, (unsigned)inodo.tamanho);
        }
    }
}