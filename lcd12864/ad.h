#define ADCH_X	0
#define ADCH_Y	1

#define JOY_UP		0x01
#define JOY_DOWN	0x02
#define JOY_RIGHT	0x04
#define JOY_LEFT	0x08

char joy_wait(void);
void joy_waitoff(void);
char joy_get(void);
unsigned int ad_get(char chnum);

signed char joy_getx(void);
signed char joy_gety(void);

