#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


MODULE_LICENSE("GPL");


#define DEVICE_NAME  "storage"
#define DEVICE_CLASS "storage"


// ========== [Device functions] ==========

static int     device_open(struct inode *, struct file *);
static int     device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

// ========== [Local functions] ==========

static char** split_request(const char *);
static int    compare(const char *, const char *);
static void   free_tokens(const char **, const int);


// ========== [Local variables] ==========

static struct file_operations fops = {
   .open    = device_open,
   .read    = device_read,
   .write   = device_write,
   .release = device_release,
};

static int major_number;
static int open_counter = 0;


// ========== [Init and exit functions] ==========

static int __init storage_init(void) {
    printk(KERN_INFO "Storage: module initialization started\n");
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Storage: failed to register device with %d\n", major_number);
        return major_number;
    }
    printk(KERN_INFO "Storage: initialization success!\n");
    return 0;
}


static void __exit storage_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Storage: module exit\n");
}


static int device_open(struct inode *i, struct file *f) {
    printk(KERN_INFO "Storage: device open\n");
    if (open_counter > 0) {
        return -EBUSY;
    }
    try_module_get(THIS_MODULE);
    ++open_counter;
    return 0;
}


static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset) {
    return 0;
}


static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset) {
    char *local_data; // local buffer to work with
    int index;        // for loop variable
    char **tokens;    // words of request
    int token_count;  // count of words

    if (len > 1024) { // forbid too large requests
        return -EINVAL;
    }

    local_data = (char*) kmalloc(len, GFP_KERNEL); // copy array from user space to kernel space
    for (index = 0; index < len; ++index) {
        get_user(local_data[index], buffer + index);
    }

    tokens = split_request(local_data, &token_count);
    if (token_count <= 0) {
        printk(KERN_WARNING "Storage: bad request\n");
        kfree(local_data);
        return -EINVAL;
    }

    if (compare(tokens[0], "get")) {

    } else if (compare(tokens[0], "add")) {

    } else if (compare(tokens[0], "del")) {

    } else {
        printk(KERN_WARNING "Storage: bad request\n");
        kfree(local_data);
    }

    free_tokens(tokens, token_count);
    kfree(local_data);
    return len;
}


static int device_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "Storage: device released\n");
    module_put(THIS_MODULE);
    --open_counter;
    return 0;
}


module_init(storage_init);
module_exit(storage_exit);


