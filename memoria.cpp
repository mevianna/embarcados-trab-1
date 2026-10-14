#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "driver/i2c_master.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <unistd.h>
#include <stdio.h>

#include "memoria.h"

// O pino 22 é o pino de CLOCK e 21 é o de DADOS
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

#define EEPROM_ADDR 0x50
#define EEPROM_SIZE 2048

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t eeprom_handle[8];

void iniciaI2C()
{
    i2c_master_bus_config_t bus_config = {};

    bus_config.i2c_port = I2C_NUM_0;
    bus_config.sda_io_num = I2C_SDA;
    bus_config.scl_io_num = I2C_SCL;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(&bus_config, &bus_handle)
    );
}

void adicionaEEPROM()
{
    for (int i = 0; i < 8; i++)
    {
        i2c_device_config_t dev_cfg = {};

        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = EEPROM_ADDR + i;
        dev_cfg.scl_speed_hz = 100000;
        dev_cfg.scl_wait_us = 0;

        ESP_ERROR_CHECK(
            i2c_master_bus_add_device(
                bus_handle,
                &dev_cfg,
                &eeprom_handle[i]
            )
        );
    }
}

i2c_master_dev_handle_t selecionaEEPROM(uint16_t endereco)
{
    int bloco = endereco >> 8;

    return eeprom_handle[bloco];
}

void aguardaEEPROM(uint16_t endereco)
{
    int bloco = endereco >> 8;

    uint8_t endereco_i2c = EEPROM_ADDR + bloco;

    while (
        i2c_master_probe(
            bus_handle,
            endereco_i2c,
            -1
        ) != ESP_OK
    )
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void escreveByte(uint16_t endereco, uint8_t valor)
{
    if (endereco >= EEPROM_SIZE)
        return;

    uint8_t dados[2];

    dados[0] = endereco & 0xFF;
    dados[1] = valor;

    i2c_master_dev_handle_t dev =
        selecionaEEPROM(endereco);

    ESP_ERROR_CHECK(
        i2c_master_transmit(
            dev,
            dados,
            sizeof(dados),
            -1
        )
    );

    aguardaEEPROM(endereco);
}


	

uint8_t leByte(uint16_t endereco)
{
    if (endereco >= EEPROM_SIZE)
        return 0;

    uint8_t endereco_i2c;

    endereco_i2c = endereco & 0xFF;

    uint8_t resultado = 0;

    i2c_master_dev_handle_t dev =
        selecionaEEPROM(endereco);

    ESP_ERROR_CHECK(
        i2c_master_transmit_receive(
            dev,
            &endereco_i2c,
            1,
            &resultado,
            1,
            -1
        )
    );

    return resultado;
}

void escreveBuffer(
    uint16_t endereco,
    const char *b,
    int size
)
{
    for (int x = 0; x < size; x++)
    {
        if ((endereco + x) >= EEPROM_SIZE)
            break;

        escreveByte(
            endereco + x,
            b[x]
        );
    }
}

void leBuffer(
    uint16_t endereco,
    int size,
    char *b
)
{
    int pos = 0;

    while (pos < size)
    {
        uint16_t endereco_atual = endereco + pos;

        if (endereco_atual >= EEPROM_SIZE)
            break;

        int restante_bloco =
            256 - (endereco_atual & 0xFF);

        int quantidade = size - pos;

        if (quantidade > restante_bloco)
            quantidade = restante_bloco;

        uint8_t endereco_i2c =
            endereco_atual & 0xFF;

        i2c_master_dev_handle_t dev =
            selecionaEEPROM(endereco_atual);

        ESP_ERROR_CHECK(
            i2c_master_transmit_receive(
                dev,
                &endereco_i2c,
                1,
                (uint8_t *)&b[pos],
                quantidade,
                -1
            )
        );

        pos += quantidade;
    }
}

void memoria_init()
{
    iniciaI2C();
    adicionaEEPROM();
}

void memoria_fwrite(void *buffer, uint16_t posicao, uint16_t tam)
{
    escreveBuffer(posicao, (char *)buffer, tam);
}

void memoria_fread(void *buffer, uint16_t posicao, uint16_t tam)
{
    leBuffer(posicao, tam, (char *)buffer);
}
