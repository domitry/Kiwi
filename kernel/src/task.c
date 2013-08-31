#include "task.h"
#include "phys_mem.h"

static TaskInfo task_info;

void saveRegisters(int task_num){
	asm("mov %%eax,%0":"=r"(task_info.tasks[task_num].reg.eax));
	asm("mov %%ebx,%0":"=r"(task_info.tasks[task_num].reg.ebx));
	asm("mov %%ecx,%0":"=r"(task_info.tasks[task_num].reg.ecx));
	asm("mov %%edx,%0":"=r"(task_info.tasks[task_num].reg.edx));
	asm("mov %%esp,%0":"=r"(task_info.tasks[task_num].reg.esp));
	asm("mov %%ebp,%0":"=r"(task_info.tasks[task_num].reg.ebp));
	asm("mov %%esi,%0":"=r"(task_info.tasks[task_num].reg.esi));
	asm("mov %%edi,%0":"=r"(task_info.tasks[task_num].reg.edi));
	
	//eipとeflagsまだ
}

void registerTask(){
	task_info.task_num++;
}

void exec(char *filename){
	asm("cli");
	ummap((void *)0x00200000,0x400000);
	

	loadFileToMem(0x00200000,filename);

	//task登録
	registerTask();
	asm("sti");

	asm("jmp 0x00200000");
}

int searchNextTask(int cur_task){
	return 0;
}


//これを一定間隔で呼び出す
void* switchTask(){
	int next_task;
	if(task_info.task_num==1)return 0;//実行中のタスクがひとつ
	task_info.tasks[task_info.cur_task].cnt++;
	if(task_info.tasks[task_info.cur_task].cnt > task_info.tasks[task_info.cur_task].limit){
		//レジスタの保存
		saveRegisters(task_info.cur_task);
		//次のタスクを探す
		next_task = searchNextTask(task_info.cur_task);
		
		//(仮想アドレス空間を変えるならここ)
		//遷移先のアドレスを返す
		return (void *)task_info.tasks[next_task].reg.eip;
	}
	return 0;
}
