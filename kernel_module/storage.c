#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


MODULE_LICENSE("GPL");


// ========== [Device functions] ==========

static int     device_open(struct inode *, struct file *);
static int     device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

// ========== [Local functions] ==========

static char** split_request(char *, size_t, size_t *);
static int    is_space(char);


// ========== [Local variables] ==========

#define DEVICE_NAME  "storage"

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


// ========== [Device functions implementation] ==========

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
    char *local_data;   // local buffer to work with
    int index;          // for loop variable
    char **tokens;      // words of request
    size_t token_count; // count of words

    if (len > 1023) {   // forbid too large requests
        return -EINVAL;
    }

    local_data = (char*) kmalloc(1024, GFP_KERNEL); // copy array from user space to kernel space
    for (index = 0; index < len; ++index) {
        get_user(local_data[index], buffer + index);
    }
    local_data[len] = '\0';

    tokens = split_request(local_data, len, &token_count);
    if (token_count == 0 || tokens == NULL) {
        printk(KERN_WARNING "Storage: bad request\n");
        kfree(local_data);
        return -EINVAL;
    }

    if (strcmp(tokens[0], "get") == 0) {
        printk(KERN_INFO "Storage: get command\n");
    } else if (strcmp(tokens[0], "add") == 0) {
        printk(KERN_INFO "Storage: add command\n");
    } else if (strcmp(tokens[0], "del") == 0) {
        printk(KERN_INFO "Storage: del command\n");
    } else {
        printk(KERN_WARNING "Storage: bad request\n");
    }

    kfree(tokens);
    kfree(local_data);
    return len;
}


static int device_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "Storage: device released\n");
    module_put(THIS_MODULE);
    --open_counter;
    return 0;
}


// ========== [Local function implementation] ==========

static int is_space(char symbol) {
    if ((symbol == ' ') || (symbol == '\n') || (symbol == '\r') || (symbol == '\t')) {
        return 1;
    }
    return 0;
}


static char **split_request(char *data, size_t length, size_t *result_length) {
    size_t index;   // for loop variable
    char **tokens;  // tokens (result)

    *result_length = 0;
    if (length == 0) {
        return NULL;
    }

    for (index = 1; index < length; ++index) {
        if (is_space(data[index]) && !is_space(data[index - 1])) {
            (*result_length)++;
        }
    }
    if (!is_space(data[length - 1])) {
        (*result_length)++;
    }

    tokens = (char**) kmalloc(sizeof(char*) * (*result_length), GFP_KERNEL);
    index = 0;

    for (index = 0; index < (*result_length); ++index) {
        while (is_space(*data)) {
            *data = '\0';
            ++data;
        }
        if (*data == '\0') {
            break;
        }
        tokens[index] = data;
        while (!is_space(*data) && (*data != '\0')) {
            ++data;
        }
    }

    return tokens;
}


// ========== [Module set up] ==========

module_init(storage_init);
module_exit(storage_exit);

