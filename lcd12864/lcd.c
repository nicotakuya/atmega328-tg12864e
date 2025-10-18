//------------------------------------------------------------
// LCD control(TG12864E)
// for ATmega328p
// by takuya matsubara
//*タイマ2を使用

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "vram.h"

#ifndef F_CPU
	#define F_CPU 8000000	// CPUクロック周波数[Hz]
#endif
#include "timer.h"

#define LCD_DATDDR  DDRD
#define LCD_DATPORT PORTD
#define LCD_CTRDDR   DDRC
#define LCD_CTRPORT  PORTC
#define LCD_D0  (1<<3)
#define LCD_E   (1<<4)
#define LCD_RS  (1<<5)
#define LCD_CSDDR   DDRB
#define LCD_CSPORT  PORTB
#define LCD_CS1     (1<<0)
#define LCD_CS2     (1<<1)
#define ENABLE_ON	LCD_CTRPORT&=~LCD_E;
#define ENABLE_OFF	LCD_CTRPORT|=LCD_E;
#define RS_COMMAND	LCD_CTRPORT&=~LCD_RS;
#define RS_DATA		LCD_CTRPORT|=LCD_RS;

void lcd_cs(char chip);
void lcd_setvram(char cs,char page);
void lcd_setbyte(unsigned char dat);
//------------------------------------------------------------
char chipnum = 0;
char pagenum = 0;
char lcd_enable = 0;
int lcdptr = 0;
char bytecnt = 64;

//------------------------------------------------------------
// 128 x 64dot
//       chip1     chip2
//page0 [ 64dot] [ 64dot]
//          :        :
//page7 [ 64dot] [ 64dot]
//------------------------------------------------------------
void lcd_control(void){
	#define SENDBYTES	1
	char i=SENDBYTES;

	if(lcd_enable==0)return; 	//LCD更新を無効

	if(bytecnt >= 64){
		pagenum = lcdptr/128;	//0-7

		lcd_cs(chipnum);
		RS_COMMAND;
		lcd_setbyte(0xB8 + pagenum);	//set page
		lcd_setbyte(0x40 + 0);		//Y address
		RS_DATA;

		chipnum ^= 1;
		bytecnt=0;
		return;
	}
	while(i--){
		lcd_setbyte( vram[lcdptr++] );
		bytecnt ++;
	}
	if(lcdptr >= 1024){
		lcdptr = 0;
	}
}

//------------------------------------------------------------
void lcd_delay(void)
{
	char loopcnt=F_CPU/1000000;

	while(loopcnt--){
		asm("nop");
	}
}

//------------------------------------------------------------
void lcd_setbyte(unsigned char dat)
{
	ENABLE_OFF;
	LCD_DATPORT = 1 | dat;

	if(dat & 1)
		LCD_CTRPORT |= LCD_D0;
	else
		LCD_CTRPORT &= ~LCD_D0;

	lcd_delay();
	ENABLE_ON;
	lcd_delay();
}

//------------------------------------------------------------chip select
void lcd_cs(char chip)
{
	LCD_CSPORT &= ~(LCD_CS1 | LCD_CS2);
	if(chip==0)	LCD_CSPORT |= LCD_CS1;
	if(chip==1)	LCD_CSPORT |= LCD_CS2;
	lcd_delay();
}

//------------------------------------------------------------
void lcd_init(void)
{
	char cs;

	LCD_DATDDR |= 0xFE;	//
	LCD_CTRDDR |= (LCD_D0 | LCD_RS | LCD_E);	//output
	LCD_CSDDR |=  (LCD_CS1 | LCD_CS2);
	lcd_cs(-1);	// CS disable
	ENABLE_ON;
	timer_waitmsec(30);
	RS_COMMAND;
	for(cs=0; cs<=1; cs++){
		lcd_cs(cs);
		lcd_setbyte(0xC0 + 0);	//set start line(0)
		lcd_setbyte(0x3F);	//Display ON
		lcd_cs(-1);	// CS disable
	}
	RS_DATA;

	lcd_enable=1;
}

//------------------------------------------------------------
void lcd_sendall(void)
{
	if(lcd_enable==0)return; 	//LCD更新を無効

	for(lcdptr=0; lcdptr<1024; lcdptr++){
		if((lcdptr % 64)==0){
			chipnum = (lcdptr % 128)/64;
			pagenum = lcdptr / 128;

			lcd_cs(chipnum);
			RS_COMMAND;
			lcd_setbyte(0xB8 + pagenum);	//set page
			lcd_setbyte(0x40 + 0);		//Y address
			RS_DATA;
		}
		if(lcdptr < 1020){
			lcd_setbyte( vram[lcdptr] );
		}else{
			//画面右上の記号を排除
			lcd_setbyte( vram[lcdptr] & 0x7F);
		}
	}
}

//------------------------------------------------------------
void lcd_redraw(char onoff)
{
	lcd_enable = onoff;
}
