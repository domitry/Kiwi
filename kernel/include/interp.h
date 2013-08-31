#ifndef INTERP_H
#define INTERP_H

#define GATE_SIZE 256
#define BUFFER_NUM 256
#define RETRY_MAX 9999999

#define PIC0_COMMUND_STATUS 0x20
#define PIC1_COMMUND_STATUS 0xa0
#define PIC0_MASK_DATA 0x21
#define PIC1_MASK_DATA 0xa1

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064

#define KBD_STATUS 0x64
#define KBD_CMD 0x64
#define KBD_DATA 0x60

#define KBD_ENC 0x60

#define INTERVAL_GRAPH 5
#define INTERVAL_SYSCALL 20

#define ON 1
#define OFF 0

#define CENTURY 21

#define CMOS_ADD		0x70
#define CMOS_DATA		0x71

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;


typedef struct {
	USHORT base_low;
	USHORT selector;
	UCHAR empty;
	UCHAR flags;//p,dpl(2),0,1,1,1,0
	USHORT base_high;
}__attribute__((packed)) GateDescripter;

typedef struct {
	USHORT size;
	GateDescripter *add;
}__attribute__((packed)) IDTR;

typedef struct {
	UCHAR data[BUFFER_NUM];
	UCHAR first,last;
	UCHAR if_update;
}InterpBuffer;

typedef struct{
	UCHAR data[3];
	int seek;
	UCHAR status;
}MouseBuffer;

typedef struct{
	unsigned int cnt;
}Timer;

typedef struct{
	UCHAR second;
	UCHAR minute;
	UCHAR hour;
	UCHAR day;
	UCHAR month;
	UCHAR year;
}Time;

typedef struct{
	int dx;
	int dy;
	UCHAR button;
	UCHAR scroll;
	UCHAR tmp[2];
}MouseStatus;

typedef struct{
	char buffer[32];
}KeyboardStatus;

void initInterp();
void addBuffer(UCHAR data,InterpBuffer *buffer);
UCHAR popBuffer(InterpBuffer *buffer);

//asmfunc.asm
void _mouseIntHandler();
void _keyIntHandler();
void _timerIntHandler();
void _softwareIntHandler();
int _io_in8(int port);
void _io_out8(int port, int data);
void _load_idtr(int limit, int addr);

/*後からデバイスを追加することを想定して分離したもの*/

//mouse.c
void initMouse();
void interpMouse();
MouseStatus checkMouse();

//keyboard.c
_Bool checkIfKeyReady();
void initKeyboard();
void interpKeyboard();

//pit.c
void initTimer();
void interpTimer();

//rtc.c
Time getCurrentTime();

#endif
