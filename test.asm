[BITS 16]
VRAM	EQU 0xa0000
MOV	AX,0x13		;320x200
INT	0x10
changeColor:
INC	BL
MOV	BX,3200
MOV	CX,10
moveX:
MOV	[EAX],BL
INC	AX
CMP	AX,64000
JE	fin
MUL	BX
;CMP	[DX:AX],0
JE	moveX
MUL	CX
;CMP	[DX:AX],0
JE	changeColor
fin: