//-------------------------------------------------
// A/D convert
//  for ATmega328p
//  by takuya matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "ad.h"
#include "timer.h"

//-------------------------------------------------
unsigned int ad_get(char chnum)
{
//	DDRC &= ~(0b11);
//	PORTC &= ~(0b11);

//REFS1 REFS0:00=AREF, internal Vref turned off
//ADC prescaler select bits
	ADCSRA = (1<<ADEN)| 6;
	ADMUX = chnum;	// É`ÉÉÉìÉlÉãê›íË

	ADCSRA |= (1<<ADIF);
	ADCSRA |= (1<<ADSC);	//AD start
	while((ADCSRA & (1<<ADIF))==0){
	}

	return(ADCW);	// A/DÉJÉEÉìÉgíl(0-0x3ff)
}

extern char tategamen;
//-------------------------------------------------
signed char joy_getx(void)
{
	int tmp;
	char channel;

	if(tategamen & 1)
		channel=ADCH_Y;
	else
		channel=ADCH_X;

	tmp = (int)ad_get(channel) / 4;	//0Å`255
	tmp -= 128;	//-128 Å` +127

	if(tategamen>=2)
		tmp = 255-tmp;

	return((signed char)tmp);
}

//-------------------------------------------------
signed char joy_gety(void)
{
	int tmp;
	char channel;

	if(tategamen & 1)
		channel=ADCH_X;
	else
		channel=ADCH_Y;

	tmp = (int)ad_get(channel) / 4;	//0Å`255

	if((tategamen==1)||(tategamen==2))
		tmp = 255-tmp;

	tmp -= 128;	//-128 Å` +127

	return((signed char)tmp);
}

//-------------------------------------------------
char joy_get(void)
{
	signed char tx,ty;
	char joy1,joy2;

	tx = joy_getx();	//ç≈èâÇéÃÇƒÇÈ
	tx = joy_getx();
	if(tx < -64){
		joy1=JOY_LEFT;
	}else if(tx > 64){
		joy1=JOY_RIGHT;
	}else{
		joy1=0;
	}

	ty = joy_gety();	//ç≈èâÇéÃÇƒÇÈ
	ty = joy_gety();
	if(ty < -64){
		joy2=JOY_UP;
	}else if(ty > 64){
		joy2=JOY_DOWN;
	}else{
		joy2=0;
	}

	return(joy1 | joy2);
}

//-------------------------------------------------
void joy_waitoff(void)
{
	char cnt; 

	cnt=0;
	while(cnt < 60){
		if(joy_get()==0)
			cnt++;
		else
			cnt=0;
	}
}

//-------------------------------------------------
char joy_wait(void)
{
	char cnt=0; 
	char joydat=0;

	joy_waitoff();

	while(cnt<3){
		joydat=joy_get();
		if(joydat==0)
			cnt=0;
		else
			cnt++;

		timer_waitmsec(30);
	}
	return(joydat);
}
