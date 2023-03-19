#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

int count_digits(int num) {
    int count = 0;
    if (num == 0) {
        return 1; // число 0 имеет 1 цифру
    }
    while (num != 0) {
        count++;
        num /= 10;
    }
    return count;
}

#define BUF_SIZE 5000

int main(int argc, char *argv[]) {
    int fd1, fd2, fd3;
    pid_t pid1, pid2, pid3;
    char buf[BUF_SIZE];

    unlink("my_pipe1");
    unlink("my_pipe2");

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile>\n", argv[0]);
        return 1;
    }

    if (mkfifo("my_pipe1", 0666) == -1 || mkfifo("my_pipe2", 0666) == -1) {
        perror("pipe");
        return 1;
    }
    
    int fd = open(argv[1], O_RDONLY); // открытие входного файла
    if (fd == -1) {
        perror("open");
        return 1;
    }
    fd1 = open("my_pipe1", O_WRONLY);
    int n;
    n = read(fd, buf, BUF_SIZE);
    printf("Read %d bytes from input file in 1st process\n", n);
    if (write(fd1, buf, n) != n) { // запись в канал
        perror("write");
        return 1;
    }
    close(fd);
    close(fd1); // закрытие записи

    wait(NULL);
    
    fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666); // открытие выходного файла
    if (fd == -1) {
        perror("open");
        return 1;
    }
    fd2 = open("my_pipe2", O_RDONLY);
    n = read(fd2, buf, BUF_SIZE);
    printf("Read %d bytes from channel fd2[0] in 1st process\n", n);
    if (write(fd, buf, n) != n) { // запись в файл
        perror("write");
        return 1;
    }
    close(fd);
    close(fd2); // закрытие чтения

    // родительский процесс
    wait(NULL);

    unlink("my_pipe1");
    unlink("my_pipe2");
    return 0;
}