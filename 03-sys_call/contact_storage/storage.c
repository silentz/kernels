#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/user_data.h>


// ========== [Local functions] ==========

static int add_user_note(const char __user *, const char __user *, const char __user *, const char __user *);
static int get_user_note(const char *);
static int delete_user_note(const char *);
static int copy_string(char **, const char __user *);


// ========== [Structs] ===========

static struct User {
    char *name;
    char *surname;
    char *phone;
    char *email;
    char status;
};

typedef struct User User;

// ========== [Local variables] ==========

#define MAX_INPUT_SIZE 1023
#define USER_COUNT  1024

static User users[USER_COUNT];

// ========== [System calls implementation] ==========

SYSCALL_DEFINE3(get_user, const char __user *, surname, unsigned int, len, struct user_data __user *, output_data) {
    char *local_surname;
    User current_user;
    int result_index;
    struct user_data result;

    printk(KERN_INFO "get_user: called\n");

    if (len > MAX_INPUT_SIZE) {
        printk(KERN_ERR "get_user: surname wrong size\n");
        return -1;
    }

    local_surname = (char*) kmalloc(len + 1, GFP_KERNEL);
    local_surname[len] = '\0';
    if (copy_from_user(local_surname, surname, len) != 0) {
        printk(KERN_ERR "get_user: can't copy surname from userspace\n");
        kfree(local_surname);
        return -1;
    }

    result_index = get_user_note(local_surname);
    kfree(local_surname);

    if (result_index == -1) {
        printk(KERN_INFO "get_user: surname not found\n");
        kfree(local_surname);
        return -1;
    }

    current_user = users[result_index];
    result.name = (char*) kmalloc(strlen(current_user.name) + 1, GFP_USER);
    result.surname = (char*) kmalloc(strlen(current_user.surname) + 1, GFP_USER);
    result.phone = (char*) kmalloc(strlen(current_user.phone) + 1, GFP_USER);
    result.email = (char*) kmalloc(strlen(current_user.email) + 1, GFP_USER);

    if (result.name == NULL || result.surname == NULL || result.phone == NULL || result.email == NULL) {
        printk(KERN_ERR "get_user: not enough memory\n");
        kfree(result.name);
        kfree(result.surname);
        kfree(result.phone);
        kfree(result.email);
        return -1;
    }

    if (copy_to_user(result.name, current_user.name, strlen(current_user.name) + 1) != 0 ||
            copy_to_user(result.surname, current_user.surname, strlen(current_user.surname) + 1) != 0 ||
            copy_to_user(result.email, current_user.email, strlen(current_user.email) + 1) != 0 ||
            copy_to_user(result.phone, current_user.phone, strlen(current_user.phone) + 1) != 0) {
        printk(KERN_ERR "get_user: can't copy data to userspace\n");
        kfree(result.name);
        kfree(result.surname);
        kfree(result.phone);
        kfree(result.email);
        return -1;
    }

    if (copy_to_user(output_data, &result, sizeof(struct user_data)) != 0) {
        printk(KERN_ERR "get_user: can't copy data to userspace\n");
        kfree(result.name);
        kfree(result.surname);
        kfree(result.phone);
        kfree(result.email);
        return -1;
    }

    return 0;
}


SYSCALL_DEFINE1(add_user, struct user_data __user *, data) {
    struct user_data current_user;

    printk(KERN_INFO "add_user: called\n");

    if (copy_from_user(&current_user, data, sizeof(struct user_data)) != 0) {
        printk(KERN_ERR "add_user: can't copy from user\n");
        return -1;
    }

    if (add_user_note(current_user.name, current_user.surname, current_user.phone, current_user.email) != 0) {
        printk(KERN_ERR "add_user: can't add new user\n");
        return -1;
    }

    return 0;
}


SYSCALL_DEFINE2(del_user, const char __user *, surname, unsigned int, len) {
    printk(KERN_INFO, "del_user system call\n");
    return -EINVAL;
}


// ========== [Local function implementation] ==========


static int copy_string(char **result, const char __user *data) {
    size_t size;

    size = strnlen_user(data, MAX_INPUT_SIZE);
    *result = (char*) kmalloc(size + 1, GFP_KERNEL);
    if (copy_from_user(*result, data, size) != 0) {
        return -1;
    }
    (*result)[size] = '\0';

    return 0;
}


static int add_user_note(const char __user *name, const char __user *surname, const char __user *phone,
        const char __user *email) {
    int index;

    for (index = 0; index < USER_COUNT; ++index) {
        if (users[index].status == 'f') { // check if user cell is free
            if (copy_string(&users[index].name, name) != 0 ||
                    copy_string(&users[index].surname, surname) != 0 ||
                    copy_string(&users[index].phone, phone) != 0 ||
                    copy_string(&users[index].email, email) != 0) {
                kfree(users[index].name);
                kfree(users[index].surname);
                kfree(users[index].phone);
                kfree(users[index].email);
                return -1;
            }
            users[index].status = 't'; // set "taken" status
            return 0;
        }
    }

    return -1; // NO FREE SPACE
}


static int get_user_note(const char *surname) {
    int index;
    int result;

    result = -1; // NOT FOUND
    for (index = 0; index < USER_COUNT; ++index) {
        if ((users[index].status == 't') && (strcmp(surname, users[index].surname) == 0)) {
            result = index;
            break;
        }
    }

    return result;
}


static int delete_user_note(const char *surname) {
    int index;
    int result;

    result = -1;
    for (index = 0; index < USER_COUNT; ++index) {
        if ((users[index].status == 't') && (strcmp(surname, users[index].surname) == 0)) {
            result = index;
            break;
        }
    }

    if (result == -1) { // NOT FOUND
        return -1;
    }

    kfree(users[result].name);
    kfree(users[result].surname);
    kfree(users[result].phone);
    kfree(users[result].email);
    users[result].status = 'f';
    return 0;
}

