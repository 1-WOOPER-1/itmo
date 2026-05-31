#include <stdio.h>
#include <stdlib.h>
#include "stddef.h"
#include "stdbool.h"

typedef struct Node {
    float value;
    struct Node *next;
} Node;

typedef struct RingQueue {
    struct Node *head;
    struct Node *tail;
    size_t size;
} RingQueue;

void init(RingQueue *q) {
    q->head = q->tail = NULL;                          // (6)
    q->size = 0;                                       // (3)
}
// Итоговая сложность init: 6 + 3 = 9

void enQueue(RingQueue *q, float value) {
    Node *node = malloc(sizeof(Node));                 // (3)
    if (!node) {                                       // (2)
        fprintf(stderr, "Ошибка: не удалось выделить память для узла кольцевой очереди.\n");
        exit(1);
    }
    node->value = value;                               // (3)

    if (q->size == 0) {                                // (3)
        node->next = node;                             // (3)
        q->head = node;                                // (3)
        q->tail = node;                                // (3)
    } else {
        node->next = q->head;                          // (5)
        q->tail->next = node;                          // (5)
        q->tail = node;                                // (3)
    }

    q->size++;                                         // (4)
}
// Итоговая сложность enQueue (худший случай - ветка else): 3 + 2 + 3 + 3 + 5 + 5 + 3 + 4 = 28

bool deQueue(RingQueue *q, float *out) {
    if (q->size == 0) return false;                    // (4)
    Node *oldHead = q->head;                           // (3)
    *out = oldHead->value;                             // (4)

    if (q->size == 1) {                                // (3)
        q->head = q->tail = NULL;                      // (6)
    } else {
        q->head = oldHead->next;                       // (5)
        q->tail->next = q->head;                       // (7)
    }

    free(oldHead);                                     // (1)
    q->size--;                                         // (4)
    return true;                                       // (1)
}
// Итоговая сложность deQueue (худший случай - ветка else): 4 + 3 + 4 + 3 + 5 + 7 + 1 + 4 + 1 = 32

void printQ(RingQueue *q) {
    if (q->size == 0) {                                // (3)
        printf("[ пусто ]\n");
        return;
    }

    Node *cur = q->head;                               // (3)
    printf("[ ");                                      // (1)
    for (size_t i = 0; i < q->size; i++) {             // (1) + (3)*(n+1) + (2)*(n) = 5n + 4
        printf("%f ", cur->value);                     // (3)
        if (i + 1 < q->size) printf("-> ");            // (5)
        cur = cur->next;                               // (3)
    } // Тело цикла: 3 + 5 + 3 = 11. Итого цикл: 5n + 4 + 11n = 16n + 4
    printf(" ] (size=%zu)\n", q->size);                // (3)
}
// Итоговая сложность printQ: 3 + 3 + 1 + (16n + 4) + 3 = 16n + 14

void insertionSort(RingQueue *q) {
    size_t n = q->size;                                // (3)
    if (n <= 1) return;                                // (2)

    Node *sortedHead = q->head;                        // (3)
    Node *sortedTail = q->head;                        // (3)
    Node *unsorted = q->head->next;                    // (5)

    sortedHead->next = sortedHead;                     // (3)
    
    for (size_t i = 1; i < n; i++) {                   // (1) + (1)*(n) + (2)*(n-1) = 3n - 1
        Node *cur = unsorted;                          // (1)
        unsorted = cur->next;                          // (3)

        if (cur->value <= sortedHead->value) {         // (5)
            cur->next = sortedHead;                    // (3)
            sortedTail->next = cur;                    // (3)
            sortedHead = cur;                          // (1)
        } else if (cur->value >= sortedTail->value) {  // (5)
            cur->next = sortedHead;                    // (3)
            sortedTail->next = cur;                    // (3)
            sortedTail = cur;                          // (1)
        } else {
            Node *prev = sortedHead;                   // (1)
            while (prev->next != sortedHead && prev->next->value <= cur->value) { // (11)
                prev = prev->next;                     // (3)
            } // Худший случай while (проход i элементов): 11*(i+1) + 3*i = 14i + 11
            cur->next = prev->next;                    // (5)
            prev->next = cur;                          // (3)
        } // Худший случай ветки else: 1 + (14i + 11) + 5 + 3 = 14i + 20
    } // Тело цикла (худший случай): 1 + 3 + 5 + 5 + (14i + 20) = 14i + 34
      // Сумма от i=1 до n-1 для (14i + 34) = 14*(n(n-1)/2) + 34(n-1) = 7n^2 + 27n - 34
      // Весь цикл: (3n - 1) + (7n^2 + 27n - 34) = 7n^2 + 30n - 35
    q->head = sortedHead;                              // (3)
    q->tail = sortedTail;                              // (3)
}
// Итоговая сложность insertionSort: 3 + 2 + 3 + 3 + 5 + 3 + (7n^2 + 30n - 35) + 3 + 3 = 7n^2 + 30n - 10

void bucketSort(RingQueue *q) {
    size_t n = q->size;                                // (3)
    if (n <= 1) return;                                // (2)

    float min = q->head->value, max = q->head->value;  // (10)
    Node *cur = q->head->next;                         // (5)
    for (size_t i = 1; i < n; i++) {                   // (1) + (1)*(n) + (2)*(n-1) = 3n - 1
        if (cur->value < min) min = cur->value;        // (6)
        if (cur->value > max) max = cur->value;        // (6)
        cur = cur->next;                               // (3)
    } // Цикл 1: (3n - 1) + 15(n-1) = 18n - 16
    
    if (min == max) return;                            // (2)

    int numBuckets = (int)n;                           // (2)
    RingQueue *buckets = malloc(numBuckets * sizeof(RingQueue)); // (4)
    if (!buckets)  {                                   // (2)
        fprintf(stderr, "Ошибка: не удалось выделить память для корзин\n"); exit(1);
    }
    for (int i = 0; i < numBuckets; i++)               // (1) + (n+1) + 2n = 3n + 2
        init(&buckets[i]);                             // (12)
    // Цикл 2: (3n + 2) + 12n = 15n + 2

    while (q->size != 0) {                             // (3) * (n+1) = 3n + 3
        float val;                                     // (0)
        deQueue(q, &val);                              // (34)
        int ind = (int)((val - min) * (numBuckets - 1) / (max - min)); // (7)
        enQueue(&buckets[ind], val);                   // (31)
    } // Цикл 3: 3n + 3 + n*(34 + 7 + 31) = 75n + 3

    for (int i = 0; i < numBuckets; i++) {             // (1) + (n+1) + 2n = 3n + 2
        if (buckets[i].size == 0) continue;            // (4)
        insertionSort(&buckets[i]);                    // (3) + insertionSort(7k^2 + 30k - 10)
        float val;                                     // (0)
        while (buckets[i].size != 0) {                 // (3) * (k+1)
            if (deQueue(&buckets[i], &val)) {          // (37)
                enQueue(q, val);                       // (29)
            }
        } // Внутренний while для k элементов: 3(k+1) + k*(37 + 29) = 69k + 3
    } // Худший случай цикла 4 (все n элементов в одной корзине, остальные n-1 пустые):
      // Пустые корзины: (n-1) * 4 = 4n - 4
      // Полная корзина: 3 (if) + (7n^2 + 30n - 7) + (69n + 3) = 7n^2 + 99n - 1
      // Весь цикл 4: (3n + 2) + (4n - 4) + (7n^2 + 99n - 1) = 7n^2 + 106n - 3
      
    free(buckets);                                     // (1)
}
// Итоговая сложность bucketSort: 
// 3 + 2 + 10 + 5 + (18n - 16) + 2 + 2 + 4 + 2 + (15n + 2) + (75n + 3) + (7n^2 + 106n - 3) + 1 = 7n^2 + 214n + 17

void readFromFile(char *filename, RingQueue *q) {
    FILE *file = fopen(filename, "r");                 // (2)
    if (!file) {                                       // (2)
        fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", filename);
        exit(1);
    }

    float tmp;                                         // (0)
    while (fscanf(file, "%f", &tmp) == 1) {            // (3) * (n+1) = 3n + 3
        enQueue(q, tmp);                               // (29)
    } // Цикл: 3n + 3 + 29n = 32n + 3
    
    fclose(file);                                      // (1)
}
// Итоговая сложность readFromFile: 2 + 2 + (32n + 3) + 1 = 32n + 8

int main(int argc, char **argv) {
    if (argc == 2) {                                   // (1)
        char *filename = argv[1];                      // (2)
        RingQueue q;                                   // (0)
        init(&q);                                      // (11)
        readFromFile(filename, &q);                    // (32n + 10)

        printf("Исходные данные из файла:\n");         // (1)
        printQ(&q);                                    // (16n + 16)
        bucketSort(&q);                                // (7n^2 + 214n + 19)
        printf("Отсортированные данные:\n");           // (1)
        printQ(&q);                                    // (16n + 16)

    } else {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
    }

    return 0;                                          // (1)
}
// Итоговая сложность main (и всей программы в худшем случае):
// 1 + 2 + 11 + (32n + 10) + 1 + (16n + 16) + (7n^2 + 214n + 19) + 1 + (16n + 16) + 1 = 7n^2 + 278n + 78