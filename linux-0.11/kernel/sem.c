/*
 *  linux/kernel/sem.c
 *
 *  (C) 2020 Xibei Liu
 */

#include <errno.h>
#include <stddef.h>
#include <linux/sched.h>
#include <asm/system.h>
#include <asm/segment.h>

#define SEM_POOL_SIZE 10
#define MAX_NAME_LENGTH 11

typedef struct {
    char name[MAX_NAME_LENGTH];
    int value;
    int is_using;
    struct task_struct * wait_queue;
} sem_t;

sem_t sem_pool[SEM_POOL_SIZE];

void sem_init() {
    int i;
    for (i = 0; i < SEM_POOL_SIZE; i++) {
        sem_pool[i].is_using = 0;
        sem_pool[i].wait_queue = NULL;
    }
}

int sys_sem_open(const char *name, unsigned int value) {
    int i, j, k, f;
    for (i = 0, k = -1; i < SEM_POOL_SIZE; i++) {
        if (sem_pool[i].is_using) {
            f = 0;
            for (j = 0; j < MAX_NAME_LENGTH; j++) {
                if (sem_pool[i].name[j] != get_fs_byte(name+j)) break;
                if (!sem_pool[i].name[j]) {
                    f = 1;
                    break;
                }
            }
            if (f) {
                return i;
            }
        }
        else {
            k = i;
        }
    }

    if (k == -1) return -1;
    sem_t* ret = &sem_pool[k];
    ret->is_using = 1;
    ret->value = value;
    for (j = 0; j < MAX_NAME_LENGTH; j++) {
        ret->name[j] = get_fs_byte(name+j);
        if (!ret->name[j]) break;
    }
    return k;
}

int sys_sem_wait(int sem_i) {
    sem_t* sem = &sem_pool[sem_i];
    while (sem->value <= 0) {
        sleep_on(&sem->wait_queue);
    }
    cli();
    sem->value--;
    sti();
    return 0;
}

int sys_sem_post(int sem_i) {
    sem_t* sem = &sem_pool[sem_i];
    cli();
    sem->value++;
    wake_up(&sem->wait_queue);
    sti();
    return 0;
}

int sys_sem_unlink(const char* name) {
    int i, j, f;
    for (i = 0; i < SEM_POOL_SIZE; i++) {
        if (sem_pool[i].is_using) {
            f = 0;
            for (j = 0; j < MAX_NAME_LENGTH; j++) {
                if (sem_pool[i].name[j] != get_fs_byte(name+j)) break;
                if (!sem_pool[i].name[j]) {
                    f = 1;
                    break;
                }
            }
            if (f) {
                sem_pool[i].is_using = 0;
                return 0;
            }
        }
    }
    return -1;
}

int sys_sem_get(int sem_i) {
    sem_t* sem = &sem_pool[sem_i];
    return sem->value;
}
