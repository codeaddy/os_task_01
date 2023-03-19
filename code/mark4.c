#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    int fd1[2], fd2[2];
    pid_t pid1, pid2, pid3;
    char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile>\n", argv[0]);
        return 1;
    }

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe");
        return 1;
    }

    pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) { // первый процесс
        close(fd1[0]); // закрытие чтения
        int fd = open(argv[1], O_RDONLY); // открытие входного файла
        if (fd == -1) {
            perror("open");
            return 1;
        }
        int n;
        n = read(fd, buf, BUF_SIZE);
        printf("Read %d bytes from input file in 1st process\n", n);
        if (write(fd1[1], buf, n) != n) { // запись в канал
            perror("write");
            return 1;
        }
        close(fd);
        close(fd1[1]); // закрытие записи
        exit(0);
    }

    pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        return 1;
    }

    if (pid2 == 0) { // второй процесс
        close(fd1[1]); // закрытие записи от первого процесса
        close(fd2[0]); // закрытие чтения от третьего процесса
        int n;
        n = read(fd1[0], buf, BUF_SIZE);
        // обработка данных
        char *result = (char*) malloc((n * 3 + 1) * sizeof(char));
        int j = 0;
        {
            for (int i = 0; i < n; i++) {
                if (isalpha(buf[i]) && !strchr("AEIOUaeiou", buf[i])) {
                    sprintf(&result[j], "%d", (int)buf[i]);
                    j += count_digits((int)buf[i]);
                } else {
                    result[j++] = buf[i];
                }
            }
            result[j] = '\0';
        }
        if (write(fd2[1], result, j) != j) { // запись в канал
            perror("write");
            return 1;
        }
        printf("Wrote %d bytes to channel fd2[1] in 2nd process\n", j);

        free(result);
        close(fd1[0]); // закрытие чтения
        close(fd2[1]); // закрытие записи
        exit(0);
    }

    pid3 = fork();

    if (pid3 == -1) {
        perror("fork");
        return 1;
    }

    if (pid3 == 0) { // третий процесс
        close(fd2[1]); // закрытие записи от второго процесса
        int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666); // открытие выходного файла
        if (fd == -1) {
            perror("open");
            return 1;
        }
        int n;
        n = read(fd2[0], buf, BUF_SIZE);
        printf("Read %d bytes from channel fd2[0] in 3rd process\n", n);
        if (write(fd, buf, n) != n) { // запись в файл
            perror("write");
            return 1;
        }
        close(fd);
        close(fd2[0]); // закрытие чтения
        exit(0);
    }

    // родительский процесс
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}