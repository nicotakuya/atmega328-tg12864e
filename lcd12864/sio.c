//---------------------------------------------------------------------
// SIO control
// by takuya matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include "sio.h"

#ifndef F_CPU
	#define F_CPU 8000000	// CPUクロック周波数[Hz]
#endif

#define RINGRXSIZE	32	//リングバッファサイズ
char ringrx[RINGRXSIZE];	//受信リングバッファ
unsigned char ringrxc1;
unsigned char ringrxc2;

#define SIO_OPENED	1
#define SIO_CLOSED	0
char sio_open=SIO_CLOSED;

//--------------------------------------------------------------
void sio_close(void)
{
	UCSR0B = 0;
	sio_open=SIO_CLOSED;
}

//--------------------------------------------------------------
void sio_init(long baudrate)
{
	unsigned int ubrr = 0;

	ubrr = ((F_CPU/baudrate/8)-1);

	PORTD |= 1;		//pull up
	DDRD &= 0xFE;	//bit0 を 0

	ringrxc1=0;	//ring buffer counter
	ringrxc2=0;

	UBRR0H = ubrr >> 8;	// baud rate
	UBRR0L = ubrr & 0xff;
	UCSR0A |= (1<<U2X0);	//double speed
	UCSR0B = (1<<RXEN0)|(1<<RXCIE0);
//	UCSR0B = (1<<RXEN0)|(1<<RXCIE0)|(1<<TXEN0);

	sio_open = SIO_OPENED;
}


//---------------------------------------------------------------------
//受信割り込み
SIGNAL(USART_RX_vect)
{
	ringrx[(int)ringrxc1++] = UDR0;	//リングバッファに格納
	if(ringrxc1 >= RINGRXSIZE)
		ringrxc1 = 0;

}

//-----------------------------------------------------------------------
// 1バイト受信
// 戻り値: 受信データ0x00～0xFF
unsigned char sio_rx(void)
{
    unsigned char rxdat;

	while(1)
	{
		if(ringrxc1 != ringrxc2){
			rxdat = ringrx[(int)ringrxc2++];	//リングバッファから取り出し
			if(ringrxc2 >= RINGRXSIZE)
				ringrxc2 = 0;

			return(rxdat);
		}
	}
}

//---------------------------------------------------------------
// 受信データ検出
//・受信データがあるかないかチェックする
// 戻り値:0=受信データなし / 1=受信データあり
char sio_rxcheck(void)
{
	if(sio_open==SIO_OPENED){
		if(ringrxc1 != ringrxc2)
			return(1);
	}
	return(0);
}
