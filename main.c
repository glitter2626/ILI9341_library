/*
 * File:   main.c
 * Author: glitter
 *
 * Created on 2017年12月25日, 下午 9:23
 */

#include <xc.h>
#include "GLCD.h"



// CONFIG1H
#pragma config OSC = INTIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
// CONFIG3H
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
// CONFIG4L
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)

#define BOARD_WIDTH 15
#define BOARD_HEIGHT 11
#define THINK_TIME 30
bool step[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};
uint8_t chess_set[BOARD_WIDTH][BOARD_HEIGHT];   // 0 = nothing , 1 = white , 2 = black
uint16_t x = 8, y = 6;      // target init state
uint8_t time = THINK_TIME;

void init_all(){
    
    init_pic();
    init_spi();
    init_glcd();
}

void init_board(){
    
    uint16_t i = 0, j = 0;
    
    for(i = 0; i < BOARD_WIDTH; i++)
        for(j = 0; j < BOARD_HEIGHT; j++)
            chess_set[i][j] = 0;
    
    fill_all(BROWN);
    
    for(i = 0; i < 16; i++)
        draw_line(i*20,0,i*20,HEIGHT,BLACK);
    for(j = 0; j < 12; j++)
        draw_line(0,j*20,WIDTH,j*20,BLACK);
}

void init_interrupt(){
    
    RCONbits.IPEN = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    INTCON2bits.INTEDG0 = 1;
    
    INTCON2bits.INTEDG1 = 1;
    INTCON3bits.INT1IP = 1;
    
    INTCONbits.INT0IE = 1;
    INTCON3bits.INT1IE = 1;
}

void init_timer(){
    
    RCONbits.IPEN = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    T0CONbits.T08BIT = 0;   // 16 bits reg
    T0CONbits.T0CS = 0;     // timer
    T0CONbits.PSA = 0;      // use prescaler
    T0CONbits.T0PS = 0b111;     // 256 prescale
    TMR0H = 34286 >> 8;
    TMR0L = 34286;
    
    T0CONbits.TMR0ON = 1;
    INTCONbits.TMR0IE = 1;
}

void init_adc()
{
    
    ADCON1bits.VCFG1 = 0 ;  //setting vref-
    ADCON1bits.VCFG0 = 0 ;  //setting vref+
    ADCON1bits.PCFG  = 0b1110 ;  //Setting A/D Port Configuration Control
    ADCON0bits.CHS = 0b0000 ;    //setting input channel
    TRISAbits.RA0 =  1;             //setting port as input 
    ADCON2bits.ADFM = 1 ;    //setting ??? justified
    
     
    //setting acquisition time (ADCON2) ACQT 2:0
    //setting conversion time (ADCON2))
    ADCON2bits.ACQT = 0b101;
    ADCON2bits.ADCS = 0b110;
    ADCON0bits.ADON = 1;    //turn on ad module 
    /*setting adc interrupt 
     * 1.clear ADIF
     * 2.set ADIE
     * 3.SET GIE
    */
    RCONbits.IPEN = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    ADIF = 0;
    ADIP = 1;
    ADIE = 1;
}

void put_chess(uint8_t i, uint8_t j, uint16_t color){
    
    if(!chess_set[i][j]){
        chess_set[i][j] = color == BLACK? 2:1;
        draw_filled_circle((i+1)*20, (j+1)*20, 10, color);
    }
}

void draw_target(){
    uint16_t xm = (x+1)*20, ym = (y+1)*20;
    draw_line(xm+1, ym+1, xm+6, ym+6, RED);
    draw_line(xm-1, ym+1, xm-6, ym+6, RED);
    draw_line(xm+1, ym-1, xm+6, ym-6, RED);
    draw_line(xm-1, ym-1, xm-6, ym-6, RED);
}

void clear_target(){
    
    uint16_t xm = (x+1)*20, ym = (y+1)*20;
    
    if(chess_set[x][y] == 0){
        draw_line(xm+1, ym+1, xm+6, ym+6, BROWN);
        draw_line(xm-1, ym+1, xm-6, ym+6, BROWN);
        draw_line(xm+1, ym-1, xm+6, ym-6, BROWN);
        draw_line(xm-1, ym-1, xm-6, ym-6, BROWN);
    }
    else if(chess_set[x][y] == 1){
        draw_line(xm+1, ym+1, xm+6, ym+6, WHITE);
        draw_line(xm-1, ym+1, xm-6, ym+6, WHITE);
        draw_line(xm+1, ym-1, xm+6, ym-6, WHITE);
        draw_line(xm-1, ym-1, xm-6, ym-6, WHITE);
    }
    else if(chess_set[x][y] == 2){  
        draw_line(xm+1, ym+1, xm+6, ym+6, BLACK);
        draw_line(xm-1, ym+1, xm-6, ym+6, BLACK);
        draw_line(xm+1, ym-1, xm+6, ym-6, BLACK);
        draw_line(xm-1, ym-1, xm-6, ym-6, BLACK);
    }
}

void chess_ai(){
    uint8_t weight[15][11];
    uint8_t i = 0, j = 0;
    
    for(i = 0; i < BOARD_WIDTH; i++){
        for(j = 0; j < BOARD_HEIGHT; j++){
            if(chess_set[i][j] == 0){
                uint8_t k = 0;
                for(k = 0; k < 8; k++){
                    // TODO: ALL RULE
                    
                }
            }
        }
    }
}

void interrupt high_priority btn_interrupt(void) {    
    // High priority interrupts handler  
    
    uint8_t buf = SSPBUF; 
    
    if(PIR1bits.ADIF & SSPIF){
        ADIE = 0;
        int MyadcValue = (ADRESH << 8) + ADRESL;
        
        if(MyadcValue > 500 && MyadcValue < 520){
            ADIF = 0;
            ADIE = 1;
            return;
        }
       
        if(MyadcValue > 520){
            clear_target();
            x++;    // TODO: edge condition
            draw_target();
        }
        
        if(MyadcValue-512 < 500){
            clear_target();
            x--;    // TODO: edge condition
            draw_target();
        }
            
        ADIF = 0;
        ADIE = 1;
    }
       
    if(INTCONbits.INT0IF & SSPIF){
       INTCONbits.INT0IE = 0;
       put_chess(x, y, WHITE);
       INTCONbits.INT0IF = 0;
       INTCONbits.INT0IE = 1;
    }
    
    if(INTCON3bits.INT1IF & SSPIF){
        INTCON3bits.INT1IE = 0;
        clear_target();
        x++;    // TODO: edge condition
        draw_target();
        INTCON3bits.INT1IF = 0;
        INTCON3bits.INT1IE = 1;
    }
    
    if(INTCONbits.TMR0IF & SSPIF){
        INTCONbits.TMR0IE = 0;
        
        __delay_us(5);
        ADCON0bits.GO = 1;
        __delay_us(5);
        
        TMR0H = 34286 >> 8;
        TMR0L = 34286;
        uint8_t i = time/10;
        draw_rect(WIDTH-2*FONT_WIDTH,HEIGHT-FONT_HEIGHT,2*FONT_WIDTH,FONT_HEIGHT,BROWN);
        if(i){
            draw_char(WIDTH-2*FONT_WIDTH,HEIGHT,i+'0', BLUE);
        }
        else
            draw_char(WIDTH-2*FONT_WIDTH,HEIGHT,'0',BLUE);
        
        i = time%10;
        draw_char(WIDTH-FONT_WIDTH,HEIGHT,i+'0', BLUE);
       
        time--;
        INTCONbits.TMR0IF = 0;
        INTCONbits.TMR0IE = 1;
    }
    
    SSPBUF = buf;
 }    



void main(void) {
    init_all(); 
    init_board();
    init_interrupt();
    init_timer();
    init_adc();
    
    draw_filled_circle(20,20,10,WHITE);
    draw_filled_circle(20,40,10,BLACK);
    draw_string(80,80,"GLITTER2626",BLACK);
    
    while(1){
        /*draw_filled_circle(20,40,10,WHITE);
        __delay_ms(1000);
        draw_filled_circle(20,40,10,BLACK);
        __delay_ms(1000);
        draw_target();*/
    }
    return;
}
