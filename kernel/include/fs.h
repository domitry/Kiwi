#ifndef FS_H
#define FS_H

#define FAT		0xc0009200
#define ROOT_DIR	0xc000a400
#define DATA_SEG	0xc000c000

#define READ_ONLY	0x01
#define SHADOW		0x02
#define SYSTEM_FILE	0x04
#define VOLUME		0x08
#define DIR			0x10
#define COMPRESS	0x20
#define LONG_NAME	0x0f

#define CLUSTER_SIZE 512

#define INF 0xfff

typedef unsigned char UCHAR;
typedef unsigned int UINT;

typedef struct Fat12_Info{
	UCHAR *fat_add;
	UCHAR *root_add;
	UCHAR *data_add;
}Fat12_Info;

struct DirLayout{
	char file_name[11];
	UCHAR file_type;
	UCHAR lower_case_flag;
	UCHAR make_time_low;//10ms’PˆÊ,~2s
	short make_time_high;
	short make_date;
	short last_ac_date;
	short reseve;
	short update_time;
	short update_date;
	short head_cluster;
	UINT size;
}__attribute__((packed));
typedef struct DirLayout DirLayout;

void initFs();
void* loadFile(char *filename);
void* loadFileToMem(void *add,char *filename);


#endif