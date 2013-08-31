#include "interp.h"

static InterpBuffer key;

_Bool checkIfKeyReady(){
	if((_io_in8(KBD_STATUS)&0x02) == 0)return 1;
	return 0;
}

void initKeyboard(){
	key.first=0;//バッファの初期化
	key.last=0;
	
	for(;;){
		if(checkIfKeyReady()){
			_io_out8(PORT_KEYCMD,0x60);//SendControlCommand
			break;
		}
	}
	for(;;){
		if(checkIfKeyReady()){
			_io_out8(PORT_KEYDAT,0x47);//KeyInterpOn,MouseInterpOn,WormReboot,KeyEnable,MouseEnable,UnuseScanCode
			break;
		}
	}
}

void interpKeyboard(){
	UCHAR data = _io_in8(KBD_ENC);
	addBuffer(data,&key);
	_io_out8(PIC0_COMMUND_STATUS,0x61);//割り込み受付完了
}

void checkKeyboard(){
	
}

