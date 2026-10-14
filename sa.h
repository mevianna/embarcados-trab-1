#ifndef SA_H
#define SA_H

#include <stdint.h>
#include <stddef.h>

#define QUANTIDADE_INODOS 12
#define TAMANHO_NOME 8
#define TAMANHO_BLOCO 32
#define STATUS_LIVRE 0
#define STATUS_OCUPADO 1
#define TAMANHO_CABECALHO 2
#define TAMANHO_INODO 45
#define QUANTIDADE_BLOCOS 44
#define MARCADOR_FIM 65535
#define PONTEIROS_POR_BLOCO (TAMANHO_BLOCO / sizeof(uint16_t))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t primeiro_bloco_livre;
} Tipo_CABEC;

typedef struct {
    uint8_t status;
    char nome[TAMANHO_NOME];
    uint16_t tamanho;
    uint8_t bloco_direto[TAMANHO_BLOCO];
    uint16_t ponteiro_indireto;
} Tipo_INODO;

/* Inicialização e formatação da camada de sistema de arquivos. */
void sa_init(void);
void sa_formata(void);

/* Acesso ao cabeçalho e aos inodes. */
void escreveCabec(Tipo_CABEC cabec);
void leiaCabec(Tipo_CABEC *cabec);
void escreveInodo(int numero, Tipo_INODO inodo);
void leiaInodo(int numero, Tipo_INODO *inodo);

/* Operações internas de gerenciamento de blocos. */
int sa_bloco_valido(uint16_t bloco);
int sa_aloca_bloco(uint16_t *bloco);
int sa_libera_bloco(uint16_t bloco);
void sa_leia_ponteiros_indiretos(uint16_t bloco, uint16_t ponteiros[PONTEIROS_POR_BLOCO]);
void sa_escreva_ponteiros_indiretos(uint16_t bloco, const uint16_t ponteiros[PONTEIROS_POR_BLOCO]);
void sa_libera_conteudo(Tipo_INODO *inodo);

/* Acesso aos blocos de dados, sem expor endereços físicos à API. */
void sa_leia_dados(uint16_t bloco, uint16_t deslocamento, void *buffer, uint16_t quantidade);
void sa_escreva_dados(uint16_t bloco, uint16_t deslocamento, const void *buffer, uint16_t quantidade);

#ifdef __cplusplus
}
#endif

#endif