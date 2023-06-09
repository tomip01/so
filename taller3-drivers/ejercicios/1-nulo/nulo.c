#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

// struct vacia
static struct cdev cdev;
static dev_t major;
char* name = "nulo";

static struct class *mi_class;

static ssize_t nulo_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
	// Completar
	return 0;
}

static ssize_t nulo_write(struct file *filp, const char __user *data, size_t s, loff_t *off) {
	// Completar
	return 0;
}

static struct file_operations nulo_operaciones = {
	// Completar
	.owner = THIS_MODULE,
	.read = nulo_read,
	.write = nulo_write
};

static int __init nulo_init(void) {
	// Completar
	cdev_init(&cdev,&nulo_operaciones);

	alloc_chrdev_region(&major,0,1,name);
	cdev_add(&cdev,major,1);

	mi_class = class_create(THIS_MODULE, name);
	device_create(mi_class, NULL, major, NULL, name);
	return 0;
}

static void __exit nulo_exit(void) {
	// Completar
	// orden inverso a la creacion
	device_destroy(mi_class, major);
	class_destroy(mi_class);

	// desregistro, desasigna el major, en la destruccion no tendria major valido
	unregister_chrdev_region(major,1);
	cdev_del(&cdev);

}

// Completar
module_init(nulo_init);
module_exit(nulo_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de '/dev/null'");