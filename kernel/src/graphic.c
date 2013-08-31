#include "graphic.h"
#include "phys_mem.h"
#include "fs.h"

static VramInfo v_info;
char str[10];

//asm volatile("cli");
//asm volatile("mov %0,%%eax"::"r"():"eax");
//for(;;)asm("hlt");

void initGraphic(){
	VbeInfo *vbe_info = (VbeInfo *)VBE_INFO;
	v_info.start = (unsigned char*)VRAM_START_VIR;//vramはこのアドレスにマップされる
	v_info.width = (int)vbe_info->width;
	v_info.height = (int)vbe_info->height;
	v_info.bit_per_pic = (char)vbe_info->bit_per_pic;
	v_info.depth = v_info.bit_per_pic/8;
	v_info.line = (char)vbe_info->bytes_per_line;
	kmmap_phys((void *)(vbe_info->vram_add),(void *)VRAM_START_VIR,v_info.width*v_info.height*v_info.depth);//vramを仮想アドレス空間にマップ
}

VramInfo getGraphInfo(){
	return v_info;
}

void fill(Color color){
	unsigned char *end = v_info.start+v_info.width*v_info.height*v_info.depth;//vramの終わり
	unsigned char *i;
	for(i=v_info.start;i<end;i+=v_info.depth){
		*(i+2) = color.r;
		*(i+1) = color.g;
		*(i) = color.b;
	}
}

void* loadImage(char *filename){
	int i;
	ImageHeader *img = (ImageHeader*)loadFile(filename);
	Pixel *r_start,*r_seek,*r_end;
	UCHAR *w_start,*w_seek,*w_end;
	
	r_start = (Pixel*)((UCHAR *)img + IH_SIZE);
	r_end = (Pixel*)((UCHAR *)img + IH_SIZE + img->size); 
	w_start = kmalloc(img->width*img->height*3);
	w_end = (UCHAR *)(w_start + img->width*img->height*3);
	w_seek = w_start;
	 
	for(r_seek=r_start;r_seek<r_end;r_seek++){
		for(i=0;i<r_seek->cnt;i++){
			*w_seek = r_seek->color[2];
			*(w_seek+1) = r_seek->color[1];
			*(w_seek+2) = r_seek->color[0];
			w_seek+=3;
		}
		if(w_seek>w_end)return w_start;
	}
	return w_start;
}

//256色イメージのロード
void* loadImage256(char *filename){
	int i;
	ImageHeader *img = (ImageHeader*)loadFile(filename);
	UCHAR *r_seek= (UCHAR*)((UCHAR *)img + IH_SIZE);
	UCHAR *r_end = (UCHAR*)((UCHAR *)img + IH_SIZE + img->size); 
	
	UCHAR *w_seek = kmalloc(img->width*img->height*3);
	UCHAR *w_start = w_seek;
	
	Color pal[256];
	
	for(i=0;i<256;i++){
		pal[i].r = ((Pallet *)r_seek)->b;
		pal[i].g = ((Pallet *)r_seek)->g;
		pal[i].b = ((Pallet *)r_seek)->r;
		r_seek+=4;
	}
	r_end = (UCHAR *)((int)r_seek + img->size);
	
	for(;r_seek<r_end;r_seek+=2){
		for(i=0;i<r_seek[0];i++){
			*w_seek = pal[r_seek[1]].r;
			*(w_seek+1) = pal[r_seek[1]].g;
			*(w_seek+2) = pal[r_seek[1]].b;
			w_seek+=3;
		}
	}
	return w_start;
}


void drawRect(int x,int y,int width,int height,Color color){
	int i;
	UCHAR *w_seek,*w_end;
	w_seek = v_info.start + (y*v_info.width + x)*v_info.depth;
	w_end = v_info.start + ((y+height)*v_info.width + x)*v_info.depth;
	while(w_seek < w_end){
		for(i=0;i<width;i++){
			*(w_seek+2) = color.r;
			*(w_seek+1) = color.g;
			*(w_seek) = color.b;
			w_seek+=v_info.depth;
		}
		w_seek += (v_info.width - width)*v_info.depth;
	}
}

void drawImage(UCHAR *image,int x,int y,int width,int height){
	UCHAR *w_seek,*r_seek,*end;
	r_seek = image;
	w_seek = v_info.start + (y*v_info.width + x)*v_info.depth;
	end = v_info.start + ((y+height)*v_info.width + x)*v_info.depth ;
	
	while(w_seek < end){
		kmemcpy(w_seek,r_seek,width * v_info.depth);
		w_seek += v_info.width *v_info.depth;
		r_seek += width * v_info.depth;
	}
}

void drawImage2(UCHAR *image,int x,int y,int width,int height,UCHAR mode){
	UCHAR *w_seek,*r_seek,*end;
	int i;
	float alpha;
	r_seek = image;
	w_seek = v_info.start + (y*v_info.width + x)*v_info.depth;
	end = v_info.start + ((y+height)*v_info.width + x)*v_info.depth ;
	
	while(w_seek < end){
		switch(mode){
			case IMG_RGBA:
				for(i=0;i<width;i++){
					alpha = (float)r_seek[3]/255;
					w_seek[i*3] = (1-alpha)*w_seek[i*3]+alpha*r_seek[0];
					w_seek[i*3+1] = (1-alpha)*w_seek[i*3+1]+alpha*r_seek[1];
					w_seek[i*3+2] = (1-alpha)*w_seek[i*3+2]+alpha*r_seek[2];
					r_seek += 4;
				}
				break;
			case IMG_RGB:
				kmemcpy(w_seek,r_seek,width * v_info.depth);
				r_seek += width * v_info.depth;
				break;
		}
		w_seek += v_info.width * v_info.depth;
	}
}

void drawImageTrans(UCHAR *image,int x,int y,int width,int height,Color trans){
	int i;
	UCHAR *w_seek,*r_seek,*end;
	r_seek = image;
	w_seek = v_info.start + (y*v_info.width + x)*v_info.depth;
	end = v_info.start + ((y+height)*v_info.width + x)*v_info.depth ;
	while(w_seek < end){
		for(i=0;i<width*v_info.depth;i+=3){
			if(r_seek[i]==trans.b&&r_seek[i+1]==trans.g&&r_seek[i+2]==trans.r)
				continue;
			else{
				w_seek[i] = r_seek[i];
				w_seek[i+1] = r_seek[i+1];
				w_seek[i+2] = r_seek[i+2];
			}
		}
		w_seek += v_info.width *v_info.depth;
		r_seek += width * v_info.depth;
	}
}
