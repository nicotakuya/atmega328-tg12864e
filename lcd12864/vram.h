//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
#define VRAMWIDTH  128     // VRAMドット幅
#define VRAMHEIGHT 64   // VRAMドット高さ
#define VRAMXMIN  0
#define VRAMXMAX  (VRAMWIDTH-1)
#define VRAMYMIN  0
#define VRAMYMAX  (VRAMHEIGHT-1)
#define VRAMSIZE   ((VRAMWIDTH/8)*VRAMHEIGHT)
#define VRSPSIZE 7

extern unsigned char vram[VRAMSIZE];	 // Bitmap Buffer

//--------------------------------------------------------------------------------
// VRAM初期化
// ・VRAMの内容を消します。
void vram_cls(void);


//--------------------------------------------------------------------------------
//ピクセル描画
//引数x：X座標
//引数x：Y座標
//引数color：カラーコード。0=消灯、1=点灯、2=XOR1
//使用例：vram_pset(5,8,1);   // 座標(5,8)に点を描きます。
void vram_pset(int x,int y,char color);

//--------------------------------------------------------------------------------
//ライン描画
//引数x1：X1座標
//引数y1：Y1座標
//引数x2：X2座標
//引数y2：Y2座標
//引数color：カラーコード。0=消灯、1=点灯、2=XOR1
void vram_line(int x1 ,int y1 ,int x2 ,int y2 ,char color);

//--------------------------------------------------------------------------------
// 画面スクロール
// ・VRAMの中身を強制的にシフトします。
// 引数x1: X方向移動量。
// 引数y1: Y方向移動量。
void vram_scroll(char x1,char y1);
void vram_locate(unsigned char x,unsigned char y);

char vram_point(int x,int y);

void vram_putch(char ch);

void vram_putdec(unsigned int x);
void vram_puthex(unsigned char x);
void vram_puthexw(unsigned int x);
void vram_putstr(char *p);
void vram_putstrpgm(PGM_P p);

void vram_spput(char x,char y, PGM_P p);
void vram_spclr(char x,char y);
void vram_bmpput(char x,char y,char w,char h, PGM_P p);

int fnc_abs(signed int a);
signed int fnc_sgn(signed int a);

char vram_gettate(void);
void vram_settate(char tate);
unsigned char vram_getw(void);
unsigned char vram_geth(void);
