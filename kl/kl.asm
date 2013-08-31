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
MME_MEMO	EQU	0x7afc	; メモリマップ情報を格納したアドレスのメモ
MEM_SIZE_MEMO	EQU	0x7af8	; メモリサイズのメモ

; VESA対応の確認
	MOV	AX,0
	MOV	ES,AX
	MOV	DI,0x7b00	; 7b00-7bffに画面情報をメモ

	MOV	AX,0x4f00
	INT	0x10
	
	CMP	AL,0x4f
	JNE	set320

; 画面モードの確認・設定
	MOV	AX,0x4f01
	MOV	CX,0x118
	INT	0x10
	CMP	AX,0x004f		; 0x4fの場合はVESAが対応
	JNE	set320
	
	MOV	AX,0x4f02	;VESAが対応していたとき
	ADD	CX,0xC000	;VRAMクリア省略のbitを立てる
	MOV	BX,CX		;1280x1024x8bit
	INT	0x10
	MOV	EAX,[ES:DI+0x28]
	MOV	[VRAM],EAX
	JMP	getMemoryInfo

set320:	
	MOV	AL,0x13		; VBE非対応の場合はVGAの320x200x8bit
	MOV	AH,0x00
	INT	0x10

getMemoryInfo:

;	MOV	AX,0x4f05
;	MOV	BH,0x01
;	MOV	BL,0x01
;	INT	0x10


; メモリマップ情報の取得
	XOR	AX,AX
	MOV	ES,AX
	MOV	DS,AX
	MOV	EAX,DWORD mme
	MOV	DI,AX
	CALL	BiosGetMemoryMap
	MOV	[MME_MEMO],DWORD mme

; メモリサイズの取得
	XOR	EAX,EAX
	XOR	EBX,EBX
	MOV	AX,0xe801
	INT	0x15
	MOV	[MEM_SIZE_MEMO],EBX


; 割り込み禁止
disableInterrupt:
	MOV	AL,0xff
	OUT	0x21,AL		; PICマスター
	NOP
	OUT	0xa1,AL		; PICスレーブ

	CLI			; CPU

; A20GATEの設定
	CALL	waitKeyboard
	MOV		AL,0xd1
	OUT		0x64,AL
	CALL	waitKeyboard
	MOV		AL,0xdf			; enable A20
	OUT		0x60,AL
	CALL	waitKeyboard

; GDTの設定
	LGDT	[GDTR0]	

; Legacy Pagingの無効化
	MOV	EAX,CR0
	AND	EAX,0x7fffffff

; Protected Modeへ移行
	OR	EAX,0x00000001	; Protected Modeへ移行
	MOV	CR0,EAX
	JMP	flushPipeline
flushPipeline:

; セグメントレジスタの指定
	MOV		AX,0x8	; データセグメントを指定
	MOV		DS,AX
	MOV		ES,AX
	MOV		FS,AX
	MOV		GS,AX
	MOV		SS,AX

	JMP	0x10:flush_seg	; コードセグメントを指定
flush_seg:

[BITS 32]

; カーネルのロード(Pagingが無効な内にやっておく)
	MOV	EAX,ROOT_DIR

;KERNELの文字を探す
search:
	ADD	EAX,0x20		; 1エントリ分進める

	CMP	EAX,ROOT_DIR_END	; RootDirectoryの末尾まで来た
	JE	error

	XOR	EDX,EDX
	MOV	EDX,k_str
	MOV	EBX,EAX

findChar:
	MOV	CL,BYTE[EDX]

	CMP	BYTE[EBX],CL		; 1文字比較
	JNE	search			; 一致しなければ次のエントリへ

	ADD	EBX,1			
	ADD	EDX,1
	CMP	EDX,k_str_end
	JNE	findChar

;KERNELを見つけた！
	MOV	BX,WORD[EAX+26]	; 先頭クラスタ番号取得
	MOV	ECX,DWORD[EAX+28]	; サイズ取得

;先頭アドレス・サイズの計算
	SUB	EBX,2		; クラスタ番号は2から始まる
	MOV	EAX,EBX
	MOV	EBX,0x200
	MUL	EBX		; クラスタ番号*512byteがデータ領域からのオフセット
	ADD	EAX,DATA_SEG	; メモリ上の実際の位置を計算
	MOV	EBX,EAX


	MOV	[PDPT],ECX	; メモ用にPDPTを使わせていただく
	MOV	EAX,[PDPT]	; EAXにサイズを代入
	XOR	EDX,EDX
	MOV	ECX,4
	DIV	ECX		; 商がEAXに、余りがEDXに
	CMP	EDX,0
	JE	finish_calc
	ADD	EAX,1		; 余りが出たら余分に4byteコピーする

; データの転送
finish_calc:
	MOV	ECX,EAX		; サイズ計算の結果をECXに
	MOV	ESI,EBX		; コピー開始位置はESIへ
	MOV	EDI,KER_BEGIN_PHYS	; コピー先
	CALL	memcpy

; PDPT,PDEの初期化
	MOV	EDI,PDE		; [PDE]を開始地点に([ES:EDI]が開始地点)
	XOR	EAX,EAX
	MOV	ECX,4104
	REP	STOSD		; EAXに格納されている0でPDEから4 x 4104byte埋める
				; REPプリフィックスを付けることでECXに指定されるカウント分繰り返す

; PDPTの設定
	MOV	EBX,PDE		; 書き込む物理アドレスのスタート
	MOV	ECX,PDPT	; 書き込み先のアドレス
setPDPT:
	MOV	EAX,EBX
	AND	EAX,0xfffff000	; 下位12bitをマスク(不要だが一応)
	OR	EAX,0b01001	; 各種フラグを立てる
	MOV	[ECX],EAX
	ADD	ECX,4		; 4byte進めて0埋め
	MOV	DWORD[ECX],0
	ADD	ECX,4		; さらに4byte進める
	ADD	EBX,0x1000	; 書き込む物理アドレスは0x1000(4kb)進める
	CMP	EBX,0x104000	; PDPTは4エントリ
	JNE	setPDPT

; PDEの設定
	MOV	EBX,0x00000	; 書き込むアドレス
	MOV	ECX,PDE		; 書き込み先のアドレス
	ADD	ECX,0x3000	; 12kb分進めてVIR:0xc000000の分から設定する
setPDE:
	MOV	EAX,EBX
	AND	EAX,0xfff00000	; 下位20bitをマスク
	OR	EAX,0b110001011	; 各種フラグ設定
	MOV	DWORD[ECX],EAX
	ADD	ECX,4		; 4byte進める
	MOV	DWORD[ECX],0
	ADD	ECX,4		; さらに4byte進める
	ADD	EBX,0x0200000	; 書き込む物理アドレスは2MB分進める
	CMP	EBX,0x1400000	; 20MB分設定
	JNE	setPDE

; 自分の場所が元と違っていたら落ちるので一時的に0x0000から2MB分設定する
setSelfPDE:
	MOV	DWORD[PDE],0b110001011
	MOV	DWORD[PDE+4],0
; スタックの設定


; CR3に値を設定
	MOV	EAX,PDPT
	MOV	CR3,EAX

; PAE(Pysical Address Extention)有効化
	MOV	EAX,CR4
	OR	EAX,0x00000020	; CR4のbit5がPAEを有効化するビット
	MOV	CR4,EAX

; Pagingの有効化
	MOV	EAX,CR0
	OR	EAX,0x80000000
	MOV	CR0,EAX
	JMP 	flushPipeline2
flushPipeline2:
	ADD	ESP,0xc0000000 ; スタックの位置を変更

; カーネルへ処理を渡す
	JMP	KER_BEGIN_VIR

error :
	HLT
	JMP	error

waitKeyboard:
	IN	AL,0x64		; 0x64のINはステータスコード読み込み
	AND	AL,0x02		; bit6の受信タイムアウトフラグが1のときは受信未完了
	JNZ	waitKeyboard
	RET

;hariboteのコードをそのまま
;ESIにコピー元アドレス,EDIにコピー先アドレス,ECXにコピーバイト数/4した数
memcpy:
	MOV	EAX,[ESI]
	ADD	ESI,4
	MOV	[EDI],EAX
	ADD	EDI,4
	SUB	ECX,1
	JNZ	memcpy			; 引き算した結果が0でなければmemcpyへ
	RET

[BITS 16]

; メモリマップの取得(0から作るOS開発さんのコードそのままです…)
BiosGetMemoryMap:
	PUSHAD
	XOR	EBX, EBX
	XOR	BP, BP		; メモリマップ情報のエントリ数をBPに入れる
	MOV	EDX, 'PAMS'	; ”SMAP”
	MOV	EAX, 0xE820
	MOV	ECX, 24		; MemoryMapEntryのバイトサイズ
	INT	0x15		; メモリマップ情報を取得
	JC	.ERROR		; エラー時は.ERRORへ
	CMP	EAX, 'PAMS'	; BIOSが”SMAP”をしたか調べる
	JNE	.ERROR		; ”SMAP”ではないときは.ERRORへ
	TEST	EBX, EBX	; 戻り値EBXが0のときはメモリ情報は無し
	JE	.ERROR
	JMP	.START		; メモリマップ情報を格納する
.NEXT_ENTRY:
	MOV	EDX, 'PAMS'	; EDSにもう一度”SMAP”入れておく
	MOV	ECX, 24		; MemoryMapEntryのバイトサイズ
	MOV	EAX, 0xE820
	INT	0x15		; 次のメモリマップ情報を取得
.START:
	JCXZ	.SKIP_ENTRY	; CFが0のときは失敗なので戻り値を格納しない
.NOTNEXT:
	MOV	ECX, [ES:DI + MemoryMapEntry.length]	; BIOSが格納したメモリマップ情報のLengthを取得（下位4バイト分）
	TEST	ECX, ECX		; Length（下位4バイト）が0かどうか調べる
	JNE	SHORT .GOOD_ENTRY	; 0じゃなければ.GOOD_ENTRYへ
	MOV	ECX, [ES:DI + MemoryMapEntry.length + 4]; BIOSが格納したメモリマップ情報のLengthを取得（上位4バイト分）
	JECXZ	.SKIP_ENTRY	; Length（上位4バイト）が0なら.SKIP_ENTRYへ
.GOOD_ENTRY:
	INC	BP		; メモリマップ情報のエントリ数をインクリメント
	ADD	DI, 24		; 格納先ポインタを進める（MemoryMapEntryは24バイト）
.SKIP_ENTRY:
	CMP	EBX, 0		; EBXが0ならこれ以上メモリマップ情報は無し
	JNE	.NEXT_ENTRY	; 次候補がある場合は.NEXT_ENTRYへ
	JMP	.MM_DONE	; 終了
.ERROR:
	STC			; CFフラグをセット
	
.MM_DONE:
	POPAD
	RET

[BITS 32]
; セグメンテーションの設定
	ALIGNB	16		; アドレスが16の倍数になるまで0埋め
GDT0:
	RESB	8		; ヌルセレクタ
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
