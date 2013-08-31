#ifndef PHYS_MEM_H
#define PHYS_MEM_H

#define KER_PAGE_SIZE 0x200000
#define PDE 0xc0100000
#define PDPT 	 0xc0104000
#define MME_MEMO 0xc0007afc
#define MEM_SIZE_MEMO 0xc0007af8
#define OS_CAN_USE 0x0001
#define PAGE_SIZE 0x1000
#define PAGE_SEARCH_START_BIT 0x1400 //最初の20mbはカーネルが確保しているので使わない
#define PAGE_SEARCH_START_BYTE 0x280
#define PAGE_SEARCH_START_INT 0xa0 //intの配列にしたらこれだけ
#define MME_NUM_IN_2MB 16

typedef unsigned int UINT;
typedef unsigned char UCHAR;

typedef struct MemMap{
	unsigned int *start;
	unsigned int size;//byte
	unsigned int entry_num;
}MemMap;

struct MemMapInfo{
	unsigned long long base_add;
	unsigned long long length;
	unsigned int type;
}__attribute__((packed));
typedef struct MemMapInfo MemMapInfo;


void kmemset(void *str,unsigned char c,int size);
_Bool kmmap_phys(void *phys_start,void *vir_start,int len);
void *kmalloc(UINT size);
void kmemcpy(void *dst,void *src,UINT size);
void initMemMap();
void initVirMem();

_Bool ummap(void *start,int len);


#endif
