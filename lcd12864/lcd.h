
void lcd_init(void);

//画面更新（LCD用タイマ割り込み）を許可／禁止
void lcd_redraw(char enable);


void lcd_control(void);
void lcd_sendall(void);

