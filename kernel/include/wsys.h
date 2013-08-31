#include "graphic.h"

#ifndef WSYS_H
#define WSYS_H

#define LAYER_NUM 20
#define BACK_NUM 19

#define WIN_BAR_HEIGHT 26
#define WIN_EDGE 2
#define BUTTON_WIDTH 110
#define BUTTON_HEIGHT 22

#define IF_UP_DOWN 0x10
#define IF_RIGHT_LEFT 0x20
#define UP 0x10
#define LEFT 0x20

typedef unsigned int UINT;
typedef unsigned char UCHAR;

typedef struct{
	UINT x,y;
	UINT width,height;
	UCHAR type;//not present->0x00,cursor->0x01,back->0x02,window->0x03
	UCHAR *context;
	UCHAR is_context_exist;
}Layer;

typedef struct{
	UCHAR *cursor;
	UCHAR *button;
	UCHAR *back;
	UCHAR *caution;
	Layer layers[LAYER_NUM];
}WsysInfo;

void proceed();
void initWsys();
void drawCursor();
void drawWindow(Rect rect);
void setLayer(int num,int x,int y,int width,int height,UCHAR type);
void setContext(int num,void *context);
void newWindow(UCHAR *buffer,int width,int height);

#endif
