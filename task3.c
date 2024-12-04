#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

char a = 'a';
int *fds, childs, tofather = -1, fromfather = -1, son = 0;

void enter_sons() {
    son = 1;
    setbuf(stdin, NULL);
    scanf("%d", &childs);
    fds = malloc(4 * childs * sizeof(int));
    printf("%d ", getpid());
    fflush(stdout);
    for(int i = 0; i < childs; ++i) {
        pipe(fds + (i * 4));
        pipe(fds + (i * 4) + 2);
        int f;
        setbuf(stdin, NULL);
        scanf("%d", &f);
        pid_t pid = fork();
        if (pid > 0) { // father
            printf("%d ", pid);
            fflush(stdout);
            close(fds[i * 4 + 1]);
        }
        else { // son
            son = 0;
            if (tofather != -1) {
                close(tofather);
                close(fromfather);
            }
            tofather = fds[i * 4 + 1];
            fromfather = fds[i * 4 + 2];
            close(fds[i * 4]);
            close(fds[i * 4 + 3]);
            for(int j = 0; j < i; ++j) {
                close(fds[j * 4]);
                close(fds[j * 4 + 1]);
                close(fds[j * 4 + 2]);
                close(fds[j * 4 + 3]);
            }
            free(fds);
            read(fromfather, &a, 1);
            return;
        }
    }
    write(1, "\n", 1);
    for(int i = 0; i < childs; ++i) {
        write(fds[i * 4 + 3], &a, 1);
        read(fds[i * 4], &a, 1);
    }
    if (tofather != -1) {
        write(tofather, &a, 1);
    }
}

void counting() {
    int k;
    setbuf(stdin, NULL);
    scanf("%d", &k);
    int *set;
    set = malloc(k * sizeof(int));
    for(int i = 0; i < k; ++i) {
        int n;
        setbuf(stdin, NULL);
        scanf("%d", &n);
        set[i] = n;
    }
    while(1) {
        int sum = 0, done_prs = 0;
        for(int i = 0; i < childs; ++i) {
            write(fds[i * 4 + 3], &a, 1);
            int d;
            wait(NULL);
            if (read(fds[i * 4], &d, sizeof(int)) == 0) {
                done_prs++;
                continue;
            }
            sum += d;
        }
        int number = 0;
        if (k != 0) {
            int min = INT_MAX, ind = -1;
            for(int i = 0; i < k; ++i) {
                if (set[i] < min) {
                    min = set[i];
                    ind = i;
                }
            }
            for(int i = ind; i < k - 1; ++i) 
                set[i] = set[i + 1];
            set = realloc(set, (k - 1) * sizeof(int));
            number = min;
            k--;
        }  
        number += sum;
        if (tofather == -1) {
            if (k == 0 && childs == done_prs && number == 0) {
                for(int i = 0; i < childs; ++i) {
                    close(fds[i * 4]);
                    close(fds[i * 4 + 3]);
                    close(fds[i * 4 + 2]);
                    close(fds[i * 4 + 1]);
                }
                free(fds);
                printf("\nProcess %d finished\n", getpid());
                fflush(stdout);
                return;
            }
            char str[20];
            sprintf(str, "%d", number);
            int size = 0;
            do {
                number /= 10;
                size++;
            }
            while (number > 0);
            write(1, str, size);
            write(1, " ", 1);
            continue;
        }
        if (k == 0 && childs == done_prs && number == 0) {
            close(tofather);
            close(fromfather);
            for(int i = 0; i < childs; ++i) {
                close(fds[i * 4]);
                close(fds[i * 4 + 2]);
                close(fds[i * 4 + 3]);
                close(fds[i * 4 + 1]);
            }
            free(fds);
            printf("Process %d finished\n", getpid());
            fflush(stdout);
            exit(0);
        }
        write(tofather, &number, sizeof(int));
    }
}

int main() {
    int n;
    setbuf(stdin, NULL);
    scanf("%d", &n);
    if (n == 0)
        return 0;
    while(son == 0) {
        enter_sons();
    }
    if (tofather == -1) { // Root process
        counting();
    }
    else {
        read(fromfather, &a, 1);
        counting();
    }
    return 0;
}