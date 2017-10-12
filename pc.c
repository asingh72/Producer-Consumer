#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include <linux/device.h>         

#define DEVICE_NAME "Producer_Consumer"    
#define CLASS_NAME  "proc" 
#define LICENSE "GPL"

MODULE_LICENSE(LICENSE);

static struct miscdevice pc;
static int buffer_size;
char* device_name = "lp";
static int string_count= 25;
static int rindex = 0, windex = 0;
static int buff_empty;
static struct semaphore full;
static struct semaphore empty;
static struct semaphore rmutex;
static struct semaphore wmutex;


static int majorNumber;                  
static struct class*  processClass  = NULL; 
static struct device* processDevice = NULL; 

module_param(buffer_size, int, 0000);

char** pc_buffer;

static ssize_t pc_read(struct file*, char*, size_t, loff_t*);
static ssize_t pc_write(struct file*, const char*, size_t, loff_t*);
static int pc_release(struct inode*, struct file*);


static struct file_operations fops = {
	.read = &pc_read,
	.write = &pc_write,
	.release = &pc_release
};


static int __init pc_init(void){

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "Failed to register a major number\n");
	        return majorNumber;
   	}
	printk(KERN_INFO "Registered correctly with major number %d\n", majorNumber);
	processClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(processClass)){                
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(processClass);          
	}
	processDevice = device_create(processClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   	if (IS_ERR(processDevice)){               
		class_destroy(processClass);           
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(processDevice);
   	}

	pc.name = device_name;
	pc.minor = MISC_DYNAMIC_MINOR;
	pc.fops = &fops;
	
	
	int register_return_value;
	if((register_return_value = misc_register(&pc))){
		
		printk(KERN_ERR "Could not register the device\n");
		return register_return_value;
	}
	
	int _allocated = 0;
	pc_buffer = (char**)kmalloc(buffer_size*sizeof(char*), GFP_KERNEL);
	while(_allocated < buffer_size){
		pc_buffer[_allocated] = (char*)kmalloc((string_count+1)*sizeof(char), GFP_KERNEL);
		pc_buffer[string_count] = '\0';
		++_allocated;
	}
	sema_init(&full, 0);
	sema_init(&empty, buffer_size);
	sema_init(&rmutex, 1);
	sema_init(&wmutex, 1);
	buff_empty = buffer_size;

	return 0;
}

static ssize_t pc_read(struct file* _file, char* buffer_r, size_t len, loff_t* offset){
	
	int user_index = 0;
	down_interruptible(&rmutex);
	down_interruptible(&full);
	rindex = (rindex%buffer_size);
	while(user_index < len)
	{
		if(buff_empty >= buffer_size){
			break;
		}
		copy_to_user(&buffer_r[user_index], &pc_buffer[rindex][user_index], 1);
		user_index++;
	}
	++rindex;
	++buff_empty;
	up(&empty);
	up(&rmutex);
	return user_index;
}


static ssize_t pc_write(struct file* _file, const char* buffer_w, size_t len, loff_t* offset){
	
	int user_index = 0;	
	down_interruptible(&wmutex);
	down_interruptible(&empty);
	windex = (windex%buffer_size);
	while(user_index < len)
	{
		if(buff_empty <= 0){
			break;
		}
		copy_from_user(&pc_buffer[windex][user_index], &buffer_w[user_index], 1);
		user_index++;
	}
	++windex;
	--buff_empty;
	up(&full);
	up(&wmutex);
	return user_index;
}


static int pc_release(struct inode* _inode, struct file* _file){
	
	return 0;
}

static void __exit pc_exit(void)
{
	device_destroy(processClass, MKDEV(majorNumber, 0));     
   	class_unregister(processClass);                          
   	class_destroy(processClass);                             
   	unregister_chrdev(majorNumber, DEVICE_NAME);            
}
module_init(pc_init);
module_exit(pc_exit);
