#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#define SNAME "shared_mem"

void find_k_largest_in_file(const char *filename, int k, int n, int index) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {return;}

    long *largest = (long *)malloc(k * sizeof(long));
    int count = 0;
    long num;

    for (int i = 0; i < k; i++){
        largest[i] = -1;
    }
    int shm_fd = shm_open(SNAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {return;}

    const int SIZE = sizeof(long) * k * n;

    long *shared_memory = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {return;}

    while (fscanf(file, "%ld", &num) != EOF) {
        if (count < k) {
            largest[count++] = num;
        } else {
            int min_index = 0;
            for (int i = 1; i < k; i++) {
                if (largest[i] < largest[min_index]) {
                    min_index = i;
                }
            }
            if (num > largest[min_index]) {
                largest[min_index] = num;
            }
        }
    }

    fclose(file);

    for (int i = 0; i < k; i++) {
        shared_memory[index * k + i] = largest[i];
    }
    shm_unlink(SNAME);
    free(largest);
}

// for qsort()
int compare_func(const void *a, const void *b) {
    long val_a = *(const long *)a;
    long val_b = *(const long *)b;
    return (val_b - val_a);
}


int main(int argc, char const *argv[]) {

    //validity checks and initial values

    if(argc < 9){
        perror("Missing arguments");
        return -1;
    }

    int k = atoi(argv[2]);
    if(k > 10000){
        perror("Too many output numbers");
        return -1;
    }
    int n = atoi(argv[4]);
    if(n > 20){
        perror("Too many processes");
        return -1;
    }
    char *input_file = (char *)argv[6];
    char *output_file = (char *)argv[8];

    FILE *file = fopen(input_file, "r");
    if (file == NULL) {return -1;}

    //shared memory
    int shm_fd = shm_open(SNAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {return -1;}

    const int SIZE = sizeof(long) * k * n;
    int ft = ftruncate(shm_fd, SIZE);
    if (ft == -1) {return -1;}

    long *shared_memory = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {return -1;}

    //create intermediate files

    FILE *inter_files[n];
    char inter_file_names[n][20];

    for (int i = 0; i < n; i++) {
        sprintf(inter_file_names[i], "inter%d.txt", i+1);
        inter_files[i] = fopen(inter_file_names[i], "w");
    }

    long num = 0;
    int index = 0;
    while (fscanf(file, "%ld", &num) != EOF) {
        int file_index = index % n;
        fprintf(inter_files[file_index], "%ld\n", num);
        index++;
    }

    fclose(file);

    //close all intermediate files
    for (int i = 0; i < n; i++) {
        fclose(inter_files[i]);
    }

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            exit(-1);
        } else if (pid == 0) { // child
            find_k_largest_in_file(inter_file_names[i], k, n, i);
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }
    
    //sort in descending order
    qsort(shared_memory, n * k, sizeof(long), compare_func);

    FILE *output = fopen(output_file, "w");
    for (int i = 0; i < k; i++) {
        fprintf(output, "%ld\n", shared_memory[i]);
    }
    fclose(output);

    
    // cleanup memory
    munmap(shared_memory, SIZE);
    shm_unlink(SNAME);

    // delete all intermediate files
    for (int i = 0; i < n; i++) {
        remove(inter_file_names[i]);
    }

    return 0;
}
