#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    if (argc < 9) {
        return -1;
    }

    int k = atoi(argv[2]);
    int n = atoi(argv[4]);
    char *input_file = (char *)argv[6];
    char *output_file = (char *)argv[8];

    FILE *file = fopen(input_file, "r");
    if (file == NULL) {
        return -1;
    }

    FILE *inter_files[n];
    char inter_file_names[n][20];

    for (int i = 0; i < n; i++) {
        sprintf(inter_file_names[i], "inter%d.txt", i);
        inter_files[i] = fopen(inter_file_names[i], "w");
    }

    long num = 0;
    int index = 0;
    while (fscanf(file, "%d", &num) != EOF) {
        int file_index = index % n;
        fprintf(inter_files[file_index], "%ld\n", num);
        index++;
    }

    fclose(file);

    for (int i = 0; i < n; i++) {
        fclose(inter_files[i]);
    }

    for (int i = 0; i < n; i++) {
        remove(inter_file_names[i]);
    }

    return 0;
}
