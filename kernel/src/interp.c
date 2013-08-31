#include "interp.h"
#include "graphic.h"
#include "phys_mem.h"
#include "font.h"
#include "wsys.h"

static GateDescripter interp_desc[GATE_SIZE];


void addBuffer(UCHAR data,InterpBuffer *buffer){
	buffer->data[buffer->last]=data;
	if(buffer->last== BUFFER_NUM-1)buffer->last = 0;
	else buffer->last++;
	buffer->if_update = 1;
}

UCHAR popBuffer(InterpBuffer *buffer){
	UCHAR result;
	result = buffer->data[buffer->first];
	if(buffer->first==BUFFER_NUM-1)buffer->first=0;
	else buffer->first++;
	if(buffer->first==buffer->last)buffer->if_update = 0;
	return result;
}

void interpPageFault(){
	Color blue = {0,0,255};
	fill(blue);
	for(;;){asm("hlt");}
}

void interpSoftware(int tmp,int edi,int esi,int ebp,int esp,int ebx,int edx,int ecx,int eax){
	switch(eax){
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3://newWindow�V�X�e���R�[��(task.c)
			//asm volatile("mov %0,%%eax"::"r"(edx));
			//for(;;)asm("hlt");
			newWindow(ebx,ecx,edx);
			break;
		case 4://mmap
			ummap((void *)ebx,ecx);
			break;
	}
}

void setInterpHandler(int num,void *add){
	interp_desc[num].selector = 0x10;//�Z�O�����g�̎w��
	interp_desc[num].flags = 0x8e;//���荞�݃Q�[�g,32bit,�������x��ring0,�Z�O�����g������
	interp_desc[num].empty = 0;
	interp_desc[num].base_low = (USHORT)((UINT)(add) & 0x0000FFFF);
	interp_desc[num].base_high = (USHORT)(((UINT)(add) & 0xFFFF0000)>>16);
}

void setTrapHandler(int num,void *add){
	interp_desc[num].selector = 0x10;//�Z�O�����g�̎w��
	interp_desc[num].flags = 0x8f;//���荞�݃Q�[�g,32bit,�������x��ring0,�Z�O�����g������
	interp_desc[num].empty = 0;
	interp_desc[num].base_low = (USHORT)((UINT)(add) & 0x0000FFFF);
	interp_desc[num].base_high = (USHORT)(((UINT)(add) & 0xFFFF0000)>>16);
}

static void initIDT(){
	kmemset(interp_desc,0,GATE_SIZE*8);
	_load_idtr(GATE_SIZE*8, ((UINT)interp_desc - 0xc0000000));
}

void initPIC(){
	_io_out8(PIC0_IMR,0xff);//�S���荞�݃}�X�N(�}�X�^)
	_io_out8(PIC1_IMR,0xff);//�S���荞�݃}�X�N(�X���[�u)
	
	_io_out8(PIC0_ICW1,0x11);//ICW1(�}�X�^)
	_io_out8(PIC0_ICW2,0x20);//ICW2(PIC0�͊��荞�݃x�N�^32-39)
	_io_out8(PIC0_ICW3,0x04);//ICW3(PIC1�̈ʒu)
	_io_out8(PIC0_ICW4,0x01);//ICW4
	
	_io_out8(PIC1_ICW1,0x11);//ICW1(�X���[�u)
	_io_out8(PIC1_ICW2,0x28);//ICW2(PIC1�͊��荞�݃x�N�^40-48)
	_io_out8(PIC1_ICW3,0x02);//ICW3(PIC0�̈ʒu)
	_io_out8(PIC1_ICW4,0x01);//ICW4
	
	_io_out8(PIC0_IMR,0xfb);//PIC1�̂݊��荞�ݎ�t
	_io_out8(PIC1_IMR,0xff);//�S���荞�݃}�X�N
}

void initInterp(InterpBuffer **k_key,InterpBuffer **k_mouse){
	initIDT();
	setTrapHandler(0x08,interpPageFault);//���}���u�I�ȃA���ł�
	setTrapHandler(0x0B,interpPageFault);
	setTrapHandler(0x0C,interpPageFault);
	setTrapHandler(0x0D,interpPageFault);
	setTrapHandler(0x0E,interpPageFault);
	setTrapHandler(0x11,interpPageFault);

	setInterpHandler(0x21,_keyIntHandler);
	setInterpHandler(0x2c,_mouseIntHandler);
	setInterpHandler(0x20,_timerIntHandler);
	setInterpHandler(0x30,_softwareIntHandler);
	
	initPIC();
	asm("sti");
	
	initKeyboard();
	initMouse();
	initTimer();
	
	_io_out8(PIC0_IMR,0xf8);//PIC0�ł�PIC1����̊��荞�݂̂ݎ�t
	_io_out8(PIC1_IMR,0xef);//PIC1�͑S���荞�݃}�X�N
}
