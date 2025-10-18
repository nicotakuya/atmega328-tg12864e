//--------------------------------------------------
// LCD 128 x 64
//  for ATmega328p
//  by takuya matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include "vram.h"
#include "lcd.h"
#include "rand.h"
#include "vect.h"
#include "timer.h"
#include "spfight.h"
#include "sio.h"
#include "siotx.h"
#include "ad.h"
#include "servo.h"

#ifndef F_CPU
#define F_CPU 8000000	// CPUクロック周波数[Hz]
#endif

char selmenu(PGM_P p,char menu);

const unsigned char yajirusi[] PROGMEM ={
	0b0001000,
	0b0001100,
	0b1111110,
	0b1111111,
	0b1111110,
	0b0001100,
	0b0001000
};

const char stretc[][17] PROGMEM ={
	// 0123456789abcdef
	"DIY LCD",	//0
	"ｹｯﾃｲ",		//1
	"...EXIT",		//2
	"  NICOTAK.COM ",	//3
};

#define MENULEN 9
const char menuname[][MENULEN] PROGMEM ={
	"ｷﾞｼﾞ3D",			//0
	"ﾃｷｽﾄ",		//1
	"ｻﾝｶｸ",		//2
	"BOUND",	//3
	"S.WATCH",	//4
	"JOYSTICK",	//5
	"V METER",	//6
	"ｼﾘｱﾙ",		//7
	"SPFIGHT",
	"LIFEGAME",
	"WAVE OUT",
	"ｽｸﾛｰﾙ",
	"ORBIT",
	"ｶｲﾃﾝ",
	"EEPROM",
	"BAUDRATE",
	"TX DATA",	//16
	"FROMVIEW",	//17
	"FROM WRT",	//18
	"RECV BMP",	//19
	""
};

const char about[][20] PROGMEM ={
	"ｻﾝｼﾞｹﾞﾝ ﾌｳ ﾋｮｳｼﾞﾃﾞﾓ",	//0
	"ﾓｼﾞ ﾋｮｳｼﾞﾃﾞﾓ",			//1
	"ｶｲﾃﾝ ﾋｮｳｼﾞﾃﾞﾓ",		//2
	"ｽﾌﾟﾗｲﾄ ﾌｳ ﾋｮｳｼﾞﾃﾞﾓ",	//3
	"ｼﾞｶﾝ ｦ ﾊｶﾘﾏｽ",			//4
	"ｱﾅﾛｸﾞ ｼﾞｮｲｽﾃｨｯｸ ﾃｽﾄ",	//5
	"(25)ADC2/PC2 ﾆｭｳﾘｮｸ",	//6
	"(2)RX/PD0 UART RX",	//7
	"ｱｸｼｮﾝｹﾞｰﾑ",			//8
	"ﾄﾞｯﾄ ｶﾞ ﾌｴﾀﾘ ﾍｯﾀﾘｽﾙ",	//9
	"(16)PB2->SPEAKER",	//10
	"ｽｸﾛｰﾙﾋｮｳｼﾞ ﾉ ｼﾞｯｹﾝ",	//11
	"365ﾆﾁ ﾃﾞ 1ｼｭｳ",		//12
	"ｶﾞﾒﾝ ｦ 90ﾄﾞｶｲﾃﾝ ｼﾏｽ",	//13
	"EEPROM",				//14
	"UART TX ADJUST",		//15
	"(17)PB3->TX",
	"(16)",
	"recv BMP",
	""
};

extern unsigned char vram[VRAMSIZE];

//------------------------------------------------------------
#define TIMERRATE	333
//1秒あたりのLCDの1ページ更新回数
//これを1/8すると、1秒あたりの1画面更新回数になります。

//int vptr=0;
int linesync;
unsigned char tick;

//SIGNAL(TIMER2_OVF_vect){
SIGNAL (TIMER1_OVF_vect){
	lcd_control();	// 15750/1000 = 15.75fps

	linesync++;
	if(linesync>=262){
		linesync=0;
		tick++;			// 1/60sec
	}
}

//--------------------------------128X64PIXEL
void siotx_sendvram1(void)
{
	int i;

	for(i=0; i<1024; i++){
		sio_tx(vram[i]);
		//		sio_tx('A');
	}
}
//--------------------------------64X32PIXEL
void siotx_sendvram2(void)
{
	unsigned char x,y;
	unsigned char bdat;
	unsigned char mask=0x03;
	char y1;
	int pos;

	for(y=0; y<64; y+=16){
		for(x=0; x<128; x+=2){
			bdat=0;
			for(y1=0; y1<16; y1+=2){
				pos = ((y+y1)/8)*128+x;
				bdat >>= 1;
				if(vram[pos] & mask){
					bdat |= 0x80;
				}
				mask <<= 2;
				if(mask==0)mask=0x03;
			}
			sio_tx(bdat);
		}
	}
}
//--------------------------------32X16PIXEL
void siotx_sendvram3(void)
{
	char x,y;
	unsigned char bdat=0;
	char bitcnt=0;
	unsigned char mask;
	int pos;

	for(y=0; y<64; y+=4){
		for(x=(3*8*4); x>=0; x-=(8*4)){
			pos = (y/8)*128+x;
			mask = 0x0F << ((y%2)*4);

			for(bitcnt=0; bitcnt<8; bitcnt++){
				bdat<<=1;
				if(vram[pos] & mask){
					bdat|=1;
				}
				pos += 4;
			}
			sio_tx(bdat);
		}
	}
}

char sendmode=0;
//--------------------------------
void siotx_senddata(void)
{
	switch(sendmode){
		case 0:
		break;
		case 1:
		siotx_sendvram1();	//128X64
		break;
		case 2:
		siotx_sendvram2();	//64X32
		break;
		case 3:
		siotx_sendvram3();	//32X16
		break;
	}
}
//------------------------------
extern unsigned int timermsec;

void timer_waitmsec2(int msec)
{
	timermsec = msec;

	siotx_senddata();
//	lcd_sendall();

	while(timermsec){
		timer_control();
	}
}

//----------------------------------------bound ball

const unsigned char patball[] PROGMEM = {
	0b0011100,
	0b0111110,
	0b1110111,
	0b1100011,
	0b1110111,
	0b0111110,
	0b0011100
};

typedef struct {
	int x;
	int y;
	int x1;
	int y1;
} ST_BALL;   // 構造体

#define MLT 3

void bound_init(ST_BALL *p)
{
	p->x = 0;
	p->y = rand_get(vram_geth())*MLT;
	p->x1 = rand_get(5)+1;
	p->y1 = -rand_get(5)-1;
}

void bound(void)
{
	#define BALLMAX 10
	ST_BALL ball[BALLMAX];

	int x2,y2;
	int xd,yd;
	unsigned char i,j;
	int xmax,ymax;

	xmax = vram_getw()-1;
	ymax = vram_geth()-1;

	for(i=0; i<BALLMAX; i++){
		bound_init(&ball[i]);
	}

	while(1){
		lcd_redraw(0);
		vram_cls();		// clear VRAM
		
		for(i=0; i<BALLMAX; i++){
			x2 = ball[i].x;
			y2 = ball[i].y;

			x2 += ball[i].x1;
			y2 += ball[i].y1;
			for(j=0 ;j<BALLMAX ;j++){
				if(i==j)continue;
				xd = x2-ball[j].x;
				yd = y2-ball[j].y;

				if( (fnc_abs(xd) < 7*MLT)&&(fnc_abs(yd) < 7*MLT)){
					ball[i].x1 = fnc_sgn(xd)*(rand_get(MLT)+1);
					ball[i].y1 = fnc_sgn(yd)*(rand_get(MLT)+1);
					ball[j].x1 = -ball[i].x1;
					ball[j].y1 = -ball[i].y1;
				}
			}

			if(rand_get(2))
			ball[i].y1 ++;

			if(y2 > (ymax-3)*MLT){
				ball[i].y1 = -ball[i].y1;
			}
			ball[i].x = x2;
			ball[i].y = y2;
			vram_spput(x2/MLT,y2/MLT,(PGM_P)patball);

			if((x2 < 0)||(x2 > xmax*MLT)){
				bound_init(&ball[i]);
			}
		}
		lcd_redraw(1);
		timer_waitmsec2(25);
		if((joy_gety()/64)!=0)break;
	}
}
//------------------------------------------------------------------sankaku
typedef struct {
	int ang;	/* 角度 */
	char len;	/* 距離 */
} VECLINE;

typedef struct {
	int x;	/*  */
	int y;	/*  */
	int x1;	/*  */
	int y1;	/*  */
	int ang;	/* 角度 */
	int a1;	/* 角度 */
	int size;  /*  */
} INSEKI;

VECLINE myship[] = {
	{  40 ,6},  /*  */
	{  320,6},  /*  */
	{  180,6},  /*  */
	{  40 ,6},  /*  */
	{  -1 ,-1}  /*  */
};

#define OFST 5

void sankaku_init(INSEKI * pinsk){
	int h;
	h = vram_geth();

	pinsk->x = -(8<<OFST);
	pinsk->x1 = (int)rand_get(20)+30;
	pinsk->ang = (int)rand_get(36)*10;
	pinsk->y = (int)rand_get(6)*((h<<OFST)/6);
	pinsk->y1 = (int)rand_get(13)-7;

	pinsk->a1 = (int)rand_get(13)-6;
	pinsk->size = rand_get(8)+4;
}

void vect_put(VECLINE *pV,int vx,int vy,int angle,int zoom)
{
	int i;
	int tempkak;
	int x1,y1,x2=0,y2=0;

	for( i=0; pV->ang != -1; i++){

		tempkak = pV->ang + angle;
		x1 = vect_x1(tempkak)* pV->len;
		y1 = vect_y1(tempkak)* pV->len;
		x1 = (zoom*x1>>10) + vx;
		y1 = (zoom*y1>>10) + vy;
		if(i > 0){
			vram_line(x1,y1,x2,y2,1);
		}
		x2 = x1;
		y2 = y1;
		pV++;
	}
}

//----------------------------------------------------
void sankaku(void)
{
	#define SPACEOVER 20000
	#define MAXSPEED 150
	#define CHANGECNT 200 // 座標更新周期
	#define INSEKIMAX 10
	INSEKI insk[INSEKIMAX];
	INSEKI *pInsk;
	int i;
	int tx,ty;

	for(i=0; i<INSEKIMAX; i++){
		sankaku_init(&insk[i]);
	}

	while(1){

		lcd_redraw(0);
		vram_cls();		// clear VRAM

		pInsk = &insk[0];
		for(i=0; i<INSEKIMAX; i++,pInsk++){
			tx = pInsk->x;
			ty = pInsk->y;
			tx += pInsk->x1;
			ty += pInsk->y1;
			if((tx < -(8<<OFST))||(tx>((128+8)<<OFST))){
				sankaku_init(pInsk);
				continue;
			}
			vect_put(myship, tx>>OFST, ty>>OFST, pInsk->ang, pInsk->size);
			pInsk->x = tx;
			pInsk->y = ty;
			pInsk->ang += pInsk->a1;
			if(pInsk->ang >=360) pInsk->ang -= 360;
			if(pInsk->ang <   0) pInsk->ang += 360;
		}
		lcd_redraw(1);
		timer_waitmsec2(100);
		//		wait_and_sendvram(100);
		if((joy_gety()/64)!=0)break;
	}
}

//-----------------------------------------life game
void lifegame(void){
	#define WSIZE	50

	signed char x,y;
	signed int ax=0,bx=0;
	signed int ay=0,by=0;
	char c,d;
	int i,pass=0;
	signed char x1[8]={ 0, 1, 1, 1, 0,-1,-1,-1};
	signed char y1[8]={-1,-1, 0, 1, 1, 1, 0,-1};

	if(vram_getw() > 64){
		bx=64;
	}else{
		by=64;
	}

	for(i=0; i<400; i++){
		x = rand_get(WSIZE);
		y = rand_get(WSIZE);
		vram_pset(x,y,1);
	}
	//	vram_line(62,0,62,63,1);

	while(1){
		lcd_redraw(0);

		vram_locate(bx,by+(64-14));
		vram_putdec(pass++);

		for(y=0; y<WSIZE; y++){
			for( x=0; x<WSIZE; x++){
				c=0;
				d=0;	//接触してる点を数える
				for( i=0;i<8;i++){
					if(vram_point(ax+x+x1[i],ay+y+y1[i])){
						d++;
						if(d>=4){
							break;
						}
					}
				}
				if(d==2) c=vram_point(ax+x,ay+y);	//現状維持
				if(d==3) c=1;				//誕生
				vram_pset(bx+x,by+y,c);
			}
		}
		x=ax;	//swap ax,bx
		ax=bx;
		bx=x;
		y=ay;	//swap ay,by
		ay=by;
		by=y;

		lcd_redraw(1);
		timer_waitmsec2(25);
		if((joy_gety()/64)!=0)break;
	}
}

//-----------------------------------------------text test
void texttest(void)
{
	unsigned char ch=0x20;

	while(1){
		vram_putch(ch++);
		if(ch == 0x7F){
			ch=0xB0;
		}
		if(ch > 0xDF){
			ch=0x20;
		}
		timer_waitmsec2(100);
		if((joy_gety()/64)!=0)break;
	}
}

//-----------------------------------------------joystick test
void joytest(void)
{
	int adcx,adcy,x=0,y=0;
	char tx=0,ty=0;
	int timeout=100;

	//テキストの位置
	if(vram_getw() > 64){//横画面
		tx=64;
	}else{		//縦画面
		ty=64;
	}

	while(timeout--){
		adcx = joy_getx();
		adcy = joy_gety();
		vram_locate(tx,ty);
		vram_putdec(adcx+128);
		vram_locate(tx,ty+24);
		vram_putdec(adcy+128);

		vram_spclr(x,y);
		x=(adcx / (256/64))+32;
		y=(adcy / (256/64))+32;
		vram_spput(x,y,(PGM_P)patball);
		vram_line(0,32,63,32,1);
		vram_line(32,0,32,63,1);
		timer_waitmsec2(50);
	}
}

//---------------------------------------
const char menubaud[][MENULEN] PROGMEM ={
	"2400BPS",
	"9600BPS",
	"38400BPS",
	"57600BPS",
	"115.2k?",
	""
};

const char menurezo[][MENULEN] PROGMEM ={
	"CANCEL",
	"128x64",
	"64x32dot",
	"32x16dot",
	""
};

const char menuchrhex[][MENULEN] PROGMEM ={
	"CHARA",
	"HEX",
	""
};

unsigned char baudrate;

void setbaudrate(void)
{
	sendmode = 0;
	baudrate = selmenu((PGM_P)menubaud ,baudrate);

	eeprom_write_byte((uint8_t *)1,baudrate);
	eeprom_busy_wait();	//3.4ms
}

void senddata(void)
{
	//	char rezo;
	//		sendmode=0;
	//		siotx_close();

	sendmode = selmenu((PGM_P)menurezo ,0);

	//	sendmode=rezo;
	if(sendmode == 0){
		siotx_close();
		return;
	}
	siotx_init(baudrate);
}


//-----------------------------------------------
void recvbmp1(void)
{
	char timeout = 0;
	int pos = 0;

	while(1){
		if(sio_rxcheck()==0){//データがない場合、
			if(joy_gety()/64)break;
			timeout++;
			if(timeout > 100){
				pos=0;	//timeout
			}
			continue;
		}
		while(sio_rxcheck()){//データがある場合、

			vram[pos++] = sio_rx();
			if(pos>=1024){
				pos=0;
				timer_waitmsec2(0);
			}
		}
		timeout=0;
	}
}

//-----------------------------------------------
void recvbmp2(void)
{
	char timeout = 0;
	int pos = 0;
	char bitcnt;
	unsigned int tempw=0;
	unsigned char bdat;

	while(1){
		if(sio_rxcheck()==0){//データがない場合、
			if(joy_gety()/64)break;
			timeout++;
			if(timeout > 100){
				pos=0;	//timeout
			}
			continue;
		}
		while(sio_rxcheck()){//データがある場合、
			bdat=sio_rx();
			for(bitcnt=0; bitcnt<8; bitcnt++){
				tempw >>= 2;
				if(bdat & 1){
					tempw |= 0xC000;
				}
				bdat>>=1;
			}
			vram[pos + 128] = tempw>>8;
			vram[pos + 0  ] = tempw & 0xFF;
			vram[pos + 129] = vram[pos + 128];
			vram[pos + 1  ] = vram[pos + 0  ];
			pos += 2;
			if((pos % 128)==0)pos += 128;
			if(pos >= 1024){
				timer_waitmsec2(0);
				pos=0;
			}
		}
		timeout=0;
	}
}

//-----------------------------------------------
void recvbmp3(void)
{
	char timeout = 0;
	int pos;
	char x=3,y=0;
	char bitcnt;
	unsigned char mask;
	unsigned char bdat;

	while(1){
		if(sio_rxcheck()==0){//データがない場合、
			if(joy_gety()/64)break;
			timeout++;
			if(timeout > 100){
				pos=0;	//timeout
			}
			continue;
		}
		while(sio_rxcheck()){//データがある場合、
			bdat=sio_rx();
			pos = (y/2)*128+(x*4*8);
			if(y%2){
				mask = 0xF0;
				}else{
				mask = 0x0F;
			}
			for(bitcnt=0; bitcnt<8; bitcnt++){
				if(bdat & 0x80){
					vram[pos++] |= mask;
					vram[pos++] |= mask;
					vram[pos++] |= mask;
					vram[pos++] |= mask;
					}else{
					mask = ~mask;
					vram[pos++] &= mask;
					vram[pos++] &= mask;
					vram[pos++] &= mask;
					vram[pos++] &= mask;
				}
				bdat <<= 1;
			}
			x--;
			if(x<0){
				x=3;
				y++;
				if(y >= 32){
					y=0;
					timer_waitmsec2(0);
				}
			}
		}
		timeout=0;
	}
}

//--------------------------
void recvbmp(void)
{
	char rezo;
	long baud;

	rezo = selmenu((PGM_P)menurezo ,0)-1;
	if(rezo < 0) return;

	vram_cls();
	timer_waitmsec2(0);
	
	switch(baudrate){
	case 0:
		baud = 2400;
		break;
	case 1:
		baud = 9600;
		break;
	case 2:
		baud = 38400;
		break;
	case 3:
		baud = 57600;
		break;
	default:
		baud = 115200;
		break;
	}

	sio_init(baud);

	switch(rezo){
		case 0:
		recvbmp1();
		break;
		case 1:
		recvbmp2();
		break;
		case 2:
		recvbmp3();
		break;
	}

	sio_close();
}

const char strhex[] PROGMEM ="INPUT\n\n\n<->:SELECT";	//

unsigned char inputhex(unsigned char x)
{
	vram_putstrpgm((PGM_P)strhex);

	while(1)
	{
		vram_locate(0,0);
		vram_puthex(x);

		if(joy_gety()/64)break;
		x += joy_getx()/64;

		timer_waitmsec2(300);
	}
	return(x);
}
//-------------------
const char strwave[] PROGMEM ="WAVE OUT\n\n   HZ\n<->:SELECT";	//

void wavetest(void)
{
	int hz=330;
	int hz1;
	
	//	vram_locate(0,0);
	vram_putstrpgm((PGM_P)strwave);

	//-----------------------------------------------
	/// PWM初期設定 ///
	TCCR1A = (2<<COM1B0)|(2<<WGM10); // タイマモード
	TCCR1B = (3<<WGM12)|(1<<CS10); // プリスケーラ (1分周)
	//	ICR1 = (F_CPU / hz) - 1;	/// TOP値
	TCNT1 = 0;
	OCR1B = ICR1 / 2;	//Output Compare Register B

	DDRB |= (1<<2);	//output

	// Timer/counter mode of operation : Fast PWM
	// TOP:ICR1
	// Update of OCR1x at:BOTTOM
	// TOV1 flag set on:TOP
	// Clock select:no prescaling

	while(1)
	{
		vram_locate(8,12*1);
		vram_putdec(hz);

		if(joy_gety()/64)break;
		hz1 = joy_getx()/24;
		hz += hz1;
		if(hz < 150)hz=150;
		if(hz > 15000)hz=15000;
		if(hz1 != 0){
		}
	}
	DDRB &= ~(1<<2);
}

//------------------------------------------------------------------
void vram_putdec2(char x)
{
	vram_putch('0'+(x/10));
	vram_putch('0'+(x%10));
}


const char strwatch[] PROGMEM ="''  '\n\n<->:START/STOP";	//

void stopwatch(void)
{
	long msec=0;	//モード=停止中
	char mode=0;
	char swdisable=1;
	unsigned char tick2;

	//	vram_locate(0,0);
	vram_putstrpgm((PGM_P)strwatch);

	tick2 = tick;

	while(1)
	{
		if(tick2 != tick){
			tick2++;
			if(mode){
				msec += 16;
			}
			vram_locate(0,12*1);
			vram_putdec2((char)(msec / 60000));	   //分
			vram_putch(':');
			vram_putdec2((char)((msec / 1000)% 60));  //秒
			vram_putch(':');
			vram_putdec2((char)((msec / 10)% 100));	//1/100秒
			timer_waitmsec2(0);
		}
		if(joy_gety()/64)break;
		if(joy_getx()/64){
			if(swdisable)continue;

			mode ^= 1;		//モード反転
			if(mode){			//タイマ停止
				msec=0;
			}
			swdisable=1;
			}else{
			swdisable=0;
		}
	}
}
//---------------------------------

const char strcount[] PROGMEM ="COUNT";
const char strvolt[] PROGMEM ="VOLT";

void vmeter(void)
{
	int x,y,v;
	//	long mcntbak;
	unsigned int adc;
	char tx=0,ty=0;
	unsigned char memori=0;

	//テキストの位置
	if(vram_getw() > 64){//横画面
		tx=64;
		}else{		//縦画面
		ty=64;
	}

	vram_cls();

	PORTC &= ~(1<<2);	//pullup off
	DDRC &= ~(1<<2);	//input

	vram_locate(tx+(3*8),ty+(12*1));
	vram_putstrpgm((PGM_P)strcount);
	vram_locate(tx+(3*8),ty+(12*4));
	vram_putstrpgm((PGM_P)strvolt);

	while(1)
	{
		if(joy_gety()/64)break;
		lcd_redraw(0);
		adc = (unsigned int)ad_get(2);
		v = (int)(((long)adc*500)/1023);
		vram_locate(tx,ty);
		vram_putdec(adc);
		vram_locate(tx,ty+(3*12));
		vram_putch('0'+(v / 100));
		vram_putch('.');
		vram_putch('0'+((v / 10)%10));
		vram_putch('0'+(v%10));

		x = 61;
		y = 63-(adc*63/1023);
		vram_pset(x, y,1);

		memori++;
		if((memori % 2)==0){
			for (y=0; y<64; y++){
				for(x=0; x<62; x++){
					vram_pset(x, y,vram_point(x+1,y));
				}
			}
		}
		if((memori % 4)==0){
			x = 61;
			for(v=0; v<=5; v++){
				y = 63*v/5;
				vram_pset(x, y ,1);	//ドット描画
			}
		}

		lcd_redraw(1);
		timer_waitmsec2(25);
	}
}

//------------------------------
void orbit(void)
{
	int xa,ya;
	int xb,yb,db=0;
	int xc,yc,dc=0;
	int day=0,dtmp;

	vram_cls();
	xa = vram_getw()/2;
	ya = 32;

	while(1){
		if(joy_gety()/64)break;

		dtmp = db;
		if(dtmp < dc)
		dtmp += 360;

		dc += 20;	//衛星の回転
		if(dtmp < dc){
			dc -= 360;
			day++;
			if(day>=365)day=0;
			db = (int)(360L*day/365);	//1年間で1周
		}
		xb=xa+vect_x1(db)/(255/20);	//地球の位置
		yb=ya+vect_y1(db)/(255/20);

		xc=xb+vect_x1(dc)/(255/10);	//月の位置
		yc=yb+vect_y1(dc)/(255/10);

		lcd_redraw(0);
		vram_cls();
		vram_locate(0,0);
		vram_putdec(day);
		vram_spput(xa,ya,(PGM_P)patball);
		vram_spput(xb,yb,(PGM_P)patball);
		vram_spput(xc,yc,(PGM_P)patball);
		lcd_redraw(1);
		timer_waitmsec2(25);
	}
}

//------------------------------
void scroll(void)
{
	int rx=64;
	unsigned char ry=0;
	int x=64;
	int i,j;

	while(1){
		if(joy_gety()/64)break;

		x += joy_getx()/32;
		if(x<  4)x=4;
		if(x>124)x=124;

		rx += rand_get(7)-3;
		if(rx<	 25 )rx=25;
		if(rx>(128-25))rx=128-25;
		ry++;

		lcd_redraw(0);
		for(j=7*128; j>=1*128; j-=128){
			for(i=0;i<128;i++){
				vram[i+j] = vram[i+j-128];
			}
		}
		for(i=0;i<128;i++){
			if(((rx-20)>i)||((rx+20)<i))
			vram[i] = 0xAA;
			else
			vram[i] = 0x0;
		}

		vram[rx-20]=0xFF;
		vram[rx+20]=0xFF;

		vram_spput(x,32,(PGM_P)patball);

		lcd_redraw(1);

		timer_waitmsec2(50);
	}
}

//-----------------------------------------------------
void randscape(void)
{
	#define DEGPITCH	15

	int x1,x2,y,i,deg,iso=0;
	int w,h,w2,h2;

	w = vram_getw();
	h = vram_geth();
	w2 = w/2;
	h2 = h/2;

	while(1){
		if(joy_gety()/64)break;

		iso += 1;
		if(iso< 0	   )iso += DEGPITCH;
		if(iso>=DEGPITCH)iso -= DEGPITCH;

		lcd_redraw(0);
		vram_cls();
		for(i=-10; i<10; i++){
			x1=i*(w/20);
			x2=i*(w/5);
			vram_line(w2+x1,h2,w2+x2,h-1,1);
		}

		for(deg=0; deg<90; deg+=DEGPITCH){
			y=(255+vect_y1(deg+iso))*h2/255;
			vram_line(0,h2+y,w-1,h2+y,1);
		}
		lcd_redraw(1);
		timer_waitmsec2(100);
	}
}
//--------------------------------------------------------
void eeprom_view(void)
{
	int adr=0;
	int i;
	unsigned char dat;
	unsigned char joydat;

	while(1){
		if(adr < 0) adr = 0;
		if(adr > (1023-4)) adr = (1023-4);
		for(i=0; i<5; i++){
			dat = eeprom_read_byte((uint8_t *)adr+i);
			vram_locate(0,12*i);
			vram_puthexw(adr+i);
			vram_putch(':');
			vram_puthex(dat);
		}
		timer_waitmsec2(0);

		joydat = joy_wait();
		if(joydat & JOY_RIGHT) adr++;
		if(joydat & JOY_LEFT ) adr--;
		if(joydat & (JOY_UP+JOY_DOWN))break;
	}
}

//--------------------------------------------------------
void flashrom_view(void)
{
	PGM_P adr = (PGM_P)0x4000;
	int i;
	unsigned char dat;
	unsigned char joydat;

	while(1){
		//		if(adr < 0x4000) adr = 0x4000;
		//		if(adr > (1023-4)) adr = (1023-4);
		for(i=0; i<5; i++){
			dat = pgm_read_byte(adr+i);
			vram_locate(0,12*i);
			vram_puthexw((unsigned int)adr+i);
			vram_putch(':');
			vram_puthex(dat);
		}
		timer_waitmsec2(0);

		joydat = joy_wait();
		if(joydat & JOY_RIGHT) adr++;
		if(joydat & JOY_LEFT ) adr--;
		if(joydat & (JOY_UP+JOY_DOWN))break;
	}
}

//----------
//0x0000 - 0x7fff
void BOOTLOADER_SECTION flashrom_write(void)
{
	#define	STARTADR	0x4000

	unsigned int	i;
	unsigned int	wdat;
	unsigned long	adr;

	adr = STARTADR;
	wdat = 0;

	while(adr < (STARTADR + 512)){
		boot_page_erase(adr);	// ページ消去
		boot_spm_busy_wait();

		for (i = 0; i < SPM_PAGESIZE; i += 2){
			boot_page_fill(adr + i, wdat);
			wdat++;
		}
		boot_page_write(adr);	// ページ書き込み
		boot_spm_busy_wait();

		adr += SPM_PAGESIZE;
	}
	boot_rww_enable();
}

//-----------------------------------------------
const char strrecv[] PROGMEM ="RX ｼﾞｭｼﾝﾁｭｳ...\n";	//
void recvchar(void)
{
	unsigned char bdat;
	char hexmode;
	long baud;

	hexmode = selmenu((PGM_P)menuchrhex ,0);

	switch(baudrate){
	case 0:
		baud = 2400;
		break;
	case 1:
		baud = 9600;
		break;
	case 2:
		baud = 38400;
		break;
	case 3:
		baud = 57600;
		break;
	default:
		baud = 115200;
		break;
	}

	sio_init(baud);

	vram_putstrpgm((PGM_P)strrecv);

	while(1){
		if(joy_gety()/64)break;

		while(sio_rxcheck()){//データがある場合、
			bdat = sio_rx();
			if(hexmode){
				vram_puthex(bdat);
			}else{
				vram_putch(bdat);
			}
		}
		timer_waitmsec2(0);
	}

	sio_close();

}

//-----------------------------------------------------------
char selmenu(PGM_P p,char menu)
{
	char menumax;
	char menustart=0;
	char menuheight;
	char y,i,joydat;

	for(menumax=0;;menumax++){
		if(pgm_read_byte(p+((int)menumax*MENULEN))==0){
			break;
		}
	}
	while(1){
		lcd_redraw(0);
		vram_cls();

		y = vram_geth()-12;
		vram_locate(14,y);
		vram_putstrpgm((PGM_P)stretc[1]);
		vram_spput(7 ,y+7,(PGM_P)yajirusi);
		menuheight = y/13;

		if(menu < 0){
			menu = menumax-1;
			menustart = menu-(menuheight-1);
		}
		if(menu >= menumax){
			menu = 0;
			menustart = 0;
		}
		if(menu < menustart)menustart--;
		if((menu-menustart)>=menuheight)menustart++;

		if((menu-menustart)>=menuheight){
			menustart=menu-(menuheight-1);
		}

		for(i=0; i<menuheight; i++){
			if((menustart+i)>=menumax)
			break;

			vram_locate(0,i*13);
			vram_putstrpgm(p+(((int)menustart+i)*MENULEN));
		}

		for(i=0;i<14;i++){
			vram_line(0,(menu-menustart)*13+i,127,(menu-menustart)*13+i,2);
		}
		lcd_redraw(1);
		timer_waitmsec2(0);

		joydat = joy_wait();
		if(joydat & JOY_UP  ) menu--;
		if(joydat & JOY_DOWN) menu++;
		if(joydat & (JOY_RIGHT+JOY_LEFT) )break;
	}
	vram_cls();
	return(menu);
}

//------------------------------------------------
int main(void)
{
	char i;
	char menu=0;

	timer_init();
	rand_init();
	siotx_close();

	//--------画面回転
	i = eeprom_read_byte((int)0);
	if((i<0)||(i>3))i=0;
	vram_settate(i);

	//--------baudrate
	baudrate = eeprom_read_byte((uint8_t *)1);
	if((baudrate<0)||(baudrate>4))baudrate=1;	//9600bps

	//	while(1){
	//		sio_tx('A');
	//	}

	sei();	//割り込み許可
	//--------------
	//	TCCR2A = 0;
	//	TCCR2B = 7;		// Clock Select(clk/1024)
	//	TIMSK2 |= (1<<TOIE2);	//Timer/Counter2 Overflow Interrupt Enable
	//	TCNT2 = 0xFF;
	//	TIFR2 |= (1<<TOV2);

	TCCR1A = (2<<WGM10);
	TCCR1B = (3<<WGM12)|(1<<CS10);
	ICR1 = (F_CPU / 15750)-1; //割り込み
	TIMSK1 |= (1<<TOIE1);	// タイマー１溢れ割込み
	TIFR1 |= (1 << TOV1);	// オーバーフローフラグをクリア
	//		timer_waitmsec(1);

	lcd_init();

	//	while(1);

	//	vram_locate(40,64-12);
	//	vram_putstrpgm((PGM_P)stretc[0]);
	//	for(i=0;i<(64-12-24)/3;i++){
	//		vram_scroll(0,3);
	//	}
	//	timer_waitmsec(1000);

	while(1){
		menu = selmenu((PGM_P)menuname ,menu);

		vram_putstrpgm((PGM_P)menuname[(int)menu]);
		vram_locate(0,12+4);
		vram_putstrpgm((PGM_P)about[(int)menu]);
		i=vram_geth()-12;
		vram_locate(14,i);
		vram_putstrpgm((PGM_P)stretc[1]);
		vram_spput(7 ,i+7,(PGM_P)yajirusi);
		timer_waitmsec2(0);

		if(joy_wait() & (JOY_UP+JOY_DOWN) )continue;

		//		vram_cls();
		//		vram_putstrpgm((PGM_P)stretc[1]);
		//		timer_waitmsec(800);
		vram_cls();
		joy_waitoff();

		switch(menu){
			case 0:
				randscape();
				break;
			case 1:
				texttest();	//
				break;
			case 2:
				sankaku();	//
				break;
			case 3:
				bound();	//
				break;
			case 4:
				stopwatch();	//
				break;
			case 5:
				joytest();	//
				break;
			case 6:
				vmeter();	//
				break;
			case 7:
				recvchar();	//
				break;
			case 8:
				spfight();	//
				break;
			case 9:
				lifegame();	//
				break;
			case 10:
				wavetest();	//
				break;
			case 11:
				scroll();	//
				break;
			case 12:
				orbit();	//
				break;
			case 13:
				i = vram_gettate();	//
				i = (i+1)%4;
				vram_settate(i);
				//			menu=0;
				//			menustart=0;
				eeprom_write_byte((int)0,i);
				eeprom_busy_wait();
				break;
			case 14:
			eeprom_view();	//
			break;
			case 15:
				setbaudrate();	//
				//			siotx_close();
				break;
			case 16:
				senddata();	//
				break;
			case 17:
				flashrom_view();
				break;
			case 18:
				flashrom_write();
				break;
			case 19:
				recvbmp();	//
				break;
			case 20:
				break;
		}
		vram_cls();
		vram_putstrpgm((PGM_P)stretc[2]);
		timer_waitmsec2(800);
	}
}


