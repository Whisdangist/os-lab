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

    freopen("c_output.txt", "w", stdout);

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

    printf("start consuming...\n");
    fflush(stdout);

	while (*read_pos <= M) {
		sem_wait(empty);
		sem_wait(mutex);

		printf("%5d", i = buffer[(*read_pos)%BUFFER_SIZE]);
		printf("consume %d %d\n", i, times(NULL));
		fflush(stdout);
        (*read_pos)++;

		sem_post(mutex);
		sem_post(full);
	}
    printf("consume finished!\n");
    fflush(stdout);

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
