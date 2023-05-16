#include <linux/init.h>
#include <linux/module.h> // THIS_MODULE
#include <linux/kernel.h> // Kernel cosas
#include <linux/fs.h>     // File operations
#include <linux/cdev.h>   // Char devices
#include <linux/device.h> // Nodos filesystem
#include <linux/uaccess.h> // copy_to_user
#include <linux/slab.h>    // kmalloc
#include <linux/random.h>  // get_random_bytes

static struct cdev cdev;
dev_t major;
char* name = "azar";
unsigned int numero_max;
bool escribieron = false;

static struct class *mi_class;

static ssize_t azar_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
    unsigned int rand;
	if (!escribieron) // nunca escribieron, devuelvo error
        return -EPERM;
    
    get_random_bytes(&rand, sizeof(rand));
    rand = rand % numero_max;

    char * buffer = kmalloc(s, GFP_KERNEL);
    int size = snprintf(buffer, s, "%d\n", rand);
    if (size < 0)
        return -EPERM; // no se si es necesario

    copy_to_user(data, buffer, size);
    kfree(buffer);

	return size;
}

static ssize_t azar_write(struct file *filp, const char __user *data, size_t s, loff_t *off) {
	
    //asigno memoria
    char* buffer = kmalloc(s+1, GFP_KERNEL);
    copy_from_user(buffer,data,s);
    buffer[s] = '\0';               // agrego caracter nulo, enunciado
    int resultado = kstrtoint(buffer,0,&numero_max); // convierto string a entero

    if (resultado != 0) // no pude convertir
        return -EPERM;

    escribieron = true;

    kfree(buffer);
	return s;
}

static struct file_operations azar_operaciones = {
	
	.owner = THIS_MODULE,
	.read = azar_read,
	.write = azar_write
};

static int __init azar_init(void) {
	
	cdev_init(&cdev,&azar_operaciones);

	alloc_chrdev_region(&major,0,1,name);
	cdev_add(&cdev,major,1);

	mi_class = class_create(THIS_MODULE, name);
	device_create(mi_class, NULL, major, NULL, name);
	return 0;
}

static void __exit azar_exit(void) {
	
	device_destroy(mi_class, major);
	class_destroy(mi_class);

	unregister_chrdev_region(major,1);
	cdev_del(&cdev);

}

// Completar
module_init(azar_init);
module_exit(azar_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Generador de números al azar");