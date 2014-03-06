#include "fs.h"
#include "phys_mem.h"

static Fat12_Info fat12_info;

void initFs(){
	fat12_info.fat_add = (UCHAR *)FAT;
	fat12_info.root_add = (UCHAR *)ROOT_DIR;
	fat12_info.data_add =  (UCHAR *)DATA_SEG;
}

int strlen(const char *str){
	int i;
	for(i=0;i<INF;i++)if(str[i]=='\0')return i;
	return -1;
}

DirLayout* searchFile(char *filename){
	int i,j;
	char name[12];
	DirLayout *seek = (DirLayout *)fat12_info.root_add;
	
	if(strlen(filename)>12)return 0;	//ロングネーム非対応
	
	//比較しやすいようにファイル名を大文字に、拡張子のドットを削除(Font.sys->FONTSYS)
	for(i=0,j=0;i<13;i++,j++){
		if(filename[i]=='.'){
			j--;
			continue;
		}
		if(filename[i]=='\0'||i==11){
			name[j]='\0';
			break;
		}
		if('a'<=filename[i])name[j]=filename[i]-('a'-'A');
		else name[j]=filename[i];
	}

	while(seek < (DirLayout *)fat12_info.data_add){
		for(i=0,j=0;j<11;i++,j++){
			if(seek->file_name[j]==0x20){//space
				i--;
				continue;
			}
			if(seek->file_name[j] != name[i])break;
			if(i==strlen(filename)-2)return seek;
		}
		seek++;
	}
	return 0;
}

int getNextCluster(int cl_num){
	UCHAR *fat_seek = fat12_info.fat_add;
	int offset = cl_num*12;
	UCHAR tmp[2];
	short result;
	tmp[0] = ((UCHAR *)(fat_seek+offset/8))[0];
	tmp[1] = ((UCHAR *)(fat_seek+offset/8))[1];
	if(offset % 8==0){
		tmp[1] &= 0x0f;
		result =  *((short *)tmp);
	}else{
		tmp[0] &= 0xf0;
		result = *((short *)tmp);
		result = result >> 4;
	}
	return result;
}

static inline void* getClusterAdd(int cl_num){
	return (fat12_info.data_add + (cl_num-2)*CLUSTER_SIZE);
}

void* loadFile(char *filename){
	DirLayout *dir = searchFile(filename);
	UCHAR *start = kmalloc(dir->size);	//コピー先確保
	UCHAR *seek = start;
	int cl_num = dir->head_cluster;
	int limit = dir->size / CLUSTER_SIZE + (dir->size % CLUSTER_SIZE==0?0:1);
	int cnt=1;
	
	if(start == 0)return 0;//mallocできなかった

	while(cl_num != 0xfff){
		if(cnt==INF)return 0;//FATがうまく読めてない
		
		//最後のあまりをコピー
		if(cnt==limit){
			kmemcpy(seek,getClusterAdd(cl_num),dir->size%CLUSTER_SIZE);
			break;
		}
		
		kmemcpy(seek,getClusterAdd(cl_num),CLUSTER_SIZE);
		
		cl_num = getNextCluster(cl_num);
		seek += CLUSTER_SIZE;
		
		cnt++;
	}
/*
	if(filename[0]=='l' && filename[1]=='i'){
		asm volatile("cli");
		asm volatile("mov %0,%%eax"::"r"((int)dir):"eax");
		for(;;)asm volatile("hlt");
	}
*/	
	return start;
}


//指定アドレスにファイルをロード
void* loadFileToMem(void *add,char *filename){
	DirLayout *dir = searchFile(filename);
	UCHAR *start = add;	//コピー先確保
	UCHAR *seek = start;
	int cl_num = dir->head_cluster;
	int limit = dir->size / CLUSTER_SIZE + (dir->size % CLUSTER_SIZE==0?0:1);
	int cnt=1;
	if(start == 0)return 0;//mallocできなかった

	while(cl_num != 0xfff){
		if(cnt==INF)return 0;//FATがうまく読めてない
		
		//最後のあまりをコピー
		if(cnt==limit){
			kmemcpy(seek,getClusterAdd(cl_num),dir->size%CLUSTER_SIZE);
			break;
		}
		kmemcpy(seek,getClusterAdd(cl_num),CLUSTER_SIZE);
		cl_num = getNextCluster(cl_num);
		seek += CLUSTER_SIZE;
		cnt++;
	}
	return start;
}

