#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/user_data.h>


// ========== [Local functions] ==========

static int add_user_note(const struct user_data *);
static int get_user_note(const char *);
static int delete_user_note(const char *);
static int copy_string(char *, const char __user *);


// ========== [Structs] ===========

static struct User {
    char name[USER_DATA_INPUT_MAX];
    char surname[USER_DATA_INPUT_MAX];
    char phone[USER_DATA_INPUT_MAX];
    char email[USER_DATA_INPUT_MAX];
    char status;
};

typedef struct User User;

// ========== [Local variables] ==========

#define MAX_INPUT_SIZE 1023
#define USER_COUNT  1024

static User users[USER_COUNT];
static int initialized = 0;

// ========== [System calls implementation] ==========

SYSCALL_DEFINE3(get_user, const char __user *, surname, unsigned int, len, struct user_data __user *, output_data) {
    char *local_surname;
    int result_index;
    int index;

    if (initialized == 0) {
        for (index = 0; index < USER_COUNT; ++index) {
            users[index].status = 'f';
        }
        initialized = 1;
    }

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
        return -1;
    }

    if (copy_to_user(output_data->name, users[result_index].name, USER_DATA_INPUT_MAX) != 0 ||
            copy_to_user(output_data->surname, users[result_index].surname, USER_DATA_INPUT_MAX) != 0 ||
            copy_to_user(output_data->email, users[result_index].email, USER_DATA_INPUT_MAX) != 0 ||
            copy_to_user(output_data->phone, users[result_index].phone, USER_DATA_INPUT_MAX) != 0) {
        printk(KERN_ERR "get_user: can't copy data to userspace\n");
        return -1;
    }

    return 0;
}


SYSCALL_DEFINE1(add_user, struct user_data __user *, data) {
    struct user_data *current_user;
    int index;

    if (initialized == 0) {
        for (index = 0; index < USER_COUNT; ++index) {
            users[index].status = 'f';
        }
        initialized = 1;
    }

    printk(KERN_INFO "add_user: called\n");
    current_user = (struct user_data*) kmalloc(sizeof(struct user_data), GFP_KERNEL);

    if (copy_from_user(current_user, data, sizeof(struct user_data)) != 0) {
        printk(KERN_ERR "add_user: can't copy from user\n");
        kfree(current_user);
        return -1;
    }

    if (add_user_note(current_user) != 0) {
        printk(KERN_ERR "add_user: can't add new user\n");
        kfree(current_user);
        return -1;
    }

    kfree(current_user);
    return 0;
}


SYSCALL_DEFINE2(del_user, const char __user *, surname, unsigned int, len) {
    char *local_surname;
    int index;

    if (initialized == 0) {
        for (index = 0; index < USER_COUNT; ++index) {
            users[index].status = 'f';
        }
        initialized = 1;
    }

    printk(KERN_INFO "del_user: called\n");

    if (len > MAX_INPUT_SIZE) {
        printk(KERN_ERR "del_user: surname wrong size\n");
        return -1;
    }

    local_surname = (char*) kmalloc(len + 1, GFP_KERNEL);
    local_surname[len] = '\0';
    if (copy_from_user(local_surname, surname, len) != 0) {
        printk(KERN_ERR "del_user: can't copy surname from userspace\n");
        kfree(local_surname);
        return -1;
    }

    if (delete_user_note(local_surname) == -1) {
        printk(KERN_INFO "del_user: user not found\n");
        kfree(local_surname);
        return -1;
    }

    kfree(local_surname);
    return 0;
}


// ========== [Local function implementation] ==========


static int copy_string(char *result, const char *data) {
    size_t size;
    int index;

    size = strlen(data);
    if (size > MAX_INPUT_SIZE) {
        return -1;
    }

    for (index = 0; index < USER_DATA_INPUT_MAX; ++index) {
        result[index] = data[index];
    }
    result[size] = '\0';

    return 0;
}


static int add_user_note(const struct user_data *current_user) {
    int index;

    for (index = 0; index < USER_COUNT; ++index) {
        if (users[index].status == 'f') { // check if user cell is free
            if (copy_string(users[index].name, current_user->name) != 0 ||
                    copy_string(users[index].surname, current_user->surname) != 0 ||
                    copy_string(users[index].phone, current_user->phone) != 0 ||
                    copy_string(users[index].email, current_user->email) != 0) {
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

    for (index = 0; index < USER_COUNT; ++index) {
        if ((users[index].status == 't') && (strcmp(surname, users[index].surname) == 0)) {
            return index;
        }
    }

    return -1; // NOT FOUND
}


static int delete_user_note(const char *surname) {
    int index;

    for (index = 0; index < USER_COUNT; ++index) {
        if ((users[index].status == 't') && (strcmp(surname, users[index].surname) == 0)) {
            users[index].status = 'f';
            return index;
        }
    }

    return -1;
}

