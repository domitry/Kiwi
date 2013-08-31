#include "graphicApp.h"


int appMain(){
	Color color = {255,0,0};
	initWindow(320,400);
	for(;;){
		fill(color);
		asm("hlt"); 
	}
}