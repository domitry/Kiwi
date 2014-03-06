#include "task.h"
#include "fs.h"
#include "phys_mem.h"
#include "elf.h"
#include "string.h"

static TaskInfo task_info;

/*
http://d.hatena.ne.jp/mFumi/20121229/135677
http://wiki.osdev.info/?ELF%2F%BC%C2%B9%D4%BB%FE%A4%CE%CF%C3
http://wiki.osdev.info/?ELF%2F%BC%C2%B9%D4%BB%FE%A4%CE%CF%C3%2F%A5%D7%A5%ED%A5%BB%A5%C3%A5%B5%A4%CB%B0%CD%C2%B8%A4%B9%A4%EB%CF%C3%2Fi386
http://lxr.free-electrons.com/source/fs/binfmt_elf.c#L815
http://7shi.hateblo.jp/entry/2013/05/28/002100
http://7shi.hateblo.jp/entry/2013/05/25/103050
http://d.hatena.ne.jp/sleepy_yoshi/20090510/p1
http://d.hatena.ne.jp/yupo5656/20071115/p1
*/

void saveRegisters(int task_num){
	asm("mov %%eax,%0":"=r"(task_info.tasks[task_num].reg.eax));
	asm("mov %%ebx,%0":"=r"(task_info.tasks[task_num].reg.ebx));
	asm("mov %%ecx,%0":"=r"(task_info.tasks[task_num].reg.ecx));
	asm("mov %%edx,%0":"=r"(task_info.tasks[task_num].reg.edx));
	asm("mov %%esp,%0":"=r"(task_info.tasks[task_num].reg.esp));
	asm("mov %%ebp,%0":"=r"(task_info.tasks[task_num].reg.ebp));
	asm("mov %%esi,%0":"=r"(task_info.tasks[task_num].reg.esi));
	asm("mov %%edi,%0":"=r"(task_info.tasks[task_num].reg.edi));
	
	//eip��eflags�܂�
}

void registerTask(){
	task_info.task_num++;
}

int DEntryFromTag(Elf32_Dyn *dyn_entry,Elf32_Sword tag){
	int seek=0;
	while(dyn_entry[seek].d_tag!=DT_NULL){
		if(dyn_entry[seek].d_tag == tag){
			return seek;
		}
		seek++;
	}
	return -1;
}

void initTask(){
	task_info.cur_task = 0;
	task_info.task_num = 0;
}

void loadSharedLib(char *filename){
	Task *self_task;
	Elf32_Ehdr *elf;
	int offset;
	
	offset = 0x06000000;//���ꂾ�����炷
	
	//Task�o�^
	self_task = &(task_info.tasks[task_info.cur_task]);
	self_task->elf_info[self_task->elf_num].start = (void *)offset;
	self_task->elf_info[self_task->elf_num].type = ELF_TYPE_SO;
	self_task->elf_num++;
	
	kmmap((void *)offset,0x400000);
	
	elf = (Elf32_Ehdr *)loadFile(filename);
	
	readElf(elf,offset);
		
	return;
}

// originally code : http://wiki.osdev.info
unsigned long elf_hash(const unsigned char *name)
{
	unsigned long h = 0, g;
	while (*name)
	{
		h = (h << 4) + *name++;
		if (g = h & 0xf0000000)
		h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

void* findSymbolAddrFromSo(char *sym_name){
	int i,j;
	Task *self_task = &(task_info.tasks[task_info.cur_task]);
	unsigned long hash = elf_hash((unsigned char*)sym_name);

	for(i=0;i<self_task->elf_num;i++){
		if(self_task->elf_info[i].type == ELF_TYPE_SO){
			
			Elf32_Ehdr *elf = (Elf32_Ehdr *)(self_task->elf_info[i].start);
			Elf32_Phdr *p_header = (Elf32_Phdr *)((int)elf + (int)elf->e_phoff);
			
			
			for(j=0;j<elf->e_phnum;j++){
				if(p_header[j].p_type == PT_DYNAMIC){
					Elf32_Dyn 	*dyn_info = (Elf32_Dyn*)((int)elf + (int)p_header[i].p_vaddr);
					Elf32_Sym 	*sym_table = (Elf32_Sym *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_SYMTAB)].d_un.d_val);
					int		 	*hash_table = (int*)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_HASH)].d_un.d_val);
					char 		*str_table = (char *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_STRTAB)].d_un.d_val);
					Elf32_Sym *seek = sym_table;
					// �n�b�V���e�[�u���T���Ɏ��s���܂����Ă炷����̂ŃV���{�����S�T���ɕύX
					while((int)seek<(int)str_table){
						if(strcmp(sym_name,&(str_table[seek->st_name]))){
							return (void *)((int)elf + seek->st_value);
						}
						else seek++;
					}
					/*
					int nbucket = hash_table[0];
					//int nchain = hash_table[1];
					int *bucket = hash_table + 2;
					int *chain = bucket + nbucket;
					
					int sym_no = bucket[nbucket % hash];
					
					__asm__ volatile("movl %0, %%eax"::"r"((int)hash));
					for(;;)asm volatile("hlt");
					
					do{
						Elf32_Sym *symbol = &(sym_table[sym_no]);
						
						if(strcmp(sym_name,&(str_table[symbol->st_name])))
							return (void *)((int)elf + symbol->st_value);
						else
							sym_no = chain[sym_no];
					}while(chain[sym_no]!=STN_UNDEF);*/
				}
			}
		}
	}
	return (void *)INF_ADDR;
}

void* getSValue(Elf32_Sym *sym_entry,Elf32_Rel *rel_entry,char *str_table){
	//�V���{�����T��
	int str_offset = sym_entry[ELF32_R_SYM(rel_entry->r_info)].st_name;
	if(str_offset == 0)return (void *)INF_ADDR;
	return findSymbolAddrFromSo(&(str_table[str_offset]));
}

void rerocate(int offset,Elf32_Rel *rel_entry,int rel_size,Elf32_Sym *sym_entry,int sym_e_size,char *str_table,int str_size,void *addr_got){
	int rel_e_num = rel_size / (int)sizeof(Elf32_Rel);
	int i;
	
	for(i=0;i<rel_e_num;i++){
		int result = 0;
		int *dest = (int*)(offset + rel_entry[i].r_offset);
		
		switch(ELF32_R_TYPE(rel_entry[i].r_info)){
			case R_386_32:
				result = (int)getSValue(sym_entry,rel_entry+i,str_table) + *dest;
				break;
			case R_386_PC32:
				result = (int)getSValue(sym_entry,rel_entry+i,str_table) + *dest - rel_entry[i].r_offset;
				break;
			case R_386_GOT32:
				//�s��(G�̋��ߕ��킩��Ȃ�)
				break;
			case R_386_PLT32:
				//�s��
				break;
			case R_386_COPY:
				//���
				break;
			case R_386_GLOB_DAT:
				result = (int)getSValue(sym_entry,rel_entry+i,str_table);
				break;
			case R_386_JMP_SLOT:
				result = (int)getSValue(sym_entry,rel_entry+i,str_table);
				break;
			case R_386_RELATIVE:
				result = offset + *dest;
				break;
			case R_386_GOTOFF:
				result = (int)getSValue(sym_entry,rel_entry+i,str_table) + *dest - (int)addr_got;
				break;
			case R_386_GOTPC:
				result = (int)addr_got + *dest - rel_entry[i].r_offset;//�Ԉ���Ă邩��
				break;
		}
		*dest = result;//�Ĕz�u
	}
}

// elf�w�b�_�ւ̃|�C���^�A���z�A�h���X��̓W�J�ʒu
void readElf(Elf32_Ehdr *elf,int offset){
	Elf32_Phdr *p_header = (Elf32_Phdr *)((int)elf + (int)elf->e_phoff);
	int i;
	
	//ProgramHeader�̏��Ɋ�Â��ă������ɓW�J(������DEP�֘A�̏����ǉ�)
	for(i=0;i<elf->e_phnum;i++){
		if(p_header[i].p_type == PT_LOAD){
			kmemcpy((void*)(offset + p_header[i].p_vaddr),(void*)((int)elf + p_header[i].p_offset),p_header[i].p_filesz);
		}
	}

	//ProgramHeader�̒�����Dynamic��T��
	for(i=0;i<elf->e_phnum;i++){
		if(p_header[i].p_type == PT_DYNAMIC){
			Elf32_Dyn *dyn_info = (Elf32_Dyn*)((int)elf + (int)p_header[i].p_offset);
			
			//���L���C�u���������[�h
			if(DEntryFromTag(dyn_info,DT_NEEDED)!=-1 && DEntryFromTag(dyn_info,DT_SYMTAB)!=-1){
				char *sym_name = 
					(char *)((int)elf + (int)dyn_info[DEntryFromTag(dyn_info,DT_STRTAB)].d_un.d_val + (int)dyn_info[DEntryFromTag(dyn_info,DT_NEEDED)].d_un.d_val);
				loadSharedLib(sym_name);
			}
			
			//�Ĕz�u���ɂ��ƂÂ��čĔz�u
			if(DEntryFromTag(dyn_info,DT_JMPREL)!=-1)//PLT�̍Ĕz�u
				rerocate(
					offset,			//���t�@�C���ʒu
					(Elf32_Rel *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_JMPREL)].d_un.d_val),	//�Ĕz�u�e�[�u���̈ʒu
					(int)dyn_info[DEntryFromTag(dyn_info,DT_PLTRELSZ)].d_un.d_val,						//���T�C�Y
					(Elf32_Sym *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_SYMTAB)].d_un.d_val),	//�V���{���e�[�u���̈ʒu
					(int)dyn_info[DEntryFromTag(dyn_info,DT_SYMENT)].d_un.d_val,							//��1�G���g�����Ƃ̃T�C�Y
					(char *)elf + dyn_info[DEntryFromTag(dyn_info,DT_STRTAB)].d_un.d_val,				//�V���{�����e�[�u���̈ʒu
					(int)dyn_info[DEntryFromTag(dyn_info,DT_STRSZ)].d_un.d_val,							//���T�C�Y
					(DEntryFromTag(dyn_info,DT_PLTGOT)!=-1?
						((void *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_PLTGOT)].d_un.d_val)):(void *)INF_ADDR)	//GOT�̃A�h���X
				);
			
			if(DEntryFromTag(dyn_info,DT_REL)!=-1)//���̑��̍Ĕz�u
				rerocate(
					offset,			//���t�@�C���ʒu
					(Elf32_Rel *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_REL)].d_un.d_val),		//�Ĕz�u�e�[�u���̈ʒu
					(int)dyn_info[DEntryFromTag(dyn_info,DT_RELSZ)].d_un.d_val,							//���T�C�Y
					(Elf32_Sym *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_SYMTAB)].d_un.d_val),	//�V���{���e�[�u���̈ʒu
					(int)dyn_info[DEntryFromTag(dyn_info,DT_SYMENT)].d_un.d_val,							//��1�G���g�����Ƃ̃T�C�Y
					(char *)elf + dyn_info[DEntryFromTag(dyn_info,DT_STRTAB)].d_un.d_val,				//�V���{�����e�[�u���̈ʒu
					(int)dyn_info[DEntryFromTag(dyn_info,DT_STRSZ)].d_un.d_val,							//���T�C�Y
					(DEntryFromTag(dyn_info,DT_PLTGOT)!=-1?
						((void *)((int)elf + dyn_info[DEntryFromTag(dyn_info,DT_PLTGOT)].d_un.d_val)):(void *)INF_ADDR)	//GOT�̃A�h���X
				);
		}
	}
}


void exec(char *filename){
	Elf32_Ehdr *elf;
	int offset;
	Task *self_task;
	int start_add;//jmp to this address
	
	asm("cli");
	
	offset = 0x02000000;//���ꂾ�����炷

	// Task�o�^
	self_task = &(task_info.tasks[task_info.cur_task]);
	self_task->elf_info[0].start = (void *)offset;
	self_task->elf_info[0].type = ELF_TYPE_EXEC;
	self_task->elf_num = 1;
	task_info.task_num++;
	
	kmmap((void *)offset,0x400000);
	
	elf = (Elf32_Ehdr *)loadFile(filename);
	
	readElf(elf,offset);//elf�ǂ�
	
	// JMP����(�����I�ɍ폜)
	start_add = offset + (int)(elf->e_entry);
	
	__asm__ volatile("sti");
	__asm__ volatile("movl %0, (%%esp);ret"::"r"((int)start_add));
	return;
}


int searchNextTask(int cur_task){
	return 0;
}

//��������Ԋu�ŌĂяo��
void* switchTask(){
	int next_task;
	if(task_info.task_num==1)return 0;//���s���̃^�X�N���ЂƂ�
	task_info.tasks[task_info.cur_task].cnt++;
	if(task_info.tasks[task_info.cur_task].cnt > task_info.tasks[task_info.cur_task].limit){
		//���W�X�^�̕ۑ�
		saveRegisters(task_info.cur_task);
		//���̃^�X�N��T��
		next_task = searchNextTask(task_info.cur_task);
		
		//(���z�A�h���X��Ԃ�ς���Ȃ炱��)
		//�J�ڐ�̃A�h���X��Ԃ�
		return (void *)task_info.tasks[next_task].reg.eip;
	}
	return 0;
}
