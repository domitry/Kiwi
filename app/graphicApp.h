
typedef unsigned int UINT;
typedef unsigned char UCHAR;

typedef struct Color{
	unsigned char r,g,b;
}Color;

typedef struct{
	UINT x,y;
	UINT width,height;
}Rect;

void drawRect(int x,int y,int width,int height,Color color);
void fill(Color color);
void tellBuffer(UCHAR *buffern,int width,int height);
void mmap(void *add,unsigned int size);
void kmemset(void *str,unsigned char c,int size);
void initWindow(int width,int height);

