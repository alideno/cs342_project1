#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_COUNT 50

int main() {
    FILE *file = fopen("input.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    srand(time(NULL)); 

    for (int i = 0; i < NUM_COUNT; i++) {
        fprintf(file, "%d\n", rand() % 100); 
    }

    fclose(file);
    return 0;
}