#include "interp.h"
#include "wsys.h"

static Timer timer;

void initTimer(){
	_io_out8(0x43,0x34);//コマンド(バイナリ,パルス,LSBとMSB)
	_io_out8(0x40,0x9c);//カウンタ0
	_io_out8(0x40,0x2e);//2e9c間隔
}

void interpTimer(){
	timer.cnt++; 
	if((timer.cnt % INTERVAL_GRAPH)==0)proceed();
	if((timer.cnt % INTERVAL_SYSCALL)==0);
	_io_out8(PIC0_COMMUND_STATUS,0x60);//割り込み完了
}

void checkTimer(){
	
	
}

