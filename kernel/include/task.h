#define TASK_MAX 5

typedef unsigned char UCHAR;
typedef unsigned int UINT;


typedef struct{
	int cr3;
	int eip,eflags,eax,ebx,ecx,edx,esp,ebp,esi,edi;
}Register;

typedef struct{
	int cnt;
	int limit;
	UCHAR *buffer;
	UCHAR is_exist_context;
	Register reg;
}Task;

typedef struct{
	int cur_task;
	int task_num;
	Task tasks[TASK_MAX];
}TaskInfo;


void exec(char *filename);
void* switchTask();
void registerTask();

