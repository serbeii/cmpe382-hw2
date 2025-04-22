#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int is_prime(int num);
void* find_primes(void* args);

sem_t queue;
const char* sem_name = "/prime_queue";
int thread_count;

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Not enough arguments\n");
        printf("Not enough arguments\n");
        return (1);
    }
    char* directory = argv[2];
    thread_count = atoi(argv[3]);
    pthread_t threads[thread_count];
    sem_unlink(sem_name);
    queue = sem_open(sem_name, O_CREAT | O_EXCL, 0666, thread_count);
    if (queue == SEM_FAILED) {
        perror("sem_open");
        return 1;
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

    long curr_file = 0;
    while (curr_file < file_count - 1) {
        pthread_join(threads[curr_file % thread_count], NULL);
        sem_wait(&queue);
        pthread_create(&threads[curr_file % thread_count], NULL, &find_primes,
                       (void*)curr_file);
    }
}

int is_prime(int num) {
    for (int i = 2; i * i < num; i++) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

void* find_primes(void* args) {
    long fileid = (long)args;
    char* filename;
    int res = sprintf(filename, "file%ld.txt", fileid);
    long id = fileid % thread_count;
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) fprintf(stderr, "Error opening file\n");
    int prime_count = 0;
    int number;
    while (fscanf(file, "%d", &number) == 0) {
        if (is_prime(number)) {
            prime_count++;
        }
    }
    printf("Thread %ld found %d primes in %s", id, prime_count, filename);
    sem_post(&queue);
    return NULL;
}
