#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "memoria.h"
#include "sa.h"
#include "sa_api.h"

void api_init(void)
{
    memoria_init();
};

void api_formata(void)
{
    Tipo_CABEC cabec;
    cabec.primeiro_bloco_livre = 0;
    escreveCabec(cabec);

    Tipo_INODO inodo = {};

    for (int i = 0; i < QUANTIDADE_INODOS; i++)
    {
        inodo.status = STATUS_LIVRE;
        memset(inodo.nome, 0, TAMANHO_NOME);
        inodo.tamanho = 0;
        memset(inodo.bloco_direto, 0, TAMANHO_BLOCO);
        inodo.ponteiro_indireto = MARCADOR_FIM;

        escreveInodo(i, inodo);
    }

    uint16_t proximo_bloco;

    for (uint16_t i = 0; i < QUANTIDADE_BLOCOS; i++)
    {
        if (i == QUANTIDADE_BLOCOS - 1)
        {
            proximo_bloco = MARCADOR_FIM;
        }
        else
        {
            proximo_bloco = i + 1;
        }

        uint16_t endereco = TAMANHO_CABECALHO
                          + QUANTIDADE_INODOS * TAMANHO_INODO
                          + i * sizeof(uint16_t);

        memoria_fwrite(
            &proximo_bloco,
            endereco,
            sizeof(proximo_bloco)
        );
    }
}
