#include "interp.h"

static Time cur_time;

/*original code : OSDev Wiki*/

UCHAR getRTCRegister(int reg){
	_io_out8(CMOS_ADD,reg);
	return _io_in8(CMOS_DATA);
}

Time getCurrentTime(){
	int regB;
	cur_time.second = getRTCRegister(0x00);
	cur_time.minute = getRTCRegister(0x02);
	cur_time.hour = getRTCRegister(0x04);
	cur_time.day = getRTCRegister(0x07);
	cur_time.month = getRTCRegister(0x08);
	cur_time.year = getRTCRegister(0x09);
	
	regB = getRTCRegister(0x0b);
	
	//16進数で記されている場合(Ex.12:30→hour:0x12,minute:0x30)
	if (!(regB & 0x04)) {
		cur_time.second = (cur_time.second & 0x0F) + ((cur_time.second / 16) * 10);
		cur_time.minute = (cur_time.minute & 0x0F) + ((cur_time.minute / 16) * 10);
		cur_time.hour = ( (cur_time.hour & 0x0F) + (((cur_time.hour & 0x70) / 16) * 10) ) | (cur_time.hour & 0x80);
		cur_time.day = (cur_time.day & 0x0F) + ((cur_time.day / 16) * 10);
		cur_time.month = (cur_time.month & 0x0F) + ((cur_time.month / 16) * 10);
		cur_time.year = (cur_time.year & 0x0F) + ((cur_time.year / 16) * 10);
	}
	
	//年をちゃんと計算する(このコードが22世紀まで使われることは想定していません＞＜)
	cur_time.year += (CENTURY-1)*100;
	
	return cur_time;
}
