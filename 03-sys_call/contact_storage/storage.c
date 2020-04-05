#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/user_data.h>


// ========== [Local functions] ==========

static int add_user_note(const char *, const char *, const char *, const char *, const char *);
static int get_user_note(const char *);
static int delete_user_note(const char *);
static char* copy_string(const char *);


// ========== [Structs] ===========

static struct User {
    char *name;
    char *surname;
    char *phone;
    char *email;
    char *age;
    char status;
};

typedef struct User User;

// ========== [Local variables] ==========

#define USER_COUNT  1024

static User users[USER_COUNT];

// ========== [System calls implementation] ==========

SYSCALL_DEFINE3(get_user, const char __user *, surname, unsigned int, len, struct user_data __user *, output_data) {
    printk(KERN_INFO "get_user system call\n");
    return -EINVAL;
}


SYSCALL_DEFINE1(add_user, struct user_data __user *, data) {
    printk(KERN_INFO "add_user system call\n");
    return -EINVAL;
}


SYSCALL_DEFINE2(del_user, const char __user *, surname, unsigned int, len) {
    printk(KERN_INFO, "del_user system call\n");
    return -EINVAL;
}


// ========== [Local function implementation] ==========


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


static int add_user_note(const char *name, const char *surname, const char *age, const char *phone,
        const char *email) {
    int index;

    for (index = 0; index < USER_COUNT; ++index) {
        if (users[index].status == 'f') { // check if user cell is free
            users[index].name = copy_string(name);
            users[index].surname = copy_string(surname);
            users[index].age = copy_string(age);
            users[index].phone = copy_string(phone);
            users[index].email = copy_string(email);
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
    kfree(users[result].age);
    kfree(users[result].phone);
    kfree(users[result].email);
    users[result].status = 'f';
    return 0;
}

