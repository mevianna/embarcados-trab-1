#include <stdio.h>
#include <string.h>
#include "sa_api.h"

int main(void)
{
    char buffer[70], a;
    ARQUIVO *A, *B, *C;

    printf("Tecle [ENTER] para iniciar\r");
    while (1)
    {
        a = getchar();
        if (a == 13) break;
    }
    printf("OK \r");

    api_init();
    api_formata(); 

    A = api_fopen("UFSC", "w");
    strcpy(buffer, "Oi mundo, teste de mensagemXXX");
    api_fwrite(A, (uint8_t *)buffer, strlen(buffer));
    api_fclose(A);

    B = api_fopen("Fabio", "w");
    strcpy(buffer, "Oi mundo, teste de mensagemXXX");
    api_fwrite(B, (uint8_t *)buffer, strlen(buffer));

    strcpy(buffer, " Prof: FABIO");
    api_fwrite(B, (uint8_t *)buffer, strlen(buffer));

    strcpy(buffer, " de la Rocha UFSC Ararangua");
    api_fwrite(B, (uint8_t *)buffer, strlen(buffer));
    api_fclose(B);

    C = api_fopen("Fabio", "r+");
    api_fseek(C, 19, SEEK_SET);
    api_fread(C, (uint8_t *)buffer, 8);
    buffer[8] = 0;
    printf("Lido=%s\r", buffer);

    lista_todos();

    while (1)
    {
    }

    return 0;
}