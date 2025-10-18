//-----------------------------------------------------------------------
//  sio tx
//   by Takuya Matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "siotx.h"
#include "vram.h"

#ifndef F_CPU
#define   F_CPU   8000000	//CPUクロック周波数 8MHz
#endif

unsigned char txtcnt=0;

#if F_CPU<=8000000

#define  PS2400	64	   //プリスケーラ値timer0用
#define  PS2400R   3		//プリスケーラ設定値timer0用
#define  PS9600	8		//プリスケーラ値timer0用
#define  PS9600R   2		//プリスケーラ設定値timer0用
#define  PS38400   1		//プリスケーラ値timer0用
#define  PS38400R  1		//プリスケーラ設定値timer0用
#define  PS57600   1		//プリスケーラ値timer0用
#define  PS57600R  1		//プリスケーラ設定値timer0用
#define  PS115200  1		//プリスケーラ値timer0用
#define  PS115200R 1		//プリスケーラ設定値timer0用

#else

#define  PS2400	  64	   //プリスケーラ値timer0用
#define  PS2400R   3		//プリスケーラ設定値timer0用
#define  PS9600   64 		//プリスケーラ値timer0用
#define  PS9600R   3		//プリスケーラ設定値timer0用
#define  PS38400   8		//プリスケーラ値timer0用
#define  PS38400R  2		//プリスケーラ設定値timer0用
#define  PS57600   8		//プリスケーラ値timer0用
#define  PS57600R  2		//プリスケーラ設定値timer0用
#define  PS115200  1		//プリスケーラ値timer0用
#define  PS115200R 1		//プリスケーラ設定値timer0用

#endif

#define SIOTX_PORT PORTB	  //ポート
#define SIOTX_DDR  DDRB	   //ポート入出力設定
#define SIO_TX   (unsigned char)(1<<3)		  //送信ビット

//#define TX_MARK  SIOTX_DDR&=~SIO_TX	//MARK(1)待機
//#define TX_SPACE SIOTX_DDR|=SIO_TX	//SPACE(0)

#define TX_MARK		SIOTX_PORT|=SIO_TX		//MARK(1)待機
#define TX_SPACE	SIOTX_PORT&=~SIO_TX	//SPACE(0)

#define SIOTX_TIFR	TIFR0	//タイマフラグ
#define SIOTX_TOV	(1<<TOV0)	//タイマフラグ
#define SIOTX_TCNT	TCNT0

#define SIOTX_TCNT2400	((F_CPU/PS2400)/2400)
#define SIOTX_TCNT9600	((F_CPU/PS9600)/9600)
#define SIOTX_TCNT38400   ((F_CPU/PS38400)/38400)
#define SIOTX_TCNT57600   ((F_CPU/PS57600)/57600)
#define SIOTX_TCNT115200  ((F_CPU/PS115200)/115200)

char siotx_open=0;

void sio_bitwait(void);

//-----------------------------------------------------------------------
// シリアルポート初期化
//*タイマ0使用
void siotx_init(char baud)
{
	SIOTX_DDR |= SIO_TX;

	TX_MARK;	//待機

	TCCR0A = 0;
	
	switch(baud){
	case 0:
		TCCR0B = PS2400R;	   // タイマ プリスケーラ設定
		txtcnt = SIOTX_TCNT2400;
		break;
	case 1:
		TCCR0B = PS9600R;	   // タイマ プリスケーラ設定
		txtcnt = SIOTX_TCNT9600;
		break;
	case 2:
		TCCR0B = PS38400R;	   // タイマ プリスケーラ設定
		txtcnt = SIOTX_TCNT38400;
		break;
	case 3:
		TCCR0B = PS57600R;	   // タイマ プリスケーラ設定
		txtcnt = SIOTX_TCNT57600;
		break;
	case 4:
		TCCR0B = PS115200R;	   // タイマ プリスケーラ設定
		txtcnt = SIOTX_TCNT115200;
		break;
	}
//   TCCR1A= 0;				// タイマ モード 
//   TCCR1B= PRESCALECR;	   // タイマ プリスケーラ設定
	siotx_open=1;
}

//-----------------------------------------------------------------------
void siotx_close(void){
	SIOTX_PORT &= ~SIO_TX;
	SIOTX_PORT |= SIO_TX;
	siotx_open=0;
}

//-----------------------------------------------------------------------
// 1バイト送信
void sio_tx(char sio_txdat)
{
	unsigned int databuf;
	char bitcnt = 11;	//startbit + stopbit + 8

	if(siotx_open==0)return;

	databuf = (unsigned int)sio_txdat;
	databuf |= 0xFF00;	//stopbit
	databuf <<= 1;		//startbit
	cli();

	//データビット0-7 下位から送信
	while(bitcnt--){	
		SIOTX_TCNT = 0;
		if(databuf & 1)
			TX_MARK;	//1
   	 	else
			TX_SPACE;	//0

		databuf >>= 1;

		while(SIOTX_TCNT < txtcnt){
		}
	}
	sei();
}

//--------------------------------
#if 0
void sio_tx(char sio_txdat)
{
	struct PB{
		char :1;		//0
		char :1;		//1
		char :1;		//2
		char TXBIT:1;	//3
		char :1;		//4
		char :1;		//5
		char :1;		//6
		char :1;		//7 
	};
	#define pob (*(volatile struct PB*)&PORTB)

	struct BITWORK{
		char LSB:1;	//0
		char :1;	//1
		char :1;	//2
		char :1;	//3
		char :1;	//4
		char :1;	//5
		char :1;	//6
		char :1;	//7 
	};
	typedef struct{
		struct BITWORK bl;
		unsigned char bh;
	}stword;
	union{
		unsigned int tmpword;
		stword  wd; 
	}wk;

	char bitcnt = 11;

	if(siotx_open==0)return;

	wk.tmpword = (unsigned int)sio_txdat;
	wk.tmpword  |= 0xFF00;	//stopbit
	wk.tmpword  <<= 1;		//startbit

	//データビット0-7 下位から送信
	while(bitcnt--){	
		SIOTX_TCNT = 0;
		pob.TXBIT = wk.wd.bl.LSB;;

		wk.tmpword  >>= 1;

		while(SIOTX_TCNT < txtcnt){
		}
	}
}

//-----------------------------------------------------------------------
// 文字列送信
void sio_txstr(char *str)
{
	char ch;

	while(*str != 0)
	{
		ch = *str++;
		sio_tx(ch);
		if(ch==13)
				sio_tx(10);
	}
}

//---------------------------------------
// 10進数2桁 シリアル送信
void sio_txhex(unsigned char x)
{
	unsigned char ch;
	char shift=4;
	
	while(shift >= 0){
		ch = (x >> shift) & 0xf;
		if(ch < 10){
			ch += '0';
		}else{
			ch += ('A'-10);
		}
		sio_tx(ch);
		shift -= 4;
	}
	sio_tx(' ');
}

//---------------------------------------
// 10進数4桁 シリアル送信
void sio_txdec(int x)
{
	unsigned char temp;
	int ketaval=1000;

	if(x < 0){
		x = -x;
		sio_tx('-');
	}

	while (ketaval != 0) {
		temp = x / ketaval;
		if(temp > 9) temp=9;
		sio_tx('0'+temp);
		x -= (ketaval*temp);
		ketaval /= 10;
	}
}
#endif
