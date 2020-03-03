/*
 *  linux/kernel/who.c
 *
 *  (C) 2020 Xibei Liu
 */

#include <errno.h>
#include <string.h>
#include <asm/segment.h>

char name_k[24];

int sys_iam(char* name) {
    __asm__ ("int $0x3");
    int i = 0;
    char tmp[30];
    for (; i < 30; i++) {
        char ch = get_fs_byte(name+i);
        tmp[i] = ch;
        if (ch == '\0') {
            break;
        }
    }
    if (i < 24) {
        strcpy(name_k, tmp);
        return i;
    }
    else {
        return -EINVAL;
    }
}

int sys_whoami(const char* name, unsigned int size) {
    int len = strlen(name_k);
    int i = 0;
    if (len < size) {
        for (; i <= len; i++) {
            put_fs_byte(name_k[i], name+i);
        }
        return len;
    }
    else return -EINVAL;
}
