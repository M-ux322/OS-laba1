#include <stdbool.h>
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>


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

int read_num() {
    int number = 0;
    bool start = false;
    char c;
    size_t len;
    char flag = 1;

    while((len = read(STDIN_FILENO, &c, 1)) > 0) {
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
            printf("s = %c", c);
            const char mess[] = "Error: invalid character in input\n";
            write(STDERR_FILENO, mess, sizeof(mess) - 1);
            _exit(EXIT_FAILURE);
        }
    }

    if (len == 0 && start) {
        return number * flag;
    }

    return -1;
}

void write_num(int n) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", n);
    write(STDOUT_FILENO, buffer, len);
    //write(STDOUT_FILENO, "\n", 1);
}

int main() {
    int number;

    while(1) {
        number = read_num();
        if (is_prime(number) || number < 0) {
            _exit(0);
        } else write_num(number);
    }

    return 0;
}