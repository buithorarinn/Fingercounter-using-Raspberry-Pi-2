#include <wiringPiI2C.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "oled_display.h"

#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

int fd;

const uint8_t init_cmds[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F,
    0xD3, 0x00, 0x40, 0x8D, 0x14,
    0x20, 0x00, 0xA1, 0xC8, 0xDA,
    0x12, 0x81, 0xCF, 0xD9, 0xF1,
    0xDB, 0x40, 0xA4, 0xA6, 0xAF};

const uint8_t font[96][5] = {
    // 5x7 ASCII font (add your characters or use a full font array)
};

void oled_cmd(uint8_t cmd)
{
    wiringPiI2CWriteReg8(fd, 0x00, cmd);
}

void oled_data(uint8_t data)
{
    wiringPiI2CWriteReg8(fd, 0x40, data);
}

bool oled_init()
{
    fd = wiringPiI2CSetup(OLED_ADDR);
    if (fd < 0)
        return false;

    for (uint8_t cmd : init_cmds)
        oled_cmd(cmd);

    oled_clear();
    return true;
}

void oled_clear()
{
    for (int page = 0; page < 8; page++)
    {
        oled_cmd(0xB0 + page);
        oled_cmd(0x00);
        oled_cmd(0x10);
        for (int i = 0; i < OLED_WIDTH; i++)
        {
            oled_data(0x00);
        }
    }
}

void oled_show_message(const std::string &msg)
{
    oled_clear();
    int page = 0;
    int col = 0;

    for (char c : msg)
    {
        if (c < 32 || c > 127)
            c = '?';
        const uint8_t *glyph = font[c - 32];

        oled_cmd(0xB0 + page);
        oled_cmd(0x00 + (col & 0x0F));
        oled_cmd(0x10 + (col >> 4));

        for (int i = 0; i < 5; i++)
        {
            oled_data(glyph[i]);
        }
        oled_data(0x00); // Space between characters

        col += 6;
        if (col > 120)
        {
            col = 0;
            page++;
            if (page > 7)
                break;
        }
    }
}
