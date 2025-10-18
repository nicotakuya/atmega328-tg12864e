//---------------------------------------------------------------------
// sin/cos
// by takuya matsubara

#include <avr/pgmspace.h>
#include "vect.h"

/*----------------ベクトル用構造体*/
//typedef struct {
//    char  dx;    /* X座標ベクトル */
//    char  dy;    /* Y座標ベクトル */
//} VECT;

//---------------------------------------------------------------------
const unsigned char vecttable[] PROGMEM ={
  0,/* 0 degrees*/
  4,/* 1 degrees*/
  9,/* 2 degrees*/
 13,/* 3 degrees*/
 18,/* 4 degrees*/
 22,/* 5 degrees*/
 27,/* 6 degrees*/
 31,/* 7 degrees*/
 35,/* 8 degrees*/
 40,/* 9 degrees*/
 44,/* 10 degrees*/
 48,/* 11 degrees*/
 53,/* 12 degrees*/
 57,/* 13 degrees*/
 61,/* 14 degrees*/
 66,/* 15 degrees*/
 70,/* 16 degrees*/
 74,/* 17 degrees*/
 78,/* 18 degrees*/
 83,/* 19 degrees*/
 87,/* 20 degrees*/
 91,/* 21 degrees*/
 95,/* 22 degrees*/
 99,/* 23 degrees*/
103,/* 24 degrees*/
107,/* 25 degrees*/
111,/* 26 degrees*/
115,/* 27 degrees*/
119,/* 28 degrees*/
123,/* 29 degrees*/
127,/* 30 degrees*/
131,/* 31 degrees*/
135,/* 32 degrees*/
138,/* 33 degrees*/
142,/* 34 degrees*/
146,/* 35 degrees*/
149,/* 36 degrees*/
153,/* 37 degrees*/
156,/* 38 degrees*/
160,/* 39 degrees*/
163,/* 40 degrees*/
167,/* 41 degrees*/
170,/* 42 degrees*/
173,/* 43 degrees*/
176,/* 44 degrees*/
180,/* 45 degrees*/
183,/* 46 degrees*/
186,/* 47 degrees*/
189,/* 48 degrees*/
192,/* 49 degrees*/
195,/* 50 degrees*/
197,/* 51 degrees*/
200,/* 52 degrees*/
203,/* 53 degrees*/
205,/* 54 degrees*/
208,/* 55 degrees*/
211,/* 56 degrees*/
213,/* 57 degrees*/
215,/* 58 degrees*/
218,/* 59 degrees*/
220,/* 60 degrees*/
222,/* 61 degrees*/
224,/* 62 degrees*/
226,/* 63 degrees*/
228,/* 64 degrees*/
230,/* 65 degrees*/
232,/* 66 degrees*/
234,/* 67 degrees*/
236,/* 68 degrees*/
237,/* 69 degrees*/
239,/* 70 degrees*/
240,/* 71 degrees*/
242,/* 72 degrees*/
243,/* 73 degrees*/
244,/* 74 degrees*/
245,/* 75 degrees*/
246,/* 76 degrees*/
247,/* 77 degrees*/
248,/* 78 degrees*/
249,/* 79 degrees*/
250,/* 80 degrees*/
251,/* 81 degrees*/
252,/* 82 degrees*/
252,/* 83 degrees*/
253,/* 84 degrees*/
253,/* 85 degrees*/
253,/* 86 degrees*/
254,/* 87 degrees*/
254,/* 88 degrees*/
254,/* 89 degrees*/
255 /* 90 degrees*/
};

//               0度(0,-255)
//               |
// (-255,0)270度-+-90度(255,0)
//               |
//              180度(0,255)

//---------------------------------------------------------------------
// sin風関数
// 引数 a:角度（単位は度）。
// 戻り値：sin値。-255～255の整数で返します。
int vect_y1(int a)
{
	PGM_P p = (PGM_P)vecttable;

	while(a < 0){
		a+=360;
	}
	while(a >= 360){
		a-=360;
	}

	if(a >= 270){              //270～359degrees
		a-=270;
		return(-pgm_read_byte(p+a));
	}
	if(a >= 180){              //180～269degrees
		a-=180;
		return(pgm_read_byte(p+(90-a)));
	}
	if(a >= 90){               //90～179degrees
		a-=90;
		return(pgm_read_byte(p+a));
	}
	return(-pgm_read_byte(p+(90-a)));  //0～90degrees
}

//---------------------------------------------------------------------
// cos風関数
// 引数 a:角度（単位は度）。
// 戻り値：cos値。-255～255の整数で返します。
int vect_x1(int a)
{
	return(vect_y1(a+90));
}



