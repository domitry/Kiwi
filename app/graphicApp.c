#include "graphicApp.h"

static UCHAR *buffer;
Rect rect;

void drawRect(int x,int y,int width,int height,Color color){
	unsigned char *seek,*end;
	int i;
	seek = buffer + (y*rect.width + x)*3;
	end = buffer + ((y+height-1)*rect.width + x)*3;
	while(seek < end){
		for(i=0;i<width;i++){
			*(seek+2) = color.r;
			*(seek+1) = color.g;
			*(seek) = color.b;
			seek+=3;
		}

		seek += (rect.width - width)*3;
	}
}


void fill(Color color){
	unsigned char *end = buffer+rect.width*rect.height*3;
	unsigned char *i;
	for(i=buffer;i<end;i+=3){
			*(i+2) = color.r;
			*(i+1) = color.g;
			*(i) = color.b;
	}
} 


// カーネルにバッファの位置を通知、eax:3,ebxにバッファアドレス
void tellBuffer(UCHAR *_buffer,int width,int height){
	asm volatile(
		"movl $3,%%eax;movl %0,%%ebx;movl %1,%%ecx;movl %2,%%edx;"::"r"((int)_buffer),"r"(width),"r"(height):"%eax","%ebx","%ecx","%edx"
	);
	//for(;;)asm("hlt");
	asm volatile("int $0x30");
}


// ユーザ空間にmmap、eax:4,ebxにアドレス,ecxにサイズ
void mmap(void *add,unsigned int size){
	asm volatile(
		"mov $4,%%eax;mov %0,%%ebx;mov %1,%%ecx;int $0x30"::"r"((int)buffer),"r"(size)
	);
}

void umemset(void *str,unsigned char c,int size){
	unsigned char *ptr = (unsigned char *)str;
	const unsigned char ch = (const unsigned char)c;
	for(;size>=0;size--,ptr++)
		*ptr = ch;
	return;
}


void initWindow(int width,int height){
	buffer = 0x00400000;
	mmap(0x00400000,0x200000);
	buffer[100] = 1;
	//umemset(buffer,0,10);
	//for(;;)asm("hlt");

	tellBuffer(buffer,width,height);//kernelにどこがバッファか教える
	rect.width = width;
	rect.height = height;
}
