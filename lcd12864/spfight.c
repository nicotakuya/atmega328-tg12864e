//-------------------------------------------------
// space fight 
//  for ATmega328p
//  by takuya matsubara

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "vram.h"
#include "lcd.h"
#include "rand.h"
#include "timer.h"
#include "spfight.h"
#include "ad.h"

#define TEKIMAX 15

const unsigned char pat[][7] PROGMEM = {
	{
		0b0010100,
		0b0010100,
		0b0011100,
		0b0011100,
		0b1111111,
		0b1111111,
		0b1011101
	},{
		0b0111110,
		0b1111111,
		0b1000001,
		0b1111111,
		0b1111111,
		0b1100011,
		0b0110110
	},{
		0b0111110,
		0b1111111,
		0b1100011,
		0b1111111,
		0b1111111,
		0b0110110,
		0b1100011
	},{
		0b0110110,
		0b0011100,
		0b0011100,
		0b0011100,
		0b0011100,
		0b0011100,
		0b0001000
	},{
		0b1001001,
		0b0101010,
		0b0000000,
		0b1100011,
		0b0000000,
		0b0101010,
		0b1001001
	},{
		0b0001000,
		0b0011100,
		0b0011100,
		0b0011100,
		0b0011100,
		0b0011100,
		0b0110110
	}
};

const unsigned char strready[] PROGMEM ="READY";
const unsigned char strover[] PROGMEM ="GAMEOVER";

//---------------------------------------------------------------------
// メイン処理
void spfight(void)
{
#define MOVEPITCH 30	//移動カウント
#define MOVESEQ   (10+10+2+2)	//移動シーケンス

	signed char tx[TEKIMAX];	//敵座標
	signed char ty[TEKIMAX];
	char tb[TEKIMAX];
	char ttime;	//Enemy Timing
	char tmove;	//Enemy Moving
	char speed=0;	//game speed
	int score=0;	//Score
	char tnum;	//Enemy Count
	signed char ax=0,ay;	//My Ship X,Y
	signed char bx,by;	//My Ship Beam
	signed char cx,cy;	//Enemy Beam
	signed char x,y;
	int i;
	char overflag=1;

	while(1){
		vram_cls();				// LED初期化
		if(overflag){
			overflag=0;
			vram_locate(12,22);
			vram_putstrpgm((PGM_P)strready);
			score=0;
			speed=MOVEPITCH;
			ax=32;	//プレーヤ座標
			ay=60;
		}
		vram_spput(ax, ay, (PGM_P)pat[0]);		//------自機をvramに転送
		bx=by=-1;		//自弾座標
		cx=cy=-1;		//敵弾座標

		for(i=0;i<TEKIMAX;i++){
			tx[i] = ((i % 4)*12)+8;
			ty[i] = ((i / 4)*12)+4;
			tb[i] = 0;
		}

		tmove=0;
		ttime=0;

		timer_waitmsec(1000);

		while(overflag==0){
			lcd_redraw(0);
			vram_cls();	
			ax += joy_getx()/60;
			if(ax<4)ax=4;
			if(ax>60)ax=60;
			vram_spput(ax, ay, (PGM_P)pat[0]);		//------自機をvramに転送

			if(by <= -1){
				bx=ax;
				by=ay;
			}else{
				by-=2;
				vram_spput(bx,by,(PGM_P)pat[5]);
			}

			//--------------------Enemy Beam
			if(cy <= -1){
				i=rand_get(TEKIMAX);
				if(ty[i] != -1){
					cx = tx[i];
					cy = ty[i];
				}
			}else{
				cy++;
				if(cy > VRAMYMAX){
					cy = -1;
				}else{
					vram_spput(cx,cy,(PGM_P)pat[3]);
				}
			}

			ttime = (ttime+1)%speed;
			if (ttime==0) {
				tmove=(tmove+1) % MOVESEQ;
			}

			tnum=0;
			for(i=0 ;i<TEKIMAX ;i++) {
				x=tx[i];
				y=ty[i];
				if(y == -1)continue;
				tnum++;

				if(tb[i]){
					tb[i]--;
					vram_spput(x,y,(PGM_P)pat[4]);
					if(tb[i]==0)
						ty[i]=-1;
					continue;
				}
				if((fnc_abs(by-y)<5)&&(fnc_abs(bx-x)<5)){ //命中
					if(score < 9999)score++;
					if(speed > 2)speed-=1;
					tb[i]=15;
					by=-1;
					continue;
				}
				if(ttime==0){
									//ジグザグ移動
					if(tmove < 10){//01234
						x+=2;
					}else if(tmove < (10+2)){//5
						y+=2;
					}else if(tmove < (10+2+10)){//6789a
						x-=2;
					}else{
						y+=2;
					}
					if(y >= VRAMYMAX)
						overflag=1;
				}
				tx[i]=x;
				ty[i]=y;
				vram_spput(x,y ,(PGM_P)pat[1+(tmove & 1)]);
				continue;
			}
			if((fnc_abs(cy-ay)<5)&&(fnc_abs(cx-ax)<5)){
				overflag=1;
			}

			lcd_redraw(1);
			timer_waitmsec(20);
			if(tnum==0){	//敵全滅
				speed+=8;
				break;
			}
		}
		if(overflag){	//game over
			vram_spclr(ax,ay);
			vram_spput(ax, ay, (PGM_P)pat[4]);		//------自機をvramに転送
			vram_locate(0,12);
			vram_putstrpgm((PGM_P)strover);
			vram_locate(4,32);
			vram_putdec(score);
			timer_waitmsec(3000);
			break;
		}
	}
}

