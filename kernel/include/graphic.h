#ifndef GRAPHIC_H
#define GRAPHIC_H

#define VBE_INFO 0xc0007b00
#define VRAM_START_VIR	0xc1400000	//カーネル＋予備の20MBの後ろすぐにマップ

#define IH_SIZE 20

enum IMG_MODE{IMG_RGB,IMG_RGBA};

typedef unsigned int UINT;
typedef unsigned char UCHAR;


struct VbeInfo{
	unsigned short mode_info;
	unsigned char a_mode_info;
	unsigned char b_mode_info;
	unsigned short win_add_measure;
	unsigned short win_size;
	unsigned short a_start;
	unsigned short b_start;
	unsigned int real_mode;
	unsigned short bytes_per_line;
	unsigned short width;
	unsigned short height;
	unsigned char empty0[3];
	unsigned char bit_per_pic;
	unsigned char empty1;
	unsigned char mem_model;	//0x04パレット,0x06ダイレクトカラー
	unsigned char empty2[12];
	unsigned int vram_add;
	unsigned int vram_offset;
	unsigned short out_vram_size;
}__attribute__((packed));


typedef struct{
	const char sign[4];
	UINT width;
	UINT height;
	UINT size;
	UCHAR type;
	UCHAR trans_color[3];
}__attribute__((packed))ImageHeader;

typedef struct{
	UCHAR byte;
	UCHAR r;
	UCHAR g;
	UCHAR b;
}Pallet;

typedef struct{
	UINT x,y;
	UINT width,height;
}Rect;


typedef struct{
	UINT cnt;
	UCHAR color[3];
}__attribute__((packed))Pixel;

typedef struct VbeInfo VbeInfo;

typedef struct VramInfo{
	unsigned char *start;
	int width;
	int height;
	char bit_per_pic;
	char line;
	unsigned char depth;
}VramInfo;

typedef struct Color{
	unsigned char r,g,b;
}Color;


void initGraphic();
void drawRect(int x,int y,int width,int height,Color color);
void fill(Color color);
void drawImage(UCHAR *image,int x,int y,int width,int height);
void drawImageTrans(UCHAR *image,int x,int y,int width,int height,Color trans);
void drawImage2(UCHAR *image,int x,int y,int width,int height,UCHAR mode);
void* loadImage(char *filename);
void* loadImage256(char *filename);
VramInfo getGraphInfo();


void drawRectInBuffer(UCHAR *buffer,Rect rect,int x,int y,int width,int height,Color color);
void fillInBuffer(UCHAR *buffer,Rect rect);

#endif
