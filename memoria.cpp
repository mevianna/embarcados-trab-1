#include <stdio.h>
#include "memoria.h"

FILE *f;

void memoria_init()
{
    printf("Chamou a versao de mem.init do linux\n");
    printf("Cria um arquivo para emular a memoria\n");
    f = fopen ("PARTICAO.BIN","r+");
if (f==NULL)
{
f = fopen ("PARTICAO.BIN","w+");
if (f==NULL)
{
printf("Impossivel criar aquivo de particao\n");
}
}
}

void memoria_fwrite(void *buffer, uint16_t posicao, uint16_t tam)
{
printf("Chamou a versao de mem.write do linux\n");
fseek(f, posicao, SEEK_SET);
fwrite(buffer, tam, 1, f);
}
void memoria_fread(void *buffer, uint16_t posicao, uint16_t tam)
{
printf("Chamou a versao de mem.read do linux\n");
fseek(f, posicao, SEEK_SET);
fread(buffer, tam, 1, f);
}
