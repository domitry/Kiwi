MAKE = make -r
GCC = /usr/bin/i686-pc-linux-gnu-gcc.exe
LD = /usr/bin/i686-pc-linux-gnu-ld.exe
NASM = nasm
OP_GCC = -O2 -I./include -Wall
COPY = cp

#--save-temps

OBJS = krnl.o phys_mem.o section.o graphic.o fs.o font.o interp.o nasmfunc.o wsys.o task.o mouse.o keyboard.o pit.o rtc.o

%.o:./src/%.c
	$(GCC) -c -o $@ $< $(OP_GCC) 

nasmfunc.o:src/asmfunc.asm
	$(NASM) -f elf -o nasmfunc.o src/asmfunc.asm

run: $(OBJS)
	$(LD)  -T link -e kmain --oformat binary -nostdlib -o kernel.bin $(OBJS) -Map kernel.map -static -L. -lgolibc --cref
	$(COPY) kernel.bin ../

default:
	run 