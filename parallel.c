#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>    


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int global_min = INT_MAX;
int global_max = INT_MIN;

typedef struct {
    int *array;
    int start;
    int end;
    int min;
    int max;
    int id;
} thread_data_t;

void write_number(int num) {
    char buffer[100];

    int i = 0;
    int sign = 1;
    
    if (num < 0) {
        sign = -1;
        num = -num;
    }
    
    do {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);
    
    if (sign == -1) {
        buffer[i++] = '-';
    }
    
    for (int j = 0; j < i/2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    
    buffer[i] = '\n';
    write(STDOUT_FILENO, buffer, i + 1);

}

int my_strlen(const char* str) {

    const char* str_copy = str;
    int len = 0;

    while(*str_copy != '\0') {
        len++;
        str_copy++;
    }
    return len;
}

unsigned long my_atoi(const char* number) {

    const char* str = number;
    unsigned long res = 0;

    while (*str != '\0') {
        char digit = *str;
        res = res * 10 + (digit - '0');
        str++;
    }

    return res;
}

int* my_random(int* array, unsigned long size) {

    if (!array || size == 0) return array;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int next = tv.tv_sec % 1000000;

    for (unsigned long i = 0; i < size; i++) {
        next = ((next * 214013 + 2531011) % 2147483648) % 1000000;
        array[i] = next;

    }

    return array;
}

void* find_min_max(void* arg) {

    thread_data_t* data = (thread_data_t*)arg;
    int local_min = INT_MAX;
    int local_max = INT_MIN;

    for (int i = data->start; i < data->end; i++) {
        if (data->array[i] > local_max) local_max = data->array[i];
        if (data->array[i] < local_min) local_min = data->array[i];
    }

    pthread_mutex_lock(&mutex);
    if (local_min < global_min) global_min = local_min;
    if (local_max > global_max) global_max = local_max;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

long long time_now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

int main(int argc, char* argv[]) {

    size_t size = my_atoi(argv[1]);

    int* array = (int*)sbrk(size * sizeof(int));
    if (array == (void *)-1) {
        char* str = "Memory allocation error!\n";
        write(STDERR_FILENO, str, my_strlen(str));
        return 1;
    }
    my_random(array, size);

    thread_data_t one_thread;
    one_thread.array = array;
    one_thread.start = 0;
    one_thread.end = size;

    global_min = INT_MAX;
    global_max = INT_MIN;
    one_thread.id = 0;

    long long start_time = time_now();
    find_min_max(&one_thread);
    long long consistent_time = time_now() - start_time;

    write(STDOUT_FILENO, "Sequential - Min: ", 18);
    write_number(global_min);
    write(STDOUT_FILENO, "Max: ", 5);
    write_number(global_max);
    write(STDOUT_FILENO, "Time: ", 6);
    write_number(consistent_time);
    write(STDOUT_FILENO, "\n", 1);

    int thread_counter = my_atoi(argv[2]);

    if (thread_counter > 1) {
        pthread_t threads[thread_counter];
        thread_data_t thread_data[thread_counter];

        global_min = INT_MAX;
        global_max = INT_MIN;

        size_t size_part = size / thread_counter;
        size_t ost_size = size % thread_counter;
        int cur_start = 0;

        for (int i = 0; i < thread_counter; i++) {
            thread_data[i].array = array;
            thread_data[i].start = cur_start;
            thread_data[i].end = cur_start + size_part + (i < ost_size ? 1 : 0);

            cur_start = thread_data[i].end;
        }

        start_time = time_now();

        for (int i = 0; i < thread_counter; i++) {
            pthread_create(&threads[i], NULL, find_min_max, &thread_data[i]);
        }

        for (int i = 0; i < thread_counter; i++) {
            pthread_join(threads[i], NULL);
        }

        long long parallel_time = time_now() - start_time;

        write(STDOUT_FILENO, "Parallel - Min: ", 16);
        write_number(global_min);
        write(STDOUT_FILENO, "Max: ", 5);
        write_number(global_max);
        write(STDOUT_FILENO, "Time: ", 6);
        write_number(parallel_time);
        write(STDOUT_FILENO, "\n", 1);

    }

    return 0;
}