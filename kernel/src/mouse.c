#include <stdio.h>
#include "interp.h"
#include "font.h"

static InterpBuffer mouse;
static MouseBuffer m_buffer;

void initMouse(){
	int cnt;
	for(cnt=0;cnt<RETRY_MAX;cnt++){
		if(checkIfKeyReady()){
			_io_out8(PORT_KEYCMD,0xd4);//SendToMouse
			break;
		}
	}
	for(cnt=0;cnt<RETRY_MAX;cnt++){
		if(checkIfKeyReady()){
			_io_out8(PORT_KEYDAT,0xf4);//EnableMouseInterrupt
			break;
		}
	}
}

void interpMouse(){
	UCHAR data = _io_in8(KBD_ENC);
	addBuffer(data,&mouse);
	_io_out8(PIC0_COMMUND_STATUS,0x62);//���荞�ݎ�t����
	_io_out8(PIC1_COMMUND_STATUS,0x64);
}

void pushMBuffer(UCHAR data){
	m_buffer.data[m_buffer.seek] = data;
	m_buffer.seek++;
}

void decodeMouseData(MouseStatus *ms){
	UCHAR data;
	int dx,dy;
	//�f�[�^����
	data = popBuffer(&mouse);
	//�}�E�X�̐ݒ����������ɗ���0xfa��ǂݔ�΂�
	if(m_buffer.status==OFF){
		if(data==0xfa)m_buffer.status=ON;
		return;
	}
	//�f�[�^�̉��
	else{
		switch(m_buffer.seek){
			case 0:
				if ((data & 0xc8) == 0x08){//�������f�[�^�ł���
					pushMBuffer(data);
				}
				break;
			case 1:
				pushMBuffer(data);
				break;
			case 2:
				pushMBuffer(data);
				dx = (int)m_buffer.data[1];
				dy = (int)m_buffer.data[2];
				if((m_buffer.data[0] & 0x10) != 0)dx|=0xffffff00;//���Ɉړ�
				if((m_buffer.data[0] & 0x20) != 0)dy|=0xffffff00;//���Ɉړ�
				dy = -dy;
				ms->dx+=dx;
				ms->dy+=dy;
				m_buffer.seek=0;
				break;
		}
	}
}

MouseStatus checkMouse(){
	MouseStatus tmp = {0,0,0,0};
	while(mouse.if_update!=0)decodeMouseData(&tmp);
	return tmp;
}
