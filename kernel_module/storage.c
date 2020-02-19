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
static void   set_message(const char*);
static void   add_user_note(const char *, const char *, const char *, const char *, const char *);
static void   get_user_note(const char *);
static void   delete_user_note(const char *);
static char*  copy_string(const char *);


// ========== [Structs] ===========

typedef struct User {
    char *name;
    char *surname;
    char *phone;
    char *email;
    char *age;
    char status;
} User;


// ========== [Local variables] ==========

#define DEVICE_NAME "storage"
#define MAX_INP     1023
#define USER_COUNT  1024
#define OUTPUT_SIZE 2048

static struct file_operations fops = {
   .open    = device_open,
   .read    = device_read,
   .write   = device_write,
   .release = device_release,
};

static int major_number;
static int open_counter = 0;
static User users[USER_COUNT];
static char output_data[OUTPUT_SIZE];
static char *output_ptr;


// ========== [Init and exit functions] ==========

static int __init storage_init(void) {
    int index;

    printk(KERN_INFO "Storage: module initialization started\n");
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Storage: failed to register device with %d\n", major_number);
        return major_number;
    }
    printk(KERN_INFO "Storage: initialization success\n");

    for (index = 0; index < USER_COUNT; ++index) {
        users[index].status = 'f'; // set free status for all cells
    }

    return 0;
}


static void __exit storage_exit(void) {
    int index;

    unregister_chrdev(major_number, DEVICE_NAME);

    for (index = 0; index < USER_COUNT; ++index) {
        if (users[index].status == 't') { // if taken -- clear data
            kfree(users[index].name);
            kfree(users[index].surname);
            kfree(users[index].age);
            kfree(users[index].phone);
            kfree(users[index].email);
            users[index].status = 'f';
        }
    }

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
    ssize_t read_bytes = 0;
    if (*output_ptr == '\0') {
        return 0;
    }
    while ((len > 0) && (*output_ptr != '\0')) {
        put_user(*output_ptr, buffer);
        ++buffer;
        ++output_ptr;
        ++read_bytes;
        --len;
    }
    return read_bytes;
}


static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset) {
    char *local_data;   // local buffer to work with
    int index;          // for loop variable
    char **tokens;      // words of request
    size_t token_count; // count of words

    if (len > MAX_INP) {   // forbid too large requests
        return -EINVAL;
    }

    local_data = (char*) kmalloc(MAX_INP + 1, GFP_KERNEL); // copy array from user space to kernel space
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

    for (index = 0; index < token_count; ++index) {
        printk(KERN_INFO "token[%d] = %s\n", index, tokens[index]);
    }

    if (strcmp(tokens[0], "get") == 0) {
        printk(KERN_INFO "Storage: get command\n");
        if (token_count != 2) {
            set_message("BAD REQUEST\n"
                    "format: get <username>\n");
            printk(KERN_WARNING "Storage: bad get command format\n");
        } else {
            get_user_note(tokens[1]);
        }
    } else if (strcmp(tokens[0], "add") == 0) {
        printk(KERN_INFO "Storage: add command\n");
        if (token_count != 6) {
            set_message("BAD REQUEST\n"
                    "format: add <name> <surname> <age> <phone> <email>\n");
            printk(KERN_WARNING "Storage: bad add command format\n");
        } else {
            add_user_note(tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        }
    } else if (strcmp(tokens[0], "delete") == 0) {
        printk(KERN_INFO "Storage: del command\n");
        if (token_count != 2) {
            set_message("BAD REQUEST\n"
                    "format: delete <surname>\n");
            printk(KERN_WARNING "Storage: bad delete command format\n");
        } else {
            delete_user_note(tokens[1]);
        }
    } else {
        set_message("BAD REQUEST\n"
                "Use one of the following commands: [add, get, delete]\n");
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


static char *copy_string(const char *data) {
    char *result;
    int index;
    size_t size;

    size = strlen(data);
    result = (char*) kmalloc(sizeof(char) * (size + 1), GFP_KERNEL);
    for (index = 0; index < size; ++index) {
        result[index] = data[index];
    }
    result[size] = '\0';

    return result;
}


static void set_message(const char *message) {
    sprintf(output_data, "%s", message);
    output_ptr = output_data;
}


static void add_user_note(const char *name,
        const char *surname, const char *age, const char *phone, const char *email) {
    int index;

    for (index = 0; index < USER_COUNT; ++index) {
        if (users[index].status == 'f') { // check if user cell is free
            users[index].name = copy_string(name);
            users[index].surname = copy_string(surname);
            users[index].age = copy_string(age);
            users[index].phone = copy_string(phone);
            users[index].email = copy_string(email);
            users[index].status = 't'; // set "taken" status
            set_message("OK\n");
            return;
        }
    }

    set_message("INTERNAL ERROR\n"
            "No free cells to store data\n");
}


static void get_user_note(const char *surname) {
    int index;
    int result;

    result = -1;
    for (index = 0; index < USER_COUNT; ++index) {
        if ((users[index].status == 't') && (strcmp(surname, users[index].surname) == 0)) {
            result = index;
            break;
        }
    }

    if (result == -1) {
        set_message("NOT FOUND\n");
        return;
    }

    sprintf(output_data, "FOUND\n"
            "Name: %s\n"
            "Surname: %s\n"
            "Age: %s\n"
            "Phone: %s\n"
            "Email: %s\n",
            users[result].name, users[result].surname,
            users[result].age, users[result].phone, users[result].email);
    output_ptr = output_data;
}


static void delete_user_note(const char *surname) {
    int index;
    int result;

    result = -1;
    for (index = 0; index < USER_COUNT; ++index) {
        if ((users[index].status == 't') && (strcmp(surname, users[index].surname) == 0)) {
            result = index;
            break;
        }
    }

    if (result == -1) {
        set_message("NOT FOUND\n");
        return;
    }

    kfree(users[result].name);
    kfree(users[result].surname);
    kfree(users[result].age);
    kfree(users[result].phone);
    kfree(users[result].email);
    users[result].status = 'f';
    set_message("OK\n");
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
    while (is_space(*data)) {
        *data = '\0';
        ++data;
    }

    return tokens;
}


// ========== [Module set up] ==========

module_init(storage_init);
module_exit(storage_exit);

