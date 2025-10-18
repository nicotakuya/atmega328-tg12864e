// random
// by takuya matsubara

#include <avr/io.h>

unsigned char rand_temp=0;

void rand_init(void)
{
    TCCR0A= 0;    // タイマ モード 
    TCCR0B= 1;    // タイマ プリスケーラ 
}


unsigned char rand_get(unsigned char max)
{
	rand_temp += rand_temp;
	rand_temp ++;
	rand_temp += TCNT0;

	return(rand_temp % max);  // タイマ カウントを乱数にします
}
