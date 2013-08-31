#include "phys_mem.h"
#include "section.h"

static MemMap mm;
static unsigned long long *pde;
static void *heap_last;
 
static int cnt;

static inline void setBit(int bit_num){
	mm.start[bit_num/32] |= (1 << (bit_num % 32));
}

static inline void clearBit(int bit_num){
	mm.start[bit_num/32] &= ~(1 << (bit_num % 32));
}

static inline _Bool checkBit(int bit_num){
	UINT check;
	check = mm.start[bit_num/32] & (1 << (bit_num % 32));
	return (check>0);
}

void kmemset(void *str,unsigned char c,int size){
	unsigned char *ptr = (unsigned char *)str;
	const unsigned char ch = (const unsigned char)c;
	for(;size>=0;size--,ptr++)
		*ptr = ch;
	return;
}

void ksetPDE(UINT phys,UINT vir){
	unsigned long long entry = phys;
	entry &= 0xfff00000;
	entry |= 0b110001011;
	pde[vir/KER_PAGE_SIZE] = entry;
} 

void usetPDE(UINT phys,UINT vir){
	unsigned long long entry = phys;
	entry &= 0xfff00000;
	entry |= 0b110001011;
	pde[vir/KER_PAGE_SIZE] = entry;
}

int ksearchEmptyPage(){
	int i,j;
	//2mb���E��T��
	for(i=PAGE_SEARCH_START_INT;i<mm.entry_num;i+=MME_NUM_IN_2MB){
		if(mm.start[i]==0){
			for(j=0;j<MME_NUM_IN_2MB;j++){
				if(mm.start[i+j]!=0)break;
				if(j==MME_NUM_IN_2MB-1){
					for(j=0;j<MME_NUM_IN_2MB;j++)mm.start[i+j]=0xffffffff;//�����������m��
					return i;
				}
			}
		}
	}
	return 0;
}

_Bool kmmap(void *start,int len){
	int i,num,add;
	num = len/KER_PAGE_SIZE + (len%KER_PAGE_SIZE>0?1:0);
	if((int)start % KER_PAGE_SIZE != 0)return 0;
	for(i=0;i<num;i++){
		add = ksearchEmptyPage();	//�����������̌����A�m��
		ksetPDE(add,(int)start+KER_PAGE_SIZE*i);	//���z�A�h���X��ԂɃ}�b�v
	}
	return 0;
}

_Bool ummap(void *start,int len){
	int i,num,add;
	num = len/KER_PAGE_SIZE + (len%KER_PAGE_SIZE>0?1:0);
	if((int)start % KER_PAGE_SIZE != 0)return 0;
	for(i=0;i<num;i++){
		add = ksearchEmptyPage();	//�����������̌����A�m��
		usetPDE(add,(int)start+KER_PAGE_SIZE*i);	//���z�A�h���X��ԂɃ}�b�v
	}
	return 0;
}

//�C�ӂ̕�����������C�ӂ̉��z�A�h���X�Ƀ}�b�v
_Bool kmmap_phys(void *phys_start,void *vir_start,int len){
	int i,j,num;
	if((int)phys_start%KER_PAGE_SIZE !=0 || (int)vir_start%KER_PAGE_SIZE != 0)return 0;
	num = len / KER_PAGE_SIZE + (len % KER_PAGE_SIZE==0?0:1);//�͂ݏo�����班�����߂Ƀ}�b�v
	for(i=0;i<num;i++){
		for(j=0;j<MME_NUM_IN_2MB;j++)setBit(((UINT)phys_start+KER_PAGE_SIZE*i)%PAGE_SIZE+j);//�����A�h���X�������}�b�v���X�V
		ksetPDE((UINT)phys_start+KER_PAGE_SIZE*i,(UINT)vir_start+KER_PAGE_SIZE*i);//���z�A�h���X��ԂɃ}�b�v
	}
	return 1;
}

void* kmalloc(UINT size){
	int rest = 4-heap_last%4;
	void *result = heap_last+rest;
	heap_last += rest + size;
	return result;
}

void kmemcpy(void *dst,void *src,UINT size){
	int i;
	int rest = 4 - (int)dst % 4;
	i=(int)dst;
	for(i=0;i<rest;i++){	//�ŏ��̃A���C�����g����
		*((UCHAR *)dst + i) = *((UCHAR *)src + i);
	}

	for(i=0;i<(size-rest)/4;i++){	//4byte�P�ʂŃR�s�[
		*((UINT *)(dst+rest) + i) = *((UINT *)(src+rest) + i);
	}
	
	for(i=0;i<(size-rest)%4;i++){	//�����̃A���C�����g����
		*((UCHAR *)(dst+rest+4*((size-rest)/4))+i)
			 = *((UCHAR *)(src+rest+4*((size-rest)/4))+i);
	}
}

void checkMemMapInfo(){
	MemMapInfo *mmi = (MemMapInfo *)MME_MEMO;
	int start,end,i;
	while(mmi->length>0){
		if(mmi->type == OS_CAN_USE){
			start = (int)mmi->base_add/PAGE_SIZE + (mmi->base_add%PAGE_SIZE==0?0:1);
			end = start + (int)(mmi->length + mmi->base_add%PAGE_SIZE)/PAGE_SIZE
				 - ((mmi->length + mmi->base_add%PAGE_SIZE)%PAGE_SIZE==0?0:1);
			for(i=start;i<=end;i++){
				setBit(i);
			}
		}
		mmi++;
	}
}

void initVirMem(){
	pde = (unsigned long long*)PDE;
}

void initMemMap(){
	UINT mem_size;
	int i;
	//�ŏ���2MB���N���A(�s��)
	//pde[0xc0000000/KER_PAGE_SIZE] = 0;
	mm.start = (UINT*)(BSS_END + 1);//kernel�̒����mmap������
	mem_size = *((UINT *)MEM_SIZE_MEMO);
	mem_size = mem_size * 16 + 4 * 1024;	//4kb�P�ʂɕϊ�
	mm.size = mem_size/8;					//4kb���̏���1bit�ɉ������߂�(�璷�ł���������)
	mm.entry_num = mm.size/4;				//int�̔z��ɂ����Ƃ��̗v�f��
	kmemset(mm.start,0xff,mm.size);			//�Sbit��1��
	checkMemMapInfo();						//BIOS����擾�������Ɋ�Â��l���Z�b�g
	for(i=0;i<PAGE_SEARCH_START_BIT;i++)setBit(i);			//����܂Ŏg�����̈���m��(20mb��)
	
	heap_last = (void *)mm.start + mm.size;

	cnt = 0;
}

