#ifndef FONT_H
#define FONT_H

#define INF 0xfff
#define FONT_HEADER_SIZE 19
#define CHAR_HEADER_SIZE 12
#define BUFFER_SIZE 7500

//ãÍì˜ÇÃçÙ
#include "graphic.h"

struct FontHeader{
	const char sign[5];
	char first_char;
	char last_char;
	char font_name[12];
}__attribute__((packed));
typedef struct FontHeader FontHeader;

struct CharHeader{
	unsigned char width;
	unsigned char height;
	unsigned char baseline;
	unsigned char empty;
	unsigned int offset;
	unsigned short size;
	unsigned char question[2];
}__attribute__((packed));
typedef struct CharHeader CharHeader;


typedef struct FontInfo{
	char name[12];
	char first,last;
	void *fh_start;
	CharHeader *ch_start;
	void *data_start;
	void *buffer;
	int b_size;
}FontInfo;

typedef unsigned int UINT;
typedef unsigned char UCHAR;


void* loadFont(char *filename);
_Bool drawFont(int x,int y,char ch,Color col);
_Bool drawString(int x,int y,const char *str,Color col);

#endif
