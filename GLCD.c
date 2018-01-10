#include "GLCD.h"

void msDelay (uint8_t ms){
    
    uint8_t i;    
    for(i = 0; i < ms; i++)
        __delay_ms(1);
}

void init_spi(){
	
	SSPCON1 = 0b00100010;
	SSPSTATbits.CKE = 0;
	SSPCON1bits.SSPEN = 1;
}

uint8_t send_spi(uint8_t byte){
	
		SSPBUF = byte;
		while(!SSPSTATbits.BF);
       return SSPBUF;
}

void init_pic(){
	
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
	
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;

	OSCCONbits.IRCF = 0b111;
	OSCTUNEbits.PLLEN = 1;
	while(!OSCCONbits.IOFS);
}

void send_glcd(bool select, uint8_t data){
		
	DC = select;
	
	CS = 0;
	send_spi(data);
	CS = 1;
}

void init_glcd(){
	
	/************************/
	
	send_glcd(COMMAND, 0xE8);
	send_glcd(DATA, 0x85);
	send_glcd(DATA, 0x00);
	send_glcd(DATA, 0x78);
	
	send_glcd(COMMAND, 0xEA);
	send_glcd(DATA, 0x00);
	send_glcd(DATA, 0x00);
	
	send_glcd(COMMAND, 0xED);
	send_glcd(DATA, 0x64);
	send_glcd(DATA, 0x03);
	send_glcd(DATA, 0x12);
	send_glcd(DATA, 0x81);
	
	/**************************/
	
	send_glcd(COMMAND, MADCTL);
	send_glcd(DATA, 0x88);
	
	send_glcd(COMMAND, PIXSET);
	send_glcd(DATA, 0x55);
	
	send_glcd(COMMAND, FRMCTR1);
	send_glcd(DATA, 0x00);
	send_glcd(DATA, 0x10);
	
	send_glcd(COMMAND, SLPOUT);
	msDelay(120);
	
	send_glcd(COMMAND, RAMWR);
	fill_all(BLACK);
	
	send_glcd(COMMAND, DISPON);
	
	send_glcd(COMMAND, RAMWR);	// start draw
}

void draw_pixel(uint16_t color){
	
	send_glcd(DATA, color>>8);
	send_glcd(DATA, color);
}

void set_cursor(uint16_t x, uint16_t y){
	
	send_glcd(COMMAND, PASET);
	send_glcd(DATA, x>>8);
	send_glcd(DATA, x);
	
	send_glcd(COMMAND, CASET);
	send_glcd(DATA, y>>8);
	send_glcd(DATA, y);
	
	send_glcd(COMMAND, RAMWR);
}

void set_all_cursor(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
	
	send_glcd(COMMAND, PASET);
	send_glcd(DATA, x0>>8);
	send_glcd(DATA, x0);
	send_glcd(DATA, x1>>8);
	send_glcd(DATA, x1);
		
	send_glcd(COMMAND, CASET);
	send_glcd(DATA, y0>>8);
	send_glcd(DATA, y0);
	send_glcd(DATA, y1>>8);
	send_glcd(DATA, y1);
	
	send_glcd(COMMAND, RAMWR);
}

void draw_rect(uint16_t x, uint16_t y, uint16_t dx, uint16_t dy, uint16_t color){
    
    uint16_t i = x+dx, j = y+dy;

    while(i >= x){
       draw_vline(i,y,i,j,color);
       i--;
    }
}

void fill_all(uint16_t color){
	
	uint16_t i = WIDTH, j = HEIGHT;
	
	set_cursor(0,0);
	while(i--){
		while(j--)
			draw_pixel(color);
		j = HEIGHT;
	}
}

void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color){
	
	int16_t dx = x1 - x0, abs_dx = dx < 0? -dx:dx;
	int16_t dy = y1 - y0, abs_dy = dy < 0? -dy:dy;
	int16_t i = x0, j = y0;
	uint16_t count = 0;
	bool select = abs_dx < abs_dy? 0:1;
	bool flag = 0;
    
    if(!abs_dy){
        draw_hline(x0, y0, x1, y1, color);
        return;
    }
    
    if(!abs_dx){
        draw_vline(x0, y0, x1, y1, color);
        return;
    }
    
   uint16_t ratio = select? abs_dy/abs_dx:abs_dx/abs_dy;
    
	while(!flag){
		
		if(i == x1 && j == y1)
			flag = 1;
		
		if(select){
			set_cursor(i, j);
			draw_pixel(color);
			if(count++ == ratio | i == x1){
				count = 0;
				dy < 0?j--:j++;
			}
			else
				dx < 0?i--:i++;
		}
		else{
			set_cursor(i, j);
			draw_pixel(color);
			if(count++ == ratio | j == y1){
				count = 0;
				dx < 0?i--:i++;
			}
			else
				dy < 0?j--:j++;
			
		}
	}
}

void draw_vline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color){        // y1 > y0
    
	uint16_t i = x0, j = y0;
	
	set_cursor(i,j);
	while(j++ <= y1)
		draw_pixel(color);
}

void draw_hline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color){        // x1 > x0
    
	uint16_t i = x0, j = y0;
	
	while(i <= x1){
		set_cursor(i++,j);
		draw_pixel(color);
	}
}

void draw_filled_circle (uint16_t xm, uint16_t ym, uint8_t r, uint16_t color){
    
    int16_t f = 1 - r, ddF_x = 1, ddF_y = 0 - (2*r), x = 0, y = r;

    draw_line(xm-r, ym, xm+r, ym, color);
    
    while(x < y)
    {
        if(f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        draw_line(xm - x, ym + y, xm + x, ym + y, color);
        draw_line(xm - x, ym - y, xm + x, ym - y, color);
        draw_line(xm - y, ym + x, xm + y, ym + x, color);
        draw_line(xm - y, ym - x, xm + y, ym - x, color);
    }
}

void draw_char(uint16_t x, uint16_t y, char c, uint16_t color){
    
    uint16_t offset = (c-' ')*FONT_HEIGHT;
    uint8_t i = 0, j =0;
    
    for(i = 0; i < FONT_HEIGHT; i++){
        for(j = 0; j < 16; j++){
            if(font16x26[offset+i] & (0x8000>>j)){
                set_cursor(x+j,y-i);
                draw_pixel(color);
            }
        }
    }
}

void draw_string(uint16_t x, uint16_t y, char* s, uint16_t color){
    
    uint8_t i = 0, j = 0;
    
    while(*s){
        draw_char(x+i*FONT_WIDTH,y,*s,color);
        i++;
        j++;
        s++;
    }
}

void draw_picture(uint8_t *image, uint16_t width, uint16_t height, uint16_t x, uint16_t y){
    
    uint16_t i = x, j = y, k = 0;
	
	while(height--){
		while(width--){
           set_cursor(i,j);
           uint16_t color = image[k]<<8 + image[k+1];
           draw_pixel(color);
           k+=2;
           i++;
       }
		i = x;
       j--;
	}
    return;
}