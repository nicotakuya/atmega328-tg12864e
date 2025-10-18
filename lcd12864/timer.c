//-------------------------------------------------
// timer control
//  by takuya matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "timer.h"
#include "siotx.h"
#include "vram.h"
#include "lcd.h"

volatile int timermsec=0;

//--------------------------
void timer_init(void)
{
	timermsec=0;
}

extern unsigned char tick;
volatile unsigned char tickbak=0;

void sleepsub(void){
	sleep_mode();
}

//------------------------
void timer_control(void)
{
//	sleepsub();
	if(tick != tickbak){
		tickbak = tick;
		if(timermsec > 0){
			timermsec -= 16;
			if(timermsec < 0){
				timermsec = 0;
			}
		}
	}
}


//------------------------
//引数：ウエイト時間　1msec単位
void timer_waitmsec(int msec)
{
	timermsec = msec;

	while(timermsec){
		timer_control();
	}
}

