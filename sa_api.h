#ifndef SA_API_H
#define SA_API_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define QUANTIDADE_BLOCOS 44
#define MARCADOR_FIM 65535
typedef struct {
    int numero_inodo;
    uint16_t posicao_atual;
} ARQUIVO;

void api_init(void);
void api_formata(void);

ARQUIVO *api_fopen(const char *nome, const char *modo);
int api_fclose(ARQUIVO *arquivo);

size_t api_fread(ARQUIVO *arquivo, uint8_t *buffer, size_t quantidade);
size_t api_fwrite(ARQUIVO *arquivo, const uint8_t *buffer, size_t quantidade);

int api_fseek(ARQUIVO *arquivo, long deslocamento, int origem);
long api_ftell(ARQUIVO *arquivo);
int api_feof(ARQUIVO *arquivo);

void lista_todos(void);

#endif
