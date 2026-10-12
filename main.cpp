
#include <stdio.h>
#include "sa_api.h"
#include "sa.h"
#include "memoria.h"

int main(void)
{
    printf("Inicializando o sistema de arquivos...\n");
    api_init();

    printf("Formatando o sistema de arquivos...\n");
    api_formata();


    return 0;
}
