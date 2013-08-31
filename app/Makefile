MAKE = make -r
GCC = /usr/bin/i686-pc-linux-gnu-gcc.exe
LD = /usr/bin/i686-pc-linux-gnu-ld.exe
NASM = nasm
OP_GCC = -O2 -I. -Wall
COPY = cp

OBJS = term.o graphicApp.o

%.o:./%.c
	$(GCC) -c -o $@ $< $(OP_GCC)


run: $(OBJS)
	$(LD)  -T link -e appMain --oformat binary -nostdlib -o term.app $(OBJS) -Map term.map -static --cref
	$(COPY) term.app ../

default:
	run 