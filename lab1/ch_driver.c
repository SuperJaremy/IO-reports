#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#define BUFFER_SIZE 256

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Character device driver");
MODULE_AUTHOR("Vladislav Saburov");
MODULE_VERSION("0");


static dev_t first;
static struct cdev c_dev;
static struct class *cl;
static struct proc_dir_entry *in_proc;
static size_t spaces[BUFFER_SIZE] = {0};
static size_t writes = 0;

size_t count_spaces(char* str, size_t size){
    size_t count = 0;
    size_t i = 0;
    for(; i < size; i++){
        if(str[i] == ' ')
            count++;
    }
    return  count;
}

static ssize_t ch_write(struct file *f, const char __user *buf, size_t len, loff_t *off){
    size_t count = 0;
    char str[BUFFER_SIZE] = {0};
    if(writes >= BUFFER_SIZE - 1)
        writes = 0;
    printk(KERN_INFO "/dev writing began\n");
    for(;;){
        if(BUFFER_SIZE >= len){
            if(copy_from_user(str, buf, len) != 0){
                printk(KERN_ERR "Error while writing\n");
                return -EFAULT;
            }
            count += count_spaces(str, len);
            break;
        }
        else{
            if(copy_from_user(str, buf, BUFFER_SIZE) != 0){
                printk(KERN_ERR "Error while writing\n");
                return -EFAULT;
            }
            count += count_spaces(str, BUFFER_SIZE);
            len -= BUFFER_SIZE;
        }
    }
    spaces[writes] = count;
    writes++;
    printk(KERN_INFO "/dev writing succeeded\n");
    return len;
}

static ssize_t proc_read(struct file *f, char __user *buf, size_t len, loff_t *off){
    char str[BUFFER_SIZE] = { 0 };
    char try[10] = {0};
    size_t read = 0;
    size_t count = 0;
    printk(KERN_INFO "/proc reading began\n");
    printk(KERN_INFO "len = %zu\n", len);
    printk(KERN_INFO "offset = %llu\n", *off);
    if(writes == 0)
        return 0;
    while(read < *off){
        read+= sprintf(try, "%zu\n", spaces[count]);
        count++;
        if(count == writes)
            break;
    }
    read = 0;
    while(count < writes){
        size_t l = sprintf(try, "%zu\n", spaces[count]);

        if((l < len - read) && l < BUFFER_SIZE - read){
            sprintf(&str[read], "%zu\n", spaces[count]);
            printk(KERN_DEBUG "copied number %lu\n", spaces[count]);
            printk(KERN_DEBUG "copied %lu bytes\n", read);
            read += l;
            count++;
        }
        else
            break;
    }
    if(copy_to_user(buf, str, read) != 0){
        printk(KERN_ERR "Error while writing");
        return -EFAULT;
    }
    printk(KERN_INFO "/proc reading ended\n");
    printk(KERN_INFO "bytes read %zu\n", read);
    *off = read;
    return read;
}

static ssize_t ch_read(struct file *f, char __user *buf, size_t len, loff_t *off){
    size_t i = 0;
    printk(KERN_INFO "Writing to read buffer began\n");
    for(; i < writes; i++){
        printk(KERN_INFO "%zu\n", spaces[i]);
    }
    printk(KERN_INFO "Writing to read buffer ended\n");
    return 0;
}

static const struct file_operations proc_fops = {
        .owner = THIS_MODULE,
        .read = proc_read
};

static const struct file_operations ch_fops = {
        .owner = THIS_MODULE,
        .read = ch_read,
        .write = ch_write
};

static int __init ch_drv_init(void){
    printk(KERN_INFO "Initialization begin\n");
    if(alloc_chrdev_region(&first, 0 , 1, "ch_dev") < 0){
        printk(KERN_ERR "Failed allocating region\n");
        return -EFAULT;
    }
    if((cl = class_create(THIS_MODULE, "chardrv")) == NULL){
        printk(KERN_ERR "Failed creating class\n");
        unregister_chrdev_region(first, 1);
        return -EFAULT;
    }
    if(device_create(cl, NULL, first, NULL, "var4") == NULL){
        printk(KERN_ERR "Failed creating device\n");
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -EFAULT;
    }
    cdev_init(&c_dev, &ch_fops);
    if (cdev_add(&c_dev, first, 1) < 0)
    {
        printk(KERN_ERR "Failed adding device\n");
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -EFAULT;
    }
    if((in_proc = proc_create("var4", 0444, NULL, &proc_fops)) == NULL){
        printk(KERN_ERR "Failed creating proc entry");
        cdev_del(&c_dev);
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -EFAULT;
    }
    printk(KERN_INFO "Initialization succeeded\n");
    return 0;
}

static void __exit ch_drv_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "Module exited\n");
}

module_init(ch_drv_init);
module_exit(ch_drv_exit);
