/*
 *  linux/mm/memory.c
 *
 *  (C) 2020  Xibei Liu
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <linux/mm.h>
#include <linux/sched.h>

extern int sys_brk(unsigned long end_data_seg);

#define LOW_MEM 0x100000
#define KEY_NUM 10

typedef int key_t;

struct {
    key_t key;
    int validate;
    void* paddr;
} shm_pool[KEY_NUM];

int shm_init() {
    memset(shm_pool, 0, sizeof(shm_pool));
}

int sys_shmget(key_t key, size_t size) {
    int i, j;

    if (size > 4096) {
        errno = EINVAL;
        return -1;
    }
    for (i = 0, j = -1; i < KEY_NUM; i++) {
        if (!shm_pool[i].validate) {
            j = i;
            continue;
        }
        if (shm_pool[i].key == key) {
            return i;
        }
    }

    if (j == -1) {
        errno = ENOMEM;
        return -1;
    }
    shm_pool[j].key = key;
    shm_pool[j].validate = 1;
    shm_pool[j].paddr = get_free_page();
    if (!shm_pool[j].paddr) {
        errno = ENOMEM;
        return -1;
    }
    return j;
}

void* sys_shmat(int shmid) {
    unsigned long tmp;

    if (!shm_pool[shmid].validate) {
        errno = EINVAL;
        return NULL;
    }
    void* retval = sys_brk((current->brk + 2*PAGE_SIZE-1) & 0xfffff000) - PAGE_SIZE;
    put_page(shm_pool[shmid].paddr, retval + current->start_code);
    return retval;
}
