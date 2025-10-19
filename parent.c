#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>  

int main() {

    char file_name[200];

    const char message[] = "Enter name of file: ";
    write(STDOUT_FILENO, message, sizeof(message));

    size_t len_name  = read(STDIN_FILENO, file_name, sizeof(file_name) - 1);

    if (len_name <= 0) {

        const char mess[] = "Error reading the file\n";
        write(STDOUT_FILENO, mess, sizeof(mess));
        exit(EXIT_FAILURE);
    }

    if (file_name[len_name - 1] == '\n') {
            file_name[len_name - 1] = '\0';

    } else file_name[len_name] = '\0';

    int file = open(file_name, O_RDONLY);


    if (file == -1) {
        const char mess[] = "Error opening file\n";
        write(STDOUT_FILENO, mess, sizeof(mess));
        exit(EXIT_FAILURE);
    }

    int child_to_parent[2];

    if (pipe(child_to_parent) == -1) {
        const char mess[] = "Error create pipe\n";
        write(STDOUT_FILENO, mess, sizeof(mess));
        exit(EXIT_FAILURE);
    }

    const pid_t child = fork();

    switch (child)
    {
    case -1:
    {   const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
        break;
    
    case 0:
    {
        close(child_to_parent[0]);

        dup2(child_to_parent[1], STDOUT_FILENO);
        close(child_to_parent[1]);

        dup2(file, STDIN_FILENO);
        close(file);

        execl("./child", "child", NULL);

        const char mess[] = "Error executing child\n";
        write(STDERR_FILENO, mess, sizeof(mess) - 1);
        exit(EXIT_FAILURE);
    }
    
    default:
    {
        close(file);
        close(child_to_parent[1]);

        char buf[100];
        size_t len;

        while((len = read(child_to_parent[0], buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, len);
        }

        close(child_to_parent[0]);
        wait(NULL);
    }

        break;
    }



    return 0;

}