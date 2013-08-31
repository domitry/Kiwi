extern int _text_start;
extern int _text_end;
extern int _bss_start;
extern int _bss_end;

unsigned int TEXT_START = (unsigned int)&_text_start;
unsigned int TEXT_END = (unsigned int)&_text_end;
unsigned int BSS_START = (unsigned int)&_bss_start;
unsigned int BSS_END = (unsigned int)&_bss_end;