#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stddef.h"
#include "stdbool.h"

#define ALPHABET_SIZE 256

typedef struct Node {
    float value;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct Deque {
    struct Node *head;
    struct Node *tail;
    size_t size;
} Deque;

void init(Deque *d) {
    d->head = d->tail = NULL;
    d->size = 0;
}

void pushFront(Deque *d, float value) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Ошибка: не удалось выделить память для узла дека.\n");
        exit(1);
    }
    node->value = value;
    node->next = d->head;
    node->prev = NULL;

    if (d->size == 0) {
        d->head = node;
        d->tail = node;
    } else {
        d->head->prev = node;
        d->head = node;
    }
    d->size++;
}

void pushBack(Deque *d, float value) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Ошибка: не удалось выделить память для узла дека.\n");
        exit(1);
    }
    node->value = value;
    node->next = NULL;
    node->prev = d->tail;

    if (d->size == 0) {
        d->head = node;
        d->tail = node;
    } else {
        d->tail->next = node;
        d->tail = node;
    }
    d->size++;
}

bool popFront(Deque *d, float *out) {
    if (d->size == 0) return false;
    Node *oldHead = d->head;
    *out = oldHead->value;

    d->head = oldHead->next;
    if (d->head == NULL) {
        d->tail = NULL;
    } else {
        d->head->prev = NULL;
    }

    free(oldHead);
    d->size--;
    return true;
}

bool popBack(Deque *d, float *out) {
    if (d->size == 0) return false;
    Node *oldTail = d->tail;
    *out = oldTail->value;

    d->tail = oldTail->prev;
    if (d->tail == NULL) {
        d->head = NULL;
    } else {
        d->tail->next = NULL;
    }

    free(oldTail);
    d->size--;
    return true;
}

void printD(Deque *d) {
    if (d->size == 0) {
        printf("[ пусто ]\n");
        return;
    }

    Node *cur = d->head;
    printf("[ ");
    for (size_t i = 0; i < d->size; i++) {
        printf("%f ", cur->value);
        if (i + 1 < d->size) printf("<-> ");
        cur = cur->next;
    }
    printf(" ] (size=%zu)\n", d->size);
}

void insertionSort(Deque *d) {
    size_t n = d->size;
    if (n <= 1) return;

    Node *cur = d->head->next;
    while (cur != NULL) {
        Node *next = cur->next;
        float val = cur->value;
        Node *prev = cur->prev;

        while (prev != NULL && prev->value > val) {
            prev = prev->prev;
        }

        if (cur->prev != NULL) {
            cur->prev->next = cur->next;
        } else {
            d->head = cur->next;
        }

        if (cur->next != NULL) {
            cur->next->prev = cur->prev;
        } else {
            d->tail = cur->prev;
        }

        if (prev == NULL) {
            cur->next = d->head;
            cur->prev = NULL;
            if (d->head != NULL) {
                d->head->prev = cur;
            }
            d->head = cur;
        } else {
            cur->next = prev->next;
            cur->prev = prev;
            if (prev->next != NULL) {
                prev->next->prev = cur;
            } else {
                d->tail = cur;
            }
            prev->next = cur;
        }
        cur = next;
    }
}

void bucketSort(Deque *d) {
    size_t n = d->size;
    if (n <= 1) return;

    float min = d->head->value, max = d->head->value;
    Node *cur = d->head->next;
    for (size_t i = 1; i < n; i++) {
        if (cur->value < min) min = cur->value;
        if (cur->value > max) max = cur->value;
        cur = cur->next;
    }
    
    if (min == max) return;

    int numBuckets = (int)n;
    Deque *buckets = malloc(numBuckets * sizeof(Deque));
    if (!buckets)  {
        fprintf(stderr, "Ошибка: не удалось выделить память для корзин\n"); exit(1);
    }
    for (int i = 0; i < numBuckets; i++)
        init(&buckets[i]);

    while (d->size != 0) {
        float val;
        popFront(d, &val);
        int ind = (int)((val - min) * (numBuckets - 1) / (max - min));
        pushBack(&buckets[ind], val);
    }

    for (int i = 0; i < numBuckets; i++) {
        if (buckets[i].size == 0) continue;
        insertionSort(&buckets[i]);
        float val;
        while (buckets[i].size != 0) {
            if (popFront(&buckets[i], &val)) {
                pushBack(d, val);
            }
        }
    }
      
    free(buckets);
}

void buildShiftTable(const char *pattern, size_t m, int shift[ALPHABET_SIZE]) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        shift[i] = (int)m;
    }
    for (size_t i = 0; i < m - 1; i++) {
        shift[(unsigned char)pattern[i]] = (int)(m - 1 - i);
    }
}

int bmhSearch(const char *text, const char *pattern) {
    size_t n = strlen(text);
    size_t m = strlen(pattern);
    if (m == 0 || m > n) return -1;

    int shift[ALPHABET_SIZE];
    buildShiftTable(pattern, m, shift);

    size_t i = m - 1;
    while (i < n) {
        size_t k = 0;
        while (k < m && pattern[m - 1 - k] == text[i - k]) {
            k++;
        }
        if (k == m) {
            return (int)(i - m + 1);
        }
        i += shift[(unsigned char)text[i]];
    }
    return -1;
}

char* readFileToString(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "Ошибка: не удалось выделить память для буфера файла\n");
        exit(1);
    }

    size_t read_bytes = fread(buffer, 1, length, file);
    buffer[read_bytes] = '\0';
    fclose(file);
    return buffer;
}

void parseBufferToDeque(const char *buffer, Deque *d) {
    const char *ptr = buffer;
    float tmp;
    int offset;
    while (sscanf(ptr, "%f%n", &tmp, &offset) == 1) {
        pushBack(d, tmp);
        ptr += offset;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла> [подстрока]\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char *fileContent = readFileToString(filename);

    Deque d;
    init(&d);
    parseBufferToDeque(fileContent, &d);

    printf("Исходный текст файла:\n%s\n", fileContent);
    printf("--- Исходные данные дека ---\n");
    printD(&d);

    char pattern[256];
    if (argc >= 3) {
        strncpy(pattern, argv[2], sizeof(pattern) - 1);
        pattern[sizeof(pattern) - 1] = '\0';
    } else {
        printf("Введите подстроку для поиска методом Бойера-Мура-Хорспула: ");
        if (fgets(pattern, sizeof(pattern), stdin)) {
            pattern[strcspn(pattern, "\n")] = '\0';
        }
    }

    printf("Поиск подстроки \"%s\"...\n", pattern);
    int pos = bmhSearch(fileContent, pattern);
    if (pos != -1) {
        printf("Подстрока найдена на позиции: %d\n", pos);
    } else {
        printf("Подстрока не найдена.\n");
    }

    bucketSort(&d);
    printf("\n--- Отсортированные данные дека ---\n");
    printD(&d);

    free(fileContent);
    float trash;
    while (d.size > 0) {
        popFront(&d, &trash);
    }

    return 0;
}