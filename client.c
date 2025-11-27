#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    int number;
    bool finished;
    bool client_finished;
} shared_data;

bool is_prime(int num) {
    if (num < 2) return false;
    if (num == 2) return true;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}

void write_num(int n) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", n);
    write(STDOUT_FILENO, buffer, len);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char mess[] = "Usage: client <parent_pid>\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    int parent_pid = atoi(argv[1]);
    
    char shm_name[32];
    char sem_parent_name[32];
    char sem_child_name[32];
    
    snprintf(shm_name, sizeof(shm_name), "/lab_shm_%d", parent_pid);
    snprintf(sem_parent_name, sizeof(sem_parent_name), "/sem_parent_%d", parent_pid);
    snprintf(sem_child_name, sizeof(sem_child_name), "/sem_child_%d", parent_pid);
    
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        const char mess[] = "Error open shared memory\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        const char mess[] = "Error mmap shared memory\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    sem_t *sem_parent = sem_open(sem_parent_name, 0);
    sem_t *sem_child = sem_open(sem_child_name, 0);
    
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        const char mess[] = "Error open semaphore\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    while(1) {
        sem_wait(sem_child);
        
        if (data->finished) {
            sem_post(sem_parent);
            break;
        }
        
        if (is_prime(data->number) || data->number < 0) {
            data->client_finished = true;
            sem_post(sem_parent);
            break;
        }
        
        write_num(data->number);
        sem_post(sem_parent);
    }
    
    munmap(data, sizeof(shared_data));
    close(shm_fd);
    sem_close(sem_parent);
    sem_close(sem_child);
    
    return 0;
}