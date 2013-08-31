#include "section.h"
#include "phys_mem.h"
#include "graphic.h"
#include "fs.h"
#include "font.h"
#include "interp.h"
#include "wsys.h"
#include "task.h"
#include <stdio.h>

static InterpBuffer *key,*mouse;

int kmain(void){
	
	kmemset((void *)BSS_START,0x00,BSS_END-BSS_START);
	initVirMem();
	initMemMap();
	initGraphic();
	initFs();
	
	loadFont("sys.fnt");
	
	initInterp(&key,&mouse);
	initWsys(key,mouse);

	exec("term.app");
	
	for(;;){
		__asm__("hlt");
	}
}