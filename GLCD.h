#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "FONT.h"

#define _XTAL_FREQ 32000000

// pin

#define DC LATDbits.LD1
#define CS LATEbits.LE1

// decide command & data

#define COMMAND 0
#define DATA 1

// GLCD's width & height

#define WIDTH 320
#define HEIGHT 240

// color

#define BROWN		0xF5A0
#define WHITE       0xFFFF
#define SILVER      0xC618
#define BLACK       0x0000
#define GREY        0x8410

#define RED         0xF800
#define MAROON      0x8000
#define FUCHSIA     0xF81F		
#define PURPLE      0x8010

#define LIME        0x07E0
#define GREEN       0x0400
#define YELLOW      0xFFE0
#define OLIVE       0x8400

#define BLUE        0x001F
#define NAVY        0x0010
#define AQUA        0x07FF
#define TEAL        0x0410

#define DARK_BLUE   0x0002

// level 1 command

#define GLCD_NOP     0x00
#define SWRESET 0x01
#define RDDIDIF   0x04
#define RDDST   0x09
#define RDDPM	0x0A
#define RDDMADCTL	0x0B
#define RDDCOLMOD	0x0C
#define RDDIM	0x0D
#define RDDSM	0x0E
#define RDDSDR	0x0F
#define SPLIN	0x10
#define SLPOUT	0x11
#define PTLON	0x12
#define NORON	0x13
#define DINVOFF	0x20
#define DINVON	0x21
#define GAMSET	0x26
#define DISPOFF	0x28
#define DISPON	0x29
#define CASET	0x2A
#define PASET	0x2B

#define RAMWR	0x2C
#define RGBSET	0x2D
#define RAMRD	0x2E
#define PLTAR	0x30
#define VSCRDEF	0x33
#define TEOFF	0x34
#define TEON	0x35
#define MADCTL	0x36
#define VSCRSADD	0x37
#define IDMOFF	0x38
#define IDMON	0x39
#define PIXSET	0x3A
#define WMC	0x3c
#define RMC	0x3E
#define STS 0x44
#define GS	0x45
#define WRDISBV	0x51
#define RDDISBV	0x52
#define WRCTRLD	0x53
#define RDCTRLD	0x54
#define WRCABC	0x55
#define RDCABC	0x56
#define RDID1	0xDA
#define RDID2	0xDB
#define RDID3	0xDC

// level 2 command

#define IFMODE	0xB0
#define FRMCTR1	0xB1
#define FRMCTR2	0xB2
#define FRMCTR3	0xB3
#define INVTR	0xB4
#define PRCTR	0xB5
#define DISCTRL	0xB6
#define ETMOD	0xB7

#define PWCTRL1	0xC0
#define PWCTRL2	0xC1
#define VMCTRL1	0xC5
#define VMCTRL2 0xC7
#define NVMWR	0xD0
#define NVMPKEY	0xD1
#define RDNVM	0xD2
#define RDID4	0xD3
#define PGAMCTRL	0xE0
#define NGAMCTRL	0xE1
#define DGAMCTRL1	0xE2
#define DGAMCTRL2	0xE3
#define IFCTL	0xF6

#define PWCTRLA	0xCB	// Power control A
#define PWCTRLB	0xCF	// Power control B

void msDelay (uint8_t ms);

void init_spi();

uint8_t send_spi(uint8_t byte);

void init_pic();

void send_glcd(bool select, uint8_t data);

void init_glcd();

void draw_pixel(uint16_t color);

void set_cursor(uint16_t x, uint16_t y);

void set_all_cursor(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void fill_all(uint16_t color);

void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void draw_vline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void draw_hline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void draw_filled_circle (uint16_t xm, uint16_t ym, uint8_t r, uint16_t color);

void draw_char(uint16_t x, uint16_t y, char c, uint16_t color);

void draw_string(uint16_t x, uint16_t y, char* s, uint16_t color);

void draw_rect(uint16_t x, uint16_t y, uint16_t dx, uint16_t dy, uint16_t color);

//void draw_picture(uint8_t image, uint16_t width, uint16_t height, uint16_t x, uint16_t y);
