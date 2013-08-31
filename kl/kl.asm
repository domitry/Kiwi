[BITS 16]

ORG	0xc000

PDPT		EQU	0x104000
PDE		EQU	0x100000
KER_BEGIN_PHYS	EQU	0x104020
KER_BEGIN_VIR	EQU	0xc0104020
ROOT_DIR	EQU	0xa400
ROOT_DIR_END	EQU	0xc000
DATA_SEG	EQU	0xc000
VRAM		EQU	0x0ff8
MME_MEMO	EQU	0x7afc	; �������}�b�v�����i�[�����A�h���X�̃���
MEM_SIZE_MEMO	EQU	0x7af8	; �������T�C�Y�̃���

; VESA�Ή��̊m�F
	MOV	AX,0
	MOV	ES,AX
	MOV	DI,0x7b00	; 7b00-7bff�ɉ�ʏ�������

	MOV	AX,0x4f00
	INT	0x10
	
	CMP	AL,0x4f
	JNE	set320

; ��ʃ��[�h�̊m�F�E�ݒ�
	MOV	AX,0x4f01
	MOV	CX,0x118
	INT	0x10
	CMP	AX,0x004f		; 0x4f�̏ꍇ��VESA���Ή�
	JNE	set320
	
	MOV	AX,0x4f02	;VESA���Ή����Ă����Ƃ�
	ADD	CX,0xC000	;VRAM�N���A�ȗ���bit�𗧂Ă�
	MOV	BX,CX		;1280x1024x8bit
	INT	0x10
	MOV	EAX,[ES:DI+0x28]
	MOV	[VRAM],EAX
	JMP	getMemoryInfo

set320:	
	MOV	AL,0x13		; VBE��Ή��̏ꍇ��VGA��320x200x8bit
	MOV	AH,0x00
	INT	0x10

getMemoryInfo:

;	MOV	AX,0x4f05
;	MOV	BH,0x01
;	MOV	BL,0x01
;	INT	0x10


; �������}�b�v���̎擾
	XOR	AX,AX
	MOV	ES,AX
	MOV	DS,AX
	MOV	EAX,DWORD mme
	MOV	DI,AX
	CALL	BiosGetMemoryMap
	MOV	[MME_MEMO],DWORD mme

; �������T�C�Y�̎擾
	XOR	EAX,EAX
	XOR	EBX,EBX
	MOV	AX,0xe801
	INT	0x15
	MOV	[MEM_SIZE_MEMO],EBX


; ���荞�݋֎~
disableInterrupt:
	MOV	AL,0xff
	OUT	0x21,AL		; PIC�}�X�^�[
	NOP
	OUT	0xa1,AL		; PIC�X���[�u

	CLI			; CPU

; A20GATE�̐ݒ�
	CALL	waitKeyboard
	MOV		AL,0xd1
	OUT		0x64,AL
	CALL	waitKeyboard
	MOV		AL,0xdf			; enable A20
	OUT		0x60,AL
	CALL	waitKeyboard

; GDT�̐ݒ�
	LGDT	[GDTR0]	

; Legacy Paging�̖�����
	MOV	EAX,CR0
	AND	EAX,0x7fffffff

; Protected Mode�ֈڍs
	OR	EAX,0x00000001	; Protected Mode�ֈڍs
	MOV	CR0,EAX
	JMP	flushPipeline
flushPipeline:

; �Z�O�����g���W�X�^�̎w��
	MOV		AX,0x8	; �f�[�^�Z�O�����g���w��
	MOV		DS,AX
	MOV		ES,AX
	MOV		FS,AX
	MOV		GS,AX
	MOV		SS,AX

	JMP	0x10:flush_seg	; �R�[�h�Z�O�����g���w��
flush_seg:

[BITS 32]

; �J�[�l���̃��[�h(Paging�������ȓ��ɂ���Ă���)
	MOV	EAX,ROOT_DIR

;KERNEL�̕�����T��
search:
	ADD	EAX,0x20		; 1�G���g�����i�߂�

	CMP	EAX,ROOT_DIR_END	; RootDirectory�̖����܂ŗ���
	JE	error

	XOR	EDX,EDX
	MOV	EDX,k_str
	MOV	EBX,EAX

findChar:
	MOV	CL,BYTE[EDX]

	CMP	BYTE[EBX],CL		; 1������r
	JNE	search			; ��v���Ȃ���Ύ��̃G���g����

	ADD	EBX,1			
	ADD	EDX,1
	CMP	EDX,k_str_end
	JNE	findChar

;KERNEL���������I
	MOV	BX,WORD[EAX+26]	; �擪�N���X�^�ԍ��擾
	MOV	ECX,DWORD[EAX+28]	; �T�C�Y�擾

;�擪�A�h���X�E�T�C�Y�̌v�Z
	SUB	EBX,2		; �N���X�^�ԍ���2����n�܂�
	MOV	EAX,EBX
	MOV	EBX,0x200
	MUL	EBX		; �N���X�^�ԍ�*512byte���f�[�^�̈悩��̃I�t�Z�b�g
	ADD	EAX,DATA_SEG	; ��������̎��ۂ̈ʒu���v�Z
	MOV	EBX,EAX


	MOV	[PDPT],ECX	; �����p��PDPT���g�킹�Ă�������
	MOV	EAX,[PDPT]	; EAX�ɃT�C�Y����
	XOR	EDX,EDX
	MOV	ECX,4
	DIV	ECX		; ����EAX�ɁA�]�肪EDX��
	CMP	EDX,0
	JE	finish_calc
	ADD	EAX,1		; �]�肪�o����]����4byte�R�s�[����

; �f�[�^�̓]��
finish_calc:
	MOV	ECX,EAX		; �T�C�Y�v�Z�̌��ʂ�ECX��
	MOV	ESI,EBX		; �R�s�[�J�n�ʒu��ESI��
	MOV	EDI,KER_BEGIN_PHYS	; �R�s�[��
	CALL	memcpy

; PDPT,PDE�̏�����
	MOV	EDI,PDE		; [PDE]���J�n�n�_��([ES:EDI]���J�n�n�_)
	XOR	EAX,EAX
	MOV	ECX,4104
	REP	STOSD		; EAX�Ɋi�[����Ă���0��PDE����4 x 4104byte���߂�
				; REP�v���t�B�b�N�X��t���邱�Ƃ�ECX�Ɏw�肳���J�E���g���J��Ԃ�

; PDPT�̐ݒ�
	MOV	EBX,PDE		; �������ޕ����A�h���X�̃X�^�[�g
	MOV	ECX,PDPT	; �������ݐ�̃A�h���X
setPDPT:
	MOV	EAX,EBX
	AND	EAX,0xfffff000	; ����12bit���}�X�N(�s�v�����ꉞ)
	OR	EAX,0b01001	; �e��t���O�𗧂Ă�
	MOV	[ECX],EAX
	ADD	ECX,4		; 4byte�i�߂�0����
	MOV	DWORD[ECX],0
	ADD	ECX,4		; �����4byte�i�߂�
	ADD	EBX,0x1000	; �������ޕ����A�h���X��0x1000(4kb)�i�߂�
	CMP	EBX,0x104000	; PDPT��4�G���g��
	JNE	setPDPT

; PDE�̐ݒ�
	MOV	EBX,0x00000	; �������ރA�h���X
	MOV	ECX,PDE		; �������ݐ�̃A�h���X
	ADD	ECX,0x3000	; 12kb���i�߂�VIR:0xc000000�̕�����ݒ肷��
setPDE:
	MOV	EAX,EBX
	AND	EAX,0xfff00000	; ����20bit���}�X�N
	OR	EAX,0b110001011	; �e��t���O�ݒ�
	MOV	DWORD[ECX],EAX
	ADD	ECX,4		; 4byte�i�߂�
	MOV	DWORD[ECX],0
	ADD	ECX,4		; �����4byte�i�߂�
	ADD	EBX,0x0200000	; �������ޕ����A�h���X��2MB���i�߂�
	CMP	EBX,0x1400000	; 20MB���ݒ�
	JNE	setPDE

; �����̏ꏊ�����ƈ���Ă����痎����̂ňꎞ�I��0x0000����2MB���ݒ肷��
setSelfPDE:
	MOV	DWORD[PDE],0b110001011
	MOV	DWORD[PDE+4],0
; �X�^�b�N�̐ݒ�


; CR3�ɒl��ݒ�
	MOV	EAX,PDPT
	MOV	CR3,EAX

; PAE(Pysical Address Extention)�L����
	MOV	EAX,CR4
	OR	EAX,0x00000020	; CR4��bit5��PAE��L��������r�b�g
	MOV	CR4,EAX

; Paging�̗L����
	MOV	EAX,CR0
	OR	EAX,0x80000000
	MOV	CR0,EAX
	JMP 	flushPipeline2
flushPipeline2:
	ADD	ESP,0xc0000000 ; �X�^�b�N�̈ʒu��ύX

; �J�[�l���֏�����n��
	JMP	KER_BEGIN_VIR

error :
	HLT
	JMP	error

waitKeyboard:
	IN	AL,0x64		; 0x64��IN�̓X�e�[�^�X�R�[�h�ǂݍ���
	AND	AL,0x02		; bit6�̎�M�^�C���A�E�g�t���O��1�̂Ƃ��͎�M������
	JNZ	waitKeyboard
	RET

;haribote�̃R�[�h�����̂܂�
;ESI�ɃR�s�[���A�h���X,EDI�ɃR�s�[��A�h���X,ECX�ɃR�s�[�o�C�g��/4������
memcpy:
	MOV	EAX,[ESI]
	ADD	ESI,4
	MOV	[EDI],EAX
	ADD	EDI,4
	SUB	ECX,1
	JNZ	memcpy			; �����Z�������ʂ�0�łȂ����memcpy��
	RET

[BITS 16]

; �������}�b�v�̎擾(0������OS�J������̃R�[�h���̂܂܂ł��c)
BiosGetMemoryMap:
	PUSHAD
	XOR	EBX, EBX
	XOR	BP, BP		; �������}�b�v���̃G���g������BP�ɓ����
	MOV	EDX, 'PAMS'	; �hSMAP�h
	MOV	EAX, 0xE820
	MOV	ECX, 24		; MemoryMapEntry�̃o�C�g�T�C�Y
	INT	0x15		; �������}�b�v�����擾
	JC	.ERROR		; �G���[����.ERROR��
	CMP	EAX, 'PAMS'	; BIOS���hSMAP�h�����������ׂ�
	JNE	.ERROR		; �hSMAP�h�ł͂Ȃ��Ƃ���.ERROR��
	TEST	EBX, EBX	; �߂�lEBX��0�̂Ƃ��̓��������͖���
	JE	.ERROR
	JMP	.START		; �������}�b�v�����i�[����
.NEXT_ENTRY:
	MOV	EDX, 'PAMS'	; EDS�ɂ�����x�hSMAP�h����Ă���
	MOV	ECX, 24		; MemoryMapEntry�̃o�C�g�T�C�Y
	MOV	EAX, 0xE820
	INT	0x15		; ���̃������}�b�v�����擾
.START:
	JCXZ	.SKIP_ENTRY	; CF��0�̂Ƃ��͎��s�Ȃ̂Ŗ߂�l���i�[���Ȃ�
.NOTNEXT:
	MOV	ECX, [ES:DI + MemoryMapEntry.length]	; BIOS���i�[�����������}�b�v����Length���擾�i����4�o�C�g���j
	TEST	ECX, ECX		; Length�i����4�o�C�g�j��0���ǂ������ׂ�
	JNE	SHORT .GOOD_ENTRY	; 0����Ȃ����.GOOD_ENTRY��
	MOV	ECX, [ES:DI + MemoryMapEntry.length + 4]; BIOS���i�[�����������}�b�v����Length���擾�i���4�o�C�g���j
	JECXZ	.SKIP_ENTRY	; Length�i���4�o�C�g�j��0�Ȃ�.SKIP_ENTRY��
.GOOD_ENTRY:
	INC	BP		; �������}�b�v���̃G���g�������C���N�������g
	ADD	DI, 24		; �i�[��|�C���^��i�߂�iMemoryMapEntry��24�o�C�g�j
.SKIP_ENTRY:
	CMP	EBX, 0		; EBX��0�Ȃ炱��ȏチ�����}�b�v���͖���
	JNE	.NEXT_ENTRY	; ����₪����ꍇ��.NEXT_ENTRY��
	JMP	.MM_DONE	; �I��
.ERROR:
	STC			; CF�t���O���Z�b�g
	
.MM_DONE:
	POPAD
	RET

[BITS 32]
; �Z�O�����e�[�V�����̐ݒ�
	ALIGNB	16		; �A�h���X��16�̔{���ɂȂ�܂�0����
GDT0:
	RESB	8		; �k���Z���N�^
	; Data
	DW 0xFFFF		; Limit
	DW 0x0000		; Base Address Low
	DB 0x00			; Base Address Mid
	DB 10010010b		; P,DPL(2),S,Type(4)
	DB 11001111b		; G,D/B,0,AVL,Limit
	DB 0			; Base Address Hi
	; Code				
	DW 0xFFFF		; Limit
	DW 0x0000		; Base Address Low
	DB 0x00			; Base Address Mid
	DB 10011010b		; P,DPL(2),S,Type(4)
	DB 11001111b		; G,D/B,0,AVL,Limit
	DB 0			; Base Address Hi

	DW		0
GDTR0:
	DW		8*3
	DD		GDT0
	ALIGNB	16

k_str:
	DB	"KERNEL"
k_str_end:

struc	MemoryMapEntry
	.baseAddress	resq	1
	.length		resq	1
	.type		resd	1
	.acpi_null	resd	1
endstruc

mme:
istruc MemoryMapEntry
	at MemoryMapEntry.baseAddress,	dd 0
	at MemoryMapEntry.length,	dd 0
	at MemoryMapEntry.type,		dd 0
	at MemoryMapEntry.acpi_null,	dd 0
iend
