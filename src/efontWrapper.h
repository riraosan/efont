
#pragma once

#define TFT_ESPI
#define EFONTENABLEJAMINI

#if defined(ADAFRUIT_GFX)
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
using GraphicsLib = Adafruit_GFX;
#endif

#if defined(TFT_ESPI)
#include <utility/In_eSPI.h>
using GraphicsLib = TFT_eSPI;
#endif

#if defined(EFONTENABLEALL)
#include <efontEnableAll.h>
#endif
#if defined(EFONTENABLEASCII)
#include <efontEnableAscii.h>
#endif
#if defined(EFONTENABLECJK)
#include <efontEnableCJK.h>
#endif
#if defined(EFONTENABLECN)
#include <efontEnableCn.h>
#endif
#if defined(EFONTENABLEJA)
#include "efontEnableJa.h"
#elif defined(EFONTENABLEJAMINI)
#include <efontEnableJaMini.h>
#endif
#if defined(EFONTENABLEKR)
#include <efontEnableKr.h>
#endif
#if defined(EFONTENABLETW)
#include <efontEnableTw.h>
#endif

class EfontWrapper : public GraphicsLib {
   public:
    EfontWrapper();   
    EfontWrapper(int16_t w, int16_t h);
    ~EfontWrapper(void);

    void printEfont(const char *str);
    void printEfont(const char *str, int x, int y);
    void printEfont(const char *str, int x, int y, uint8_t textsize);

   private:
    const char *_efontUFT8toUTF16(uint16_t *pUTF16, const char *pUTF8);
    void _getefontData(uint8_t *font, uint16_t fontUTF16);
};

#include <Arduino.h>
#include <efontWrapper.h>
#include <esp32-hal-log.h>
#include <efont.h>

EfontWrapper::EfontWrapper(int16_t w, int16_t h) : GraphicsLib(w, h) {}

EfontWrapper::EfontWrapper() : GraphicsLib() {}

EfontWrapper::~EfontWrapper(void) {}

void EfontWrapper::printEfont(const char *str, int x, int y) {
    setCursor(x, y);
    printEfont(str);
}

void EfontWrapper::printEfont(const char *str, int x, int y, uint8_t textsize) {
    setCursor(x, y);
    setTextSize(textsize);
    printEfont(str);
}

const char *EfontWrapper::_efontUFT8toUTF16(uint16_t *pUTF16, const char *pUTF8) {
    return efontUFT8toUTF16(pUTF16, pUTF8);
}

void EfontWrapper::_getefontData(uint8_t *font, uint16_t fontUTF16) {
    getefontData(font, fontUTF16);
}

void EfontWrapper::printEfont(const char *str) {
    int posX = getCursorX();
    int posY = getCursorY();
#if defined(ADAFRUIT_GFX)
    uint8_t textsize = GraphicsLib::textsize_y;
#elif defined(TFT_ESPI)
    uint8_t textsize = GraphicsLib::textsize;
#endif
    uint32_t textcolor   = GraphicsLib::textcolor;
    uint32_t textbgcolor = GraphicsLib::textbgcolor;

    byte font[32] = {0};

    while (*str != 0x00) {
        // ���s����
        if (*str == '\n') {
            // ���s
            posY += 16 * textsize;
            posX = getCursorX();
            str++;
            continue;
        }

        // �t�H���g�擾
        uint16_t strUTF16;
        str = efontUFT8toUTF16(&strUTF16, str);
        getefontData(font, strUTF16);

        // ��������
        int width = 16 * textsize;
        if (strUTF16 < 0x0100) {
            // ���p
            width = 8 * textsize;
        }
#ifdef EFONT_DEBUG
        log_printf("str : U+%04X\n", strUTF16);
#endif

        // �w�i�h��Ԃ�
        fillRect(posX, posY, width, 16 * textsize, textbgcolor);

        // �擾�t�H���g�̊m�F
        for (uint8_t row = 0; row < 16; row++) {
            word fontdata = font[row * 2] * 256 + font[row * 2 + 1];
            for (uint8_t col = 0; col < 16; col++) {
#ifdef EFONT_DEBUG
                log_printf("%s", ((0x8000 >> col) & fontdata) ? "#" : " ");
#endif
                if ((0x8000 >> col) & fontdata) {
                    int drawX = posX + col * textsize;
                    int drawY = posY + row * textsize;
                    if (textsize == 1) {
                        drawPixel(drawX, drawY, textcolor);
                    } else {
                        fillRect(drawX, drawY, textsize, textsize, textcolor);
                    }
                }
            }
#ifdef EFONT_DEBUG
            log_printf("\n");
#endif
        }

        // �`��J�[�\����i�߂�
        posX += width;

        // �ܕԂ�����
        if (GraphicsLib::width() <= posX) {
            posX = 0;
            posY += 16 * textsize;
        }
    }

    // �J�[�\�����X�V
    setCursor(posX, posY);
}
