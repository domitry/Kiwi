#include <stdio.h>
#include "wsys.h" 
#include "graphic.h"
#include "interp.h"
#include "font.h"

static WsysInfo wsys;
static Time cur_time;
static char time_str[6];
InterpBuffer *key,*mouse;
void* buffer_addr_k = (void *)(VRAM_START_VIR + 0x400000*2);


void setLayer(int num,int x,int y,int width,int height,UCHAR type){
	if(num<0)return;
	wsys.layers[num].x=x;
	wsys.layers[num].y=y;
	wsys.layers[num].width=width;
	wsys.layers[num].height=height;
	wsys.layers[num].type=type;	
}

void setContext(int num,void *context){
	wsys.layers[num].context = context;
	wsys.layers[num].is_context_exist = 1;
}

// newWindowシステムコール時に呼ばれる
void registerBuffer(UCHAR *buffer_u,int width,int height){
	void *buffer_phys = (void *)kmmap(buffer_u,0x200000);
	//for(;;)asm volatile("hlt"::"a"(buffer_phys));
	kmmap_phys(buffer_phys,(void *)buffer_addr_k,0x200000);
	
	setLayer(1,100,200,width,height,3);	//layer生成
	setContext(1,buffer_phys);				//コンテキストの設定
}

void switchLayer(){
}

void moveLayer(){
	//消す
	
	//書き直す
}

void moveCursor(){
	int x,y;
	MouseStatus ms = {0,0,0,0};
	ms = checkMouse();
	x = wsys.layers[0].x + ms.dx;
	y = wsys.layers[0].y + ms.dy;
	if(x<0)x=0;
	else if(x > wsys.layers[BACK_NUM].width-32)
		x = wsys.layers[BACK_NUM].width-32;
	if(y<0)y=0;
	else if(y > wsys.layers[BACK_NUM].height-32)
		y = wsys.layers[BACK_NUM].height-32;
	wsys.layers[0].x = x;
	wsys.layers[0].y = y;
}

void drawCursor(){
	Color trans = {125,0,156};
	drawImageTrans(wsys.cursor,wsys.layers[0].x,wsys.layers[0].y,32,32,trans);
}
 
void drawBack(Rect rect){
	Color green = {0,191,0};
	Color black = {0,0,0};
	drawRect(0,0,1024,768,green);
	drawImage(wsys.back,31,344,962,424);
	drawRect(0,738,1024,30,black);
}

void drawWindow(Rect rec){
	Color black = {26,31,30};
	Color lime = {0,162,0};
	Color grey = {52,56,56};
	drawRect(rec.x,rec.y,rec.width,WIN_BAR_HEIGHT-WIN_EDGE,black);//タイトルバー
	drawImage(wsys.button,rec.x+rec.width-BUTTON_WIDTH,rec.y,BUTTON_WIDTH,BUTTON_HEIGHT);//ボタン
	drawRect(rec.x,rec.y+WIN_BAR_HEIGHT-WIN_EDGE,rec.width,WIN_EDGE,lime);//上の縁
	drawRect(rec.x,rec.y+WIN_BAR_HEIGHT-WIN_EDGE-2,WIN_EDGE,rec.height-WIN_BAR_HEIGHT+WIN_EDGE+2,black);//左
	drawRect(rec.x+rec.width-WIN_EDGE,rec.y+WIN_BAR_HEIGHT-WIN_EDGE-2,WIN_EDGE,rec.height-WIN_BAR_HEIGHT+WIN_EDGE+2,black);//右
	drawRect(rec.x,rec.y+rec.height-WIN_EDGE,rec.width,WIN_EDGE,black);//下
	//drawRect(rec.x+WIN_EDGE,rec.y+WIN_BAR_HEIGHT,rec.width-WIN_EDGE*2,rec.height-WIN_BAR_HEIGHT-WIN_EDGE,grey);
}

void blueScreen(){
	Color blue = {17,114,169};
	Color white = {255,255,255};
	
	fill(blue);
	drawString(500,370,":(",white);
	drawString(500,430,"Your PC ran into a problem that it could'nt",white);
	drawString(500,450," handle, and now it needs to restart.",white);
	return;
}

void updateContext(){
	Rect win={wsys.layers[1].x,wsys.layers[1].y,wsys.layers[1].width,wsys.layers[1].height};
	if(wsys.layers[1].is_context_exist){
		drawImage(buffer_addr_k,wsys.layers[1].x,wsys.layers[1].y,wsys.layers[1].width,wsys.layers[1].height);
		drawWindow(win);
	}
}

void initWsys(InterpBuffer *_key,InterpBuffer *_mouse){
	int i;
	VramInfo info = getGraphInfo();
	wsys.cursor = loadImage("cursor.img");
	wsys.button = loadImage("button.img");
	wsys.back = loadImage256("back.img");
	wsys.caution = loadImage("caution.img");
	
	setLayer(0,500,300,32,32,0x01);
	for(i=1;i<BACK_NUM;i++)
		setLayer(i,0,0,0,0,0);
	setLayer(BACK_NUM,0,0,info.width,info.height,0x02);
	
	cur_time = getCurrentTime();
}

void drawAlert(const char* message){
	Color white = {255,255,255};
	Color black = {0,0,0};
	drawRect(312,304,400,100,white);
	drawImage(wsys.caution,322,314,86,75);
	drawString(420,340,message,black);
}

void proceed(){
	Rect all={0,0,wsys.layers[BACK_NUM].width,wsys.layers[BACK_NUM].height};
	
	drawBack(all);

	updateContext();

	cur_time = getCurrentTime();
	sprintf(time_str,"%d:%2d",cur_time.hour,cur_time.minute);
	//drawAlert("This is an alert!");
	
	moveCursor();
	drawCursor();
	
	exchangeBuffer();
}
