#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef struct {
    int number;
    bool finished;
    bool client_finished;
} shared_data;

int read_num_from_fd(int fd) {
    int number = 0;
    bool start = false;
    char c;
    size_t len;
    char flag = 1;

    while((len = read(fd, &c, 1)) > 0) {
        if (c == '-') {
            flag = -1;
        } else if (c >= '0' && c <= '9') {
            number = number * 10 + (c - '0');
            start = true;
        } else if (c == '\n' || c == ' ' || c == '\t' || c == '\r') {
            if (start) {
                return number * flag;
            }
        } else {
            const char mess[] = "Error: invalid character in input\n";
            write(STDERR_FILENO, mess, sizeof(mess) - 1);
            exit(EXIT_FAILURE);
        }
    }

    if (len == 0 && start) {
        return number * flag;
    }

    return -1;
}

int main() {
    char file_name[200];
    const char message[] = "Enter name of file: ";
    write(STDOUT_FILENO, message, sizeof(message) - 1);

    size_t len_name = read(STDIN_FILENO, file_name, sizeof(file_name) - 1);
    if (len_name <= 0) {
        const char mess[] = "Error reading the file\n";
        write(STDOUT_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }

    if (file_name[len_name - 1] == '\n') {
        file_name[len_name - 1] = '\0';
    } else {
        file_name[len_name] = '\0';
    }

    int file = open(file_name, O_RDONLY);
    if (file == -1) {
        const char mess[] = "Error opening file\n";
        write(STDOUT_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    
    char shm_name[32];
    char sem_parent_name[32];
    char sem_child_name[32];
    
    snprintf(shm_name, sizeof(shm_name), "/lab_shm_%d", pid);
    snprintf(sem_parent_name, sizeof(sem_parent_name), "/sem_parent_%d", pid);
    snprintf(sem_child_name, sizeof(sem_child_name), "/sem_child_%d", pid);
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        const char mess[] = "Error create shared memory\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        const char mess[] = "Error set size shared memory\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        const char mess[] = "Error mmap shared memory\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    data->number = 0;
    data->finished = false;
    data->client_finished = false;
    
    sem_t *sem_parent = sem_open(sem_parent_name, O_CREAT, 0666, 1);
    sem_t *sem_child = sem_open(sem_child_name, O_CREAT, 0666, 0);
    
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        const char mess[] = "Error create semaphore\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }

    const pid_t child = fork();

    switch (child) {
        case -1: {
            const char msg[] = "error: failed to spawn new process\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        break;
    
        case 0: {
            close(file);
            
            char pid_str[32];
            snprintf(pid_str, sizeof(pid_str), "%d", pid);
            
            execl("./client", "client", pid_str, NULL);

            const char mess[] = "Error executing client\n";
            write(STDERR_FILENO, mess, sizeof(mess) - 1);
            exit(EXIT_FAILURE);
        }
    
        default: {
            int number;
            
            while(1) {
                sem_wait(sem_parent);
                
                if (data->client_finished) {
                    break;
                }
                
                number = read_num_from_fd(file);
                data->number = number;
                
                if (number == -1) {
                    data->finished = true;
                    sem_post(sem_child);
                    break;
                }
                
                sem_post(sem_child);
            }
            
            close(file);
            wait(NULL);
            
            munmap(data, sizeof(shared_data));
            close(shm_fd);
            shm_unlink(shm_name);
            sem_close(sem_parent);
            sem_close(sem_child);
            sem_unlink(sem_parent_name);
            sem_unlink(sem_child_name);
        }
        break;
    }

    return 0;
}