#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int is_prime(int num);
void* find_primes(void* args);
int check_sem();

sem_t queue;
int thread_count;

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Not enough arguments\n");
        printf("Not enough arguments\n");
        return (1);
    }
    // system("bash setup.sh");
    char* directory = argv[1];
    thread_count = atoi(argv[2]);
    bool thread_use[thread_count];
    pthread_t threads[thread_count];
    sem_init(&queue, 0, thread_count);
    check_sem();
    for (int i = 0; i < thread_count; i++) {
        thread_use[i] = false;
    }

    int file_count = 0;
    struct dirent* entry;
    DIR* dirp;
    dirp = opendir(directory);

    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            file_count++;
        }
    }
    closedir(dirp);
    free(entry);

    for (long i = 1; i <= file_count; i++) {
        sem_wait(&queue);
        // check_sem();
        if (thread_use[i % thread_count]) {
            // return 0;
            pthread_join(threads[i % thread_count], NULL);
            thread_use[i % thread_count] = false;
        }
        if (check_sem() > thread_count) return 1;
        pthread_create(&threads[i % thread_count], NULL, &find_primes,
                       (void*)i);
        thread_use[i % thread_count] = true;
    }
    // wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        if (thread_use[i % thread_count]) {
            // return 0;
            pthread_join(threads[i % thread_count], NULL);
            thread_use[i % thread_count] = false;
        }
    }
}

int is_prime(int num) {
    if (num <= 1) return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

void* find_primes(void* args) {
    long fileid = (long)args;
    char* filename = malloc(sizeof(char) * 20);
    int res = sprintf(filename, "myDir/file%ld.txt", fileid);
    long id = fileid % thread_count + 1;
    FILE* file;
    // printf("%s\n", filename);
    file = fopen(filename, "r");
    if (file == NULL) fprintf(stderr, "Error opening file\n");
    int prime_count = 0;
    int number;
    while (fscanf(file, "%d", &number) >= 0) {
        // printf("current number: %d", number);
        if (is_prime(number)) {
            prime_count++;
        }
    }
    printf("Thread %ld found %d primes in %s\n", id, prime_count, filename);
    sem_post(&queue);
    return NULL;
}

int check_sem() {
    int semval;
    sem_getvalue(&queue, &semval);
    // printf("Current value of the semaphore: %d\n", semval);
    return semval;
}
