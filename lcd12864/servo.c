//-------------------------------------------------------------------
// servo control
//  by Takuya Matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "vram.h"
#include "lcd.h"
#include "timer.h"
#include "servo.h"
#include "ad.h"

#define SERVO_PORT  PORTB    // ポート
#define SERVO_DDR   DDRB     // 入出力設定
#define SERVO_BIT	(1<<2)   // ポート出力用マスク値
#define SERVO_BITHIGH	SERVO_PORT|=SERVO_BIT
#define SERVO_BITLOW	SERVO_PORT&=~SERVO_BIT

#define SERVO_CENTER  1500   //  Signalの中心値[μs]
#define SERVO_CYCLE  16000   //  Signalの周期[μs]

#ifndef F_CPU
#define F_CPU    8000000     // CPUクロック周波数 [Hz]
#endif

#define PRESCALE   8         // プリスケーラ値
#define PRESCALER  2         // プリスケーラレジスタ値
#define SERVO_TIFR  TIFR1    // タイマフラグ
#define SERVO_TCNT  TCNT1    // タイマカウント
#define SERVO_CNTMAX   ((F_CPU/PRESCALE)/(1000000/SERVO_CYCLE))

const unsigned char strservo[] PROGMEM ="ANGLE\n\nDEGREE\n<->:MOVE";

//------------------------------------------------------------------
void servo_ctrl(void)
{
	#define TIMEPARANG 9    // 1度あたりのSignal時間[μs]
	int servo_signal;    // SignalのHレベル時間[μs]
	int servo_count;    // SignalのHレベル時間[カウント]
	char servo_angle=0;	//サーボの角度(-90～0～+90)
	char tmp;
	char joycnt=0;

	TCCR1A = 0;               // タイマ1 モード 
	TCCR1B = PRESCALER;       // タイマ1 プリスケーラ設定
    SERVO_TCNT = 0;

	SERVO_DDR  |= SERVO_BIT;
	SERVO_BITLOW;    //サーボのSignalをLに

	vram_locate(0,0);
	vram_putstrpgm((PGM_P)strservo);

	while(1){
		joycnt=(joycnt+1)%8;
		if(joycnt==0){
			if(joy_gety()/64)break;

			servo_angle += joy_getx()/20;
			if(servo_angle<-90)servo_angle=-90;
			if(servo_angle> 90)servo_angle=90;

			tmp = servo_angle;
			vram_locate(16,12);
			if(tmp<0){
				tmp = -tmp;
				vram_putch('-');
			}else{
				vram_putch('+');
			}
			vram_putch('0'+(tmp / 10));
			vram_putch('0'+(tmp % 10));
		}
	    servo_signal = SERVO_CENTER - ((int)servo_angle*TIMEPARANG);
		servo_count = (int)((F_CPU/PRESCALE)/(1000000L/servo_signal));

	    while(SERVO_TCNT < SERVO_CNTMAX);   // サーボの1周期待ち

		cli();
	    SERVO_TCNT = 0;
		SERVO_BITHIGH;   //サーボのSignalをHに
	    while(SERVO_TCNT < servo_count) {
	    }
	    SERVO_BITLOW;    //サーボのSignalをLに
		sei();
	}
	TCCR1B =0;
}
