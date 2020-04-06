#ifndef INCLUDE_LINUX_USER_DATA
#define INCLUDE_LINUX_USER_DATA

#define USER_DATA_INPUT_MAX 1024

struct user_data {
    char name[USER_DATA_INPUT_MAX];
    char surname[USER_DATA_INPUT_MAX];
    char phone[USER_DATA_INPUT_MAX];
    char email[USER_DATA_INPUT_MAX];
};

#endif
