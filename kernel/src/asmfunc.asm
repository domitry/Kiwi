[BITS 32]

GLOBAL	_keyIntHandler,_mouseIntHandler,_io_in8,_io_out8,_load_idtr,_timerIntHandler,_softwareIntHandler
EXTERN	interpKeyboard,interpMouse,interpTimer,interpSoftware

[SECTION .text]

_io_in8:	; int io_in8(int port);
	MOV	EDX,[ESP+4]	; port
	MOV	EAX,0
	IN	AL,DX
	RET

_io_out8:	; void io_out8(int port, int data);
	MOV	EDX,[ESP+4]	; port
	MOV	AL,[ESP+8]	; data
	OUT	DX,AL
	RET

_load_idtr:	; void load_idtr(int limit, int addr);
	MOV	AX,[ESP+4]	; limit
	MOV	[ESP+6],AX
	LIDT	[ESP+6]
	RET

_keyIntHandler:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX,ESP
	PUSH	EAX
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	CALL	interpKeyboard
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD
	
_mouseIntHandler:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX,ESP
	PUSH	EAX
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	CALL	interpMouse
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD
	
_timerIntHandler:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX,ESP
	PUSH	EAX
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	CALL	interpTimer
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_softwareIntHandler:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV	EAX,ESP
	PUSH	EAX
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	CALL	interpSoftware
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

