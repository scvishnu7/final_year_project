#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/fs_struct.h>
#include <linux/types.h>

int init_module(void);
void cleanup_module(void);

static int pid1;
static int pid2;
static struct task_struct *task1;
static struct task_struct *task2;

int init_module(void){
	struct files_struct *files;

	printk( "Module inserted\n");
	
	for_each_process(task1) {
		if( task1->pid == pid1)
			break;
	}
	for_each_process(task2) {
		if(task2->pid == pid2)
			break;
	}

	printk("==========================\n");
	printk(" Swapping files_struct of:\n");
	printk(" %s:%d and %s:%d\n",task1->comm, task1->pid, task2->comm, task2->pid);
	printk("==========================\n");

	/*swaping the files_struct  struct of open fd's */

	files = kmalloc(sizeof(struct files_struct), GFP_KERNEL);
	memcpy(files, task1->files, sizeof(struct files_struct));
	memcpy(task1->files, task2->files, sizeof(struct files_struct));
	memcpy(task2->files, files, sizeof(struct files_struct));
	
	return 0;
}

void cleanup_module(void) {
	printk("Module removed\n");

}

module_param(pid1, int ,1);
MODULE_PARM_DESC(pid1, "first pid");

module_param(pid2, int , 1);
MODULE_PARM_DESC(pid2, "second pid");
