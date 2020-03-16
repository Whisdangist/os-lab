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

_syscall2(int, sem_open, const char*, name, unsigned int, value);
_syscall1(int, sem_wait, int, sem);
_syscall1(int, sem_post, int, sem);
_syscall1(int, sem_unlink, const char*, name);
_syscall1(int, sem_get, int, sem);

#define M 1000
#define LINE_LENGTH 5
#define CONSUMER_COUNT 5
#define BUFFER_SIZE 10

int buffer;
int empty, full, mutex;
int write_pos, read_pos;

void produce_() {
	int i, j;
	char num[LINE_LENGTH];
	for (i = 0; i <= M; i++) {
		sprintf(num, "%d", i);
		for (j = 0; j < LINE_LENGTH; j++) {
			if (num[j] < '0' || num[j] > '9') {
				num[j] = ' ';
			}
		}
		sem_wait(full);
		sem_wait(mutex);

		lseek(buffer, sem_get(write_pos) % BUFFER_SIZE * LINE_LENGTH, SEEK_SET);
		if (write(buffer, num, LINE_LENGTH) == -1) {
			printf("Error writing buffer!");
			exit(-1);
		}
		printf("produce %d %d\n", i, times(NULL));
		fflush(stdout);

		sem_post(write_pos);
		sem_post(mutex);
		sem_post(empty);
	}
}

void consume_() {
	int i;
	char num[LINE_LENGTH];
	while (sem_get(read_pos) <= M) {
		sem_wait(empty);
		sem_wait(mutex);

		lseek(buffer, sem_get(read_pos) % BUFFER_SIZE * LINE_LENGTH, SEEK_SET);
		if (read(buffer, num, LINE_LENGTH) != LINE_LENGTH) {
			printf("Error reading buffer!");
			exit(-1);
		}
		for (i = 0; i < LINE_LENGTH; i++) {
			putchar(num[i]);
			fflush(stdout);
		}
		printf("consumer %d %d\n", getpid(), times(NULL));
		fflush(stdout);
		
		sem_post(read_pos);
		sem_post(mutex);
		sem_post(full);
	}
}

int main()
{
	int i, pid;

    freopen("output.txt", "w", stdout);

	buffer = open("buffer.txt", O_RDWR);
	if (buffer == -1) {
		printf("Failed to open the sharing files!\n");
		return -1;
	}

	empty = sem_open("empty", 0);
	full = sem_open("full", BUFFER_SIZE);
	mutex = sem_open("mutex", 1);
	write_pos = sem_open("write_pos", 0);
	read_pos = sem_open("read_pos", 0);
	if (empty == -1 || full == -1 || mutex == -1 || write_pos == -1 || read_pos == -1) {
		printf("sem_open() failed!\n");
		return -1;
	}

	for (i = 0; i < CONSUMER_COUNT; i++) {
		if (!(pid = fork())) {
			consume_();
			exit(0);
		}
		else {
			printf("Child process created! pid: %d\n", pid);
			fflush(stdout);
		}
	}
	produce_();

	wait(NULL);
	if (sem_unlink("empty") == -1) {
		printf("sem_unlink() falied: emtpy!");
	};
	if (sem_unlink("full") == -1) {
		printf("sem_unlink() falied: full!");
	};
	if (sem_unlink("mutex") == -1) {
		printf("sem_unlink() falied: mutex!");
	};
	if (sem_unlink("write_pos") == -1) {
		printf("sem_unlink() falied: write_pos!");
	};
	if (sem_unlink("read_pos") == -1) {
		printf("sem_unlink() falied: read_pos!");
	};
	close(buffer);
	return 0;
}
