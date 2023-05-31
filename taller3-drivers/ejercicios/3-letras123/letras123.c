#include <linux/init.h>
#include <linux/module.h> // THIS_MODULE
#include <linux/kernel.h> // Kernel cosas
#include <linux/fs.h>     // File operations
#include <linux/cdev.h>   // Char devices
#include <linux/device.h> // Nodos filesystem
#include <linux/uaccess.h> // copy_to_user
#include <linux/slab.h>    // kmalloc
#include <linux/random.h>  // get_random_bytes
// #include <semaphore.h>

static struct cdev cdev;
dev_t major;
char* name = "letras";
bool disp[3] = {true, true, true};
struct semaphore mtx;

static struct class *mi_class;

typedef struct foo {
    int id;
    char letter;
} foo_t;


static ssize_t letras_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
    foo_t * userdata = (foo_t*)filp->private_data;
    if(userdata->letter == '\0')
        return -EPERM;

    char* buff = kmalloc(sizeof(s), GFP_KERNEL);
    int i;
    for (i = 0; i < s; ++i) { 
        buff[i] = userdata->letter;
    }

    copy_to_user(data,buff,s);
    kfree(buff);
    
	return s;
}

static ssize_t letras_write(struct file *filp, const char __user *data, size_t s, loff_t *off) {
    if (((foo_t*)filp->private_data)->letter != '\0')
        return 0;

    char* buff = kmalloc(sizeof(1), GFP_KERNEL);
    copy_from_user(buff,data,1);
    ((foo_t*)filp->private_data)->letter = *buff;
    kfree(buff);
	
	return 1;
}

static int letras_open(struct inode* inode, struct file* file) {
    down(&mtx);
    int i = 0;
    while (i < 3 && !disp[i]) ++i;

    if (i == 3) return -EPERM;
    disp[i] = false;
    up(&mtx);

    foo_t *userdata = kmalloc(sizeof(foo_t), GFP_KERNEL);
    userdata->id = i;
    userdata->letter = '\0';
    file->private_data = userdata;

    return 0;
}

static int letras_release(struct inode* inode, struct file* file) {
    down(&mtx);
    int id = ((foo_t*)file->private_data)->id;
    disp[id] = true;
    up(&mtx);
    kfree(file->private_data);

    return 0;
}

static struct file_operations letras_operaciones = {
	
	.owner = THIS_MODULE,
	.read = letras_read,
	.write = letras_write,
    .open = letras_open,
    .release = letras_release
};

static int __init letras_init(void) {
	
	cdev_init(&cdev,&letras_operaciones);

	alloc_chrdev_region(&major,0,1,name);
	cdev_add(&cdev,major,1);

	mi_class = class_create(THIS_MODULE, name);
	device_create(mi_class, NULL, major, NULL, name);

    sema_init(&mtx,1);
	return 0;
}

static void __exit letras_exit(void) {
	
	device_destroy(mi_class, major);
	class_destroy(mi_class);

	unregister_chrdev_region(major,1);
	cdev_del(&cdev);

}

module_init(letras_init);
module_exit(letras_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Generador de letras");