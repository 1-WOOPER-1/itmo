#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define MB_TO_ALLOC 1024*15

int main() {
    long page_size = sysconf(_SC_PAGESIZE);

    size_t total_bytes_to_alloc = (size_t)MB_TO_ALLOC * 1024 * 1024;
    char *mem_ptr = NULL;
    size_t allocated_so_far = 0;
    size_t offset = 0;

    mem_ptr = (char *)mmap(NULL, total_bytes_to_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    printf("Start allocating memory...\n");

    for (offset = 0; offset < total_bytes_to_alloc; offset += page_size) {
        mem_ptr[offset] = 0;
        allocated_so_far += page_size;
        if (allocated_so_far % (100 * 1024 * 1024) == 0) {
            printf("Allocated: %.2f MB\n", (double)allocated_so_far / (1024 * 1024));
        }
    }

    printf("Total allocated memory: %.2f MB\nPress Enter to free\n", (double)allocated_so_far / (1024 * 1024));
    getchar();

    return 0;
}
