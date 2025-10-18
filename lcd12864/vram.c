//--------------------------------------------------------------------
// VRAM control
//  by Takuya Matsubara

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "vram.h"
#include "sfont.h"	//4x6スモールフォント

char tategamen=0;
unsigned char text_x=0;  // printf用カーソル位置
unsigned char text_y=0;
unsigned char vramwidth=128;
unsigned char vramheight=64;
unsigned char vram[VRAMSIZE];	 // VRAM Bitmap Buffer

//---------------------------------------------------------------------
char vram_gettate(void)
{
	return(tategamen);
}

//---------------------------------------------------------------------
void vram_settate(char tate)
{
	tategamen = tate;

	if(tategamen & 1){
		vramwidth=64;
		vramheight=128;
	}else{
		vramwidth=128;
		vramheight=64;
	}
}

//---------------------------------------------------------------------
unsigned char vram_getw(void)
{
	return(vramwidth);
}

//---------------------------------------------------------------------
unsigned char vram_geth(void)
{
	return(vramheight);
}

//---------------------------------------------------------------------
void vram_cls(void)
{
	int i;

	for(i=0; i<VRAMSIZE; i++){
		vram[i] = 0x00;
	}
	text_x = 0;
	text_y = 0;
}

//---------------------------------------------------------------------
//ピクセル取得
//引数x：X座標。
//引数x：Y座標。
char vram_point(int x,int y)
{
	unsigned char mask;
	int i;

	if(x<0)return(0);
	if(y<0)return(0);
	if(x>=vramwidth)return(0);
	if(y>=vramheight)return(0);

	switch(tategamen){
	case 0:
		break;
	case 1:
		i=y;
		y=x;
		x=127-i;
		break;
	case 2:
		y=63-y;
		x=127-x;
		break;
	default:
		i=y;
		y=63-x;
		x=i;
		break;
	}

	mask = 1 << (y & 7);
	i = ((int)(y / 8)*VRAMWIDTH)+ x;

	return((vram[i] & mask)!= 0);
}

//---------------------------------------------------------------------
//ピクセル描画
//引数x：X座標。
//引数x：Y座標。
//引数color：カラーコード。0=消灯、1=点灯、2=XOR1
void vram_pset(int x,int y,char color)
{
	unsigned char mask;
	int i;

	if(x<0)return;
	if(y<0)return;
	if(x>=vramwidth)return;
	if(y>=vramheight)return;

	switch(tategamen){
	case 0:
		break;
	case 1:
		i=y;
		y=x;
		x=127-i;
		break;
	case 2:
		y=63-y;
		x=127-x;
		break;
	default:
		i=y;
		y=63-x;
		x=i;
		break;
	}

	mask = 1 << (y & 7);
	i = ((int)(y / 8)*VRAMWIDTH)+ x;

	if(color==1)
		vram[i] |= mask;
	else if(color==0)
		vram[i] &= ~mask;
	else if(color==2)
		vram[i] ^= mask;

}

//--------------------------------------------------------------------------------
//ライン描画
// x1,y1：開始座標
// x2,y2：終了座標
// color：カラーコード。0=消灯、1=点灯（赤色）、2=XOR1
void vram_line(int x1 ,int y1 ,int x2 ,int y2 ,char color)
{
	int xd;    // X2-X1座標の距離
	int yd;    // Y2-Y1座標の距離
	int xs=1;  // X方向の1pixel移動量
	int ys=1;  // Y方向の1pixel移動量
	int e;

	xd = x2 - x1;	 // X2-X1座標の距離
	if(xd < 0){
		xd = -xd;	 // X2-X1座標の絶対値
		xs = -1;	  // X方向の1pixel移動量
	}
	yd = y2 - y1;	 // Y2-Y1座標の距離
	if(yd < 0){
		yd = -yd;	 // Y2-Y1座標の絶対値
		ys = -1;	  // Y方向の1pixel移動量
	}
	vram_pset (x1, y1 ,color); //ドット描画
	e = 0;
	if( yd < xd ) {
		while( x1 != x2) {
			x1 += xs;
			e += (2 * yd);
			if(e >= xd) {
				y1 += ys;
				e -= (2 * xd);
			}
			vram_pset (x1, y1 ,color); //ドット描画
		}
	}else{
		while( y1 != y2) {
			y1 += ys;
			e += (2 * xd);
			if(e >= yd) {
				x1 += xs;
				e -= (2 * yd);
			}
			vram_pset (x1, y1 ,color); //ドット描画
		}
	}
}

//--------------------------------------------------------------------------------
// 画面スクロール
// ・VRAMの中身を強制的にシフトします。
// 引数x1: X方向移動量。
// 引数y1: Y方向移動量。
void vram_scroll(char x1,char y1)
{
	int x,y,xa,ya;
	int w,h;
	unsigned char c;

	if(y1<0){
		ya=-1;
		y=vramheight-1;
	}else{
		ya=1;
		y=0;
	}
	h=vramheight;
	while(h--){
		w=vramwidth;
		if(x1<0){
			xa=-1;
			x=vramwidth-1;
		}else{
			xa=1;
			x=0;
		}

		while(w--){
			c = vram_point(x+x1,y+y1);
			vram_pset(x, y ,c);	//ドット描画
			x += xa;
		}

		y += ya;
	}
}

//--------------------------------------------------------------------
// カーソル位置設定
// 引数x：X座標。一定以上だと自動改行。
// 引数y：Y座標。一定以上だと下へ画面スクロールします。
void vram_locate(unsigned char tx,unsigned char ty)
{
	text_x = tx;
	text_y = ty;
}

//---------------------------------------------------------------------
// 1キャラクタをVRAM転送
// ・4x6スモールフォント用
// 引数ch：キャラクターコード（0x00-0xff）
// 戻り値：0を返します。
void vram_putch(char ch)
{
	unsigned char tx,ty,col;
	unsigned char i,j;
	unsigned char bitdata;
	PGM_P p;

	if((ch==10)||(text_x > (vramwidth-(4*2)))){
		text_x = 0;
		text_y += 6*2;
	}
	ty=(vramheight-(6*2));
	if(text_y > ty){
		vram_scroll(0,text_y-ty);
		text_y = ty;
	}
	if((unsigned char)ch < 0x20)return;

	p = (PGM_P)smallfont;
	p += ((int)((unsigned char)ch - 0x20) * 3);

	ty = text_y;
	for(i=0 ;i<6 ;i++) {
		bitdata = pgm_read_byte(p);
		if((i % 2)==0){
			bitdata >>= 4;
		}else{
			p++;
		}
		bitdata &= 0xf;
		tx = text_x;
		for(j=0;j<4;j++){
			col = ((bitdata & 0b1000)!=0);
			vram_pset(tx  ,ty  ,col);
			vram_pset(tx+1,ty  ,col);
			vram_pset(tx  ,ty+1,col);
			vram_pset(tx+1,ty+1,col);
			bitdata <<= 1;
			tx += 2;
		}
		ty += 2;
	}
	text_x += 8;    // カーソル移動
	if((unsigned char)ch>=0xA0)
		text_x += 2;    // カーソル移動(カタカナ用)

}


//------------------------------------------------------16進数表示
void vram_puthexw(unsigned int x)
{
	vram_puthex(x >> 8);
	vram_puthex(x & 0xFF);
}

//-------------------------------------------------
void vram_putdec(unsigned int x)
{
    unsigned char ch;
    unsigned int shift=10000;
	
	while(shift > 0){
		ch = (x / shift) % 10;
		ch += '0';
		vram_putch(ch);
		shift /= 10;
	}
}

//-------------------------------------------------
void vram_puthex(unsigned char x)
{
    unsigned char ch;
    unsigned char shift=0x10;
	
	while(shift > 0){
		ch = (x / shift) % 0x10;
		if(ch < 10){
			ch += '0';
		}else{
			ch += ('A'-10);
		}
		vram_putch(ch);
		shift /= 0x10;
	}
}

//------------------------------------------------------文字列表示
void vram_putstr(char *p)
{
	while(*p != 0){
		vram_putch(*p++);
	}
}

//---------------------------------------------------------------------
// キャラクターを表示
void vram_spput(char x,char y, PGM_P p)
{
	char i,j;
	unsigned char dat;

	x -= (VRSPSIZE/2);
	y -= (VRSPSIZE/2);

	for(j=0; j<VRSPSIZE; j++){
		dat = pgm_read_byte(p++);
		for(i=0; i<VRSPSIZE; i++){
			if(dat & (1<<(VRSPSIZE-1))){
				vram_pset(x+i,y+j,1);
			}
			dat <<= 1;
		}
	}
}

//---------------------------------------------------------------------
void vram_bmpput(char x,char y,char w,char h, PGM_P p)
{
	char i,j,x2,y2;
	unsigned char dat=0;

	for(j=0; j<h; j++){
		for(i=0; i<w; i++){
			if((i % 8)==0){
				dat = pgm_read_byte(p++);
			}
			if(dat & 0x80){
				x2=x+i*2;
				y2=y+j*2;
				vram_pset(x2  ,y2  ,1);
				vram_pset(x2+1,y2  ,1);
				vram_pset(x2  ,y2+1,1);
				vram_pset(x2+1,y2+1,1);
			}
			dat <<= 1;
		}
	}
}
//-------------------------------------------------
void vram_spclr(char x,char y)
{
	char i,j;

	x -= (VRSPSIZE/2);
	y -= (VRSPSIZE/2);

	for(j=0; j<VRSPSIZE; j++){
		for(i=0; i<VRSPSIZE; i++){
			vram_pset(x+i,y+j,0);
		}
	}
}

//-------------------------------------------------
int fnc_abs(signed int a){
	if(a<0)a = -a;
	return (a);
}

//-------------------------------------------------
signed int fnc_sgn(signed int a){
	if(a<0)return(-1);
	return (1);
}

//---------------------------------------------------------------------
void vram_putstrpgm(PGM_P p)
{
	char ch;

	while(1){
		ch = pgm_read_byte(p++);
		if(ch==0)
			break;

		vram_putch(ch);
	}
}
