#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BATCH_SIZE 50 

long get_nanoseconds(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}

int main() {
    size_t sizes[] = {
        1, 32, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 100000, 
        120000, 131072, 150000, 200000, 262144, 524288, 1048576, 2097152
    };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    int iterations = 100000; 
    void *ptrs[BATCH_SIZE];

    printf("Size_Bytes,Avg_Time_NS\n");

    for (int i = 0; i < num_sizes; i++) {
        size_t size = sizes[i];
        long total_time = 0;
        int successful_allocs = 0;

        for (int j = 0; j < iterations / BATCH_SIZE; j++) {
            for (int k = 0; k < BATCH_SIZE; k++) {
                struct timespec start, end;
                clock_gettime(CLOCK_MONOTONIC, &start);
                ptrs[k] = malloc(size);
                clock_gettime(CLOCK_MONOTONIC, &end);

                if (ptrs[k]) {
                    total_time += get_nanoseconds(start, end);
                    successful_allocs++;
                }
            }

            for (int k = 0; k < BATCH_SIZE; k++) {
                if (ptrs[k]) {
                    free(ptrs[k]);
                    ptrs[k] = NULL;
                }
            }
        }
        
        if (successful_allocs > 0) {
            printf("%zu,%ld\n", size, total_time / successful_allocs);
        }
    }

    return 0;
}