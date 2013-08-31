#include "fs.h"
#include "font.h"
#include "phys_mem.h"
#include "graphic.h" 

static FontInfo f_info;

int max(int a,int b){
	return (a>b?a:b);
}

//一致したときtrue(1)
_Bool strcmp(const char *s1,const char *s2){
	int i=0;
	while(s1[i]!='\0' && s2[i]!='\0' && i<INF){
		if(s1[i]!=s2[i])return 0;
		i++;
	}
	if(s1[i]=='\0' && s2[i]=='\0')return 1;
	else return 0;
}

void strcpy(char *s1,const char *s2){
	int i=0;
	while(s2[i]!='\0' && i<INF){
		s1[i] = s2[i];
		i++;
	}
}

void* loadFont(char *filename){
	FontHeader *start = (FontHeader *)loadFile(filename);
	if(start==0)return 0;
	
	if(!strcmp(start->sign,"FONT"))return 0;
	strcpy(f_info.name,start->font_name);
	
	f_info.first = start->first_char;
	f_info.last = start->last_char;
	f_info.fh_start = (void *)start;
	f_info.ch_start = (CharHeader *)((void *)start + FONT_HEADER_SIZE);
	f_info.data_start = (UCHAR *)f_info.ch_start + CHAR_HEADER_SIZE * (f_info.last-f_info.first+1);
	
	f_info.buffer = kmalloc(BUFFER_SIZE);
	f_info.b_size = BUFFER_SIZE;//今は定数だがいずれはフォントファイルから最大サイズを読み込めるように
	return start;
}

_Bool drawFont(int x,int y,char ch,Color col){
	int i;
	char offset = ch - f_info.first;
	CharHeader *ch_h = f_info.ch_start + offset;
	UCHAR *r_seek,*w_seek,*r_end;
	
	if(ch<f_info.first||ch>f_info.last)return 0;
	
	w_seek = (UCHAR *)f_info.buffer;
	r_seek = (UCHAR *)f_info.fh_start + ch_h->offset;
	r_end = r_seek + ch_h->size;
	
	for(;r_seek < r_end;r_seek+=2){
		for(i=0;i<r_seek[0];i++){
			w_seek[i*4] = col.b;
			w_seek[i*4+1] = col.g;
			w_seek[i*4+2] = col.r;
			w_seek[i*4+3] = r_seek[1];
		}
		w_seek += r_seek[0] * 4;
	}
	drawImage2(f_info.buffer,x,y,ch_h->width,ch_h->height,IMG_RGBA);
	return 1;
}

_Bool drawString(int x,int y,const char *str,Color col){
	int i=0,cur_x=0,num=0,height=0;
	CharHeader ch;

	for(i=0;str[i]!='\0';i++){
		if(i>INF)return 0;
		if(str[i]==' ')continue;
		if(str[i]<f_info.first || str[i]>f_info.last)return 0;
		height = max(height,(f_info.ch_start + (str[i]-f_info.first))->height);
	}
	cur_x = x;
	height += y;
	num = i;//\0を除く数
	for(i=0;i<num;i++){
		if(str[i]==' '){
			ch = f_info.ch_start['i'-f_info.first];
			cur_x += ch.width + 2;
			continue;
		}
		ch = f_info.ch_start[str[i]-f_info.first];
		drawFont(cur_x,height-ch.height,str[i],col);
		cur_x += ch.width + 2;
	}
	return 1;
}



