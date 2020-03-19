#define __LIBRARY__
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/sched.h>

typedef struct {
    char name[10];
    int value;
} sem_t;
typedef int key_t;

_syscall2(int, sem_open, const char*, name, unsigned int, value);
_syscall1(int, sem_wait, int, sem);
_syscall1(int, sem_post, int, sem);
_syscall1(int, sem_unlink, const char*, name);
_syscall2(int, shmget, key_t, key, size_t, size);
_syscall1(void*, shmat, int, shmid);

#define M 1000
#define BUFFER_SIZE 10
#define SHM_KEY 1

int empty, full, mutex;
void* shmmem;
int *buffer;
int *write_pos, *read_pos;

int main() {
    int i, j;

    freopen("p_output.txt", "w", stdout);

	empty = sem_open("empty", 0);
	full = sem_open("full", BUFFER_SIZE);
	mutex = sem_open("mutex", 1);
	if (empty == -1 || full == -1 || mutex == -1) {
		printf("sem_open() failed!\n");
        fflush(stdout);
		return -1;
	}

    if ((i = shmget(SHM_KEY, (BUFFER_SIZE+2)*sizeof(int))) == -1 || (shmmem = shmat(i)) == NULL) {
        printf("shm operate failed!\n");
        fflush(stdout);
        return -1;
    }

    buffer = (int *) shmmem;
    write_pos = buffer + BUFFER_SIZE;
    read_pos = write_pos + 1;

    printf("start producing...\n");
    fflush(stdout);

	for (i = 0; i <= M; i++) {
		sem_wait(full);
		sem_wait(mutex);

		buffer[(*write_pos)%BUFFER_SIZE] = i;
		printf("produce %d %d\n", i, times(NULL));
		fflush(stdout);
        (*write_pos)++;

		sem_post(mutex);
		sem_post(empty);
	}
    printf("produce finished!\n");
    fflush(stdout);

    /* 有一点还没搞懂 为什么这里没有等待consumer的话 就会出现"trying to free free page"的报错 */
    while (*read_pos <= M);
    
	if (sem_unlink("empty") == -1) {
		printf("sem_unlink() falied: emtpy!");
	};
	if (sem_unlink("full") == -1) {
		printf("sem_unlink() falied: full!");
	};
	if (sem_unlink("mutex") == -1) {
		printf("sem_unlink() falied: mutex!");
	};
}
