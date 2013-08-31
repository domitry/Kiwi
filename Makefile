MAKE = make -r
NASM = nasm
EDIMG = tools/edimg.exe
QEMU = tools/qemu/qemu-win.bat
CD = cd

default:
	$(MAKE) run

ipl.bin : ipl/ipl.asm Makefile
	$(NASM) ipl/ipl.asm -o ipl.bin

kl.bin : kl/kl.asm Makefile
	$(NASM) kl/kl.asm -o kl.bin

kernel.bin : 
	($(CD) ./kernel;$(MAKE) run)

term.app :
	($(CD) ./app;$(MAKE) run)

graphos.img : ipl.bin kl.bin kernel.bin term.app Makefile
	$(EDIMG)   imgin:tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0\
		copy from:kl.bin to:@:\
		copy from:kernel.bin to:@:\
		copy from:term.app to:@:\
		copy from:data/sys.fnt to:@:\
		copy from:data/cursor.img to:@:\
		copy from:data/button.img to:@:\
		copy from:data/back.img to:@:\
		copy from:data/caution.img to:@:\
		imgout:graphos.img
		
run :
	$(MAKE) graphos.img
	@echo "Successed!!"

qemu :
	run
	$(QEMU)
