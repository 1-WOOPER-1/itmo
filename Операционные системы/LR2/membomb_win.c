#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALLOC_STEP_BYTES (1024 * 1024 * 4)
#define MAX_BLOCKS 100000
void* allocated_memory_blocks[MAX_BLOCKS];
int num_allocated_blocks = 0;

int main() {
    long long total_allocated_bytes = 0;
    DWORD error_code;

    printf("Start allocating memory...\n");

    while (num_allocated_blocks < MAX_BLOCKS) {
        void* ptr = VirtualAlloc(
            NULL,               
            ALLOC_STEP_BYTES,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        );

        if (ptr == NULL) {
            error_code = GetLastError();
            fprintf(stderr, "Can't allocate memory. Error code: %lu\n", error_code);
            break;
        }

        memset(ptr, 0, ALLOC_STEP_BYTES);

        allocated_memory_blocks[num_allocated_blocks++] = ptr;
        total_allocated_bytes += ALLOC_STEP_BYTES;

        printf("Allocated %lld MB...\n", total_allocated_bytes / (1024 * 1024));
    }

    printf("Total allocated memory: %lld MB\nPress Enter to free\n", total_allocated_bytes / (1024 * 1024));

    getchar();

    printf("Free memory...\n");
    for (int i = 0; i < num_allocated_blocks; ++i) {
        VirtualFree(allocated_memory_blocks[i], 0, MEM_RELEASE);
    }

    return 0;
}