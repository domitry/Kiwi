tmp	EQU	0x7bff
result	EQU	0x9ffff
handred EQU	0x7bfa

[BITS 16]
MOV	BYTE[handred],100
loop:
MOV	[tmp],AX
FLD	DWORD[tmp]
FDIV	DWORD[handred]
FSIN
FMUL	DWORD[handred]
FIST	DWORD[EAX]
ADD AX,4
CMP AX,780
JNE loop
HLT
