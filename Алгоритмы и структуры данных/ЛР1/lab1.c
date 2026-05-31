#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

void printArray(const char *desc, float array[], int size) {
    if (desc) printf("%s", desc);                                // (2) (неявное != NULL + вызов)
    for (int i = 0; i < size; i++) {                             // (1) + (n+1) + (2n) = 3n + 2
        printf("%.3f ", array[i]);                               // (2) (индексация[] + вызов)
    }
    printf("\n");                                                // (1)
} 
// Итоговая сложность printArray: 2 + 3n + 2 + 2n + 1 = 5n + 5

int readFromFile(char *filename, float *array, int maxSize) {
    FILE *file = fopen(filename, "r");                           // (2) (вызов + присваивание)
    if (!file) {                                                 // (1) (логическое !)
        fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", filename); // (1)
        exit(1);                                                 // (1)
    }

    int n = 0;                                                   // (1)
    while (n < maxSize && fscanf(file, "%f", &array[n]) == 1) {  // (6) (<, &&, &,[], вызов, ==) * (n+1)
        n++;                                                     // (2) (инкремент) * n
    }
    
    fclose(file);                                                // (1)
    return n;                                                    // (1)
} 
// Итоговая сложность readFromFile (успешное чтение): 2 + 1 + 1 + 6(n+1) + 2n + 1 + 1 = 8n + 12

int countNums(char *filename) {
    FILE *file = fopen(filename, "r");                           // (2) (вызов + присваивание)
    if (!file) {                                                 // (1) (логическое !)
        fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", filename); // (1)
        exit(1);                                                 // (1)
    }
    int count = 0;                                               // (1)
    float temp;                                                  // (0)
    while (fscanf(file, "%f", &temp) == 1) {                     // (3) (&, вызов, ==) * (n+1)
        count++;                                                 // (2) (инкремент) * n
    }
    fclose(file);                                                // (1)
    return count;                                                // (1)
} 
// Итоговая сложность countNums (успешное чтение): 2 + 1 + 1 + 0 + 3(n+1) + 2n + 1 + 1 = 5n + 9

void insertionSort(float array[], int size) {
    for (int i = 1; i < size; i++) {                             // (1) + (n) + (2(n-1)) = 3n - 1
        int j = i - 1;                                           // (2) (- и =) * (n-1)
        while (j >= 0 && array[j] > array[j+1]) {                // (6) (>=, &&, [], [], +, >)
            float tmp = array[j+1];                              // (3) (+, [], =)
            array[j+1] = array[j];                               // (4) (+, [], [], =)
            array[j] = tmp;                                      // (2) ([], =)
            j--;                                                 // (2) (декремент)
        }
        // Сложность одного while в худшем случае: 6(i+1) + (3+4+2+2)*i = 17i + 6
    } 
    // Все while: Сумма от i=1 до n-1 (17i + 6) = 17*(n(n-1)/2) + 6(n-1) = 8.5n^2 - 2.5n - 6
} 
// Итоговая сложность insertionSort: (3n - 1) + 2(n-1) + (8.5n^2 - 2.5n - 6) = 8.5n^2 + 2.5n - 9

void bucketSort(float array[], int size) {
    if (size <= 1) return;                                       // (2) (<=, return)

    float min = array[0], max = array[0];                        // (4) ([], =,[], =)
    for (int i = 1; i < size; i++) {                             // (1) + (n) + (2(n-1)) = 3n - 1
        if (array[i] < min) min = array[i];                      // (4) ([], <,[], =)
        if (array[i] > max) max = array[i];                      // (4) ([], >,[], =)
    } // Цикл: (3n - 1) + 8(n-1) = 11n - 9

    if (min == max) return;                                      // (2) (==, return)

    int numBuckets = size;                                       // (1)
    
    float **buckets = (float**)malloc(numBuckets*sizeof(float*));// (5) (cast, malloc, *, sizeof, =)
    if (!buckets) {                                              // (1)
        fprintf(stderr, "Ошибка: не удалось выделить память для корзин\n"); // (1)
        exit(1);                                                 // (1)
    }
    for (int i = 0; i < numBuckets; i++) {                       // (1) + (n+1) + (2n) = 3n + 2
        buckets[i] = (float*)malloc(size*sizeof(float));         // (6) ([], cast, malloc, *, sizeof, =)
        if (!buckets[i]) {                                       // (2) ([], !)
            fprintf(stderr, "Ошибка: не удалось выделить память для корзины %d\n", i); // (1)
            exit(1);                                             // (1)
        }
    } // Цикл: (3n + 2) + 8n = 11n + 2
    
    int *bucketSizes = (int*)calloc(numBuckets, sizeof(int));    // (4) (cast, calloc, sizeof, =)
    if (!bucketSizes) {                                          // (1)
        fprintf(stderr, "Ошибка: не удалось выделить память для размеров корзин\n"); // (1)
        exit(1);                                                 // (1)
    }

    for (int i = 0; i < size; i++) {                             // (1) + (n+1) + (2n) = 3n + 2
        int bucketInd = (int)((array[i] - min)*(numBuckets - 1) / (max - min)); // (8) ([], -, -, *, -, /, cast, =)
        buckets[bucketInd][bucketSizes[bucketInd]] = array[i];   // (5) ([], [], [], [], =)
        bucketSizes[bucketInd]++;                                // (3) ([], ++)
    } // Цикл: 3n + 2 + 8n + 5n + 3n = 19n + 2

    int ind = 0;                                                 // (1)
    for (int i = 0; i < numBuckets; i++) {                       // (1) + (n+1) + (2n) = 3n + 2
        if (bucketSizes[i] > 0) {                                // (2) ([], >)
            insertionSort(buckets[i], bucketSizes[i]);           // (3) ([],[], вызов)
            for (int j = 0; j < bucketSizes[i]; j++) {           // (1) + 2(k+1) + (2k) = 4k + 3 (где k = bucketSizes[i])
                array[ind++] = buckets[i][j];                    // (6) (ind++,[], [],[], =)
            } // Внутренний цикл: (4k + 3) + 6k = 10k + 3
        }
        free(buckets[i]);                                        // (2) ([], вызов)
    } // Внешний цикл в худшем случае (по 1 элементу в корзине): (3n + 2) + 2n + 3n + 1n + 4n + 2n + 6n + 2n = 23n + 2 + O(n^2)
    
    free(buckets);                                               // (1)
    free(bucketSizes);                                           // (1)
} 
// Итоговая сложность bucketSort: 
// 2 + 4 + (11n - 9) + 2 + 1 + 5 + 1 + (11n + 2) + 4 + 1 + (19n + 2) + 1 + (23n + 2) + 1 + 1 = 64n + 20 + O(n^2)

int main(int argc, char **argv) {

    static struct option longopts[] = {                          // (1)
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"static", required_argument, 0, 's'},
        {"dynamic", no_argument, 0, 'd'},
        {0, 0, 0, 0},
    };

    int size = 0;                                                // (1)
    int isDynamic = -1;                                          // (1)

    int opt;                                                     // (0)
    while ((opt = getopt_long(argc, argv, "vhs:d", longopts, NULL)) != -1) { // (3) (вызов, =, !=) * (m+1)
        if (opt == 'v') {                                        // (1)
            printf("Карманная сортировка (Bucket Sort) v1.0\n"); // (1)
            printf("Информация об авторе:\nПахомов Владимир Юрьевич, гр. N3250, ИСУ 467036\n"); // (1)
            return 0;                                            // (1)
        } else if (opt == 'h') {                                 // (1)
            printf("Использование: ./lab1 [ОПЦИИ] <имя_файла>\n\n"); // (1)
            printf("Программа для сортировки чисел из файла методом карманной сортировки (Bucket Sort)\n\n"); // (1)
            printf("Опции:\n");                                  // (1)
            printf("    -s, --static=SIZE    Использовать статический массив (VLA) заданного размера SIZE\n"); // (1)
            printf("    -d, --dynamic        Использовать динамический массив (автоопределение размера файла)\n"); // (1)
            printf("    -h, --help           Показать эту справку и выйти\n"); // (1)
            printf("    -v, --version        Показать информацию о версии и выйти\n\n"); // (1)
            printf("Примеры:\n");                                // (1)
            printf("./lab1 -s=100 input.txt\n");                 // (1)
            printf("./lab1 -d input.txt\n");                     // (1)
            return 0;                                            // (1)
        }
        if (opt == 's') {                                        // (1)
            char *endptr;                                        // (0)
            if (optarg[0] == '=') optarg++;                      // (2) ([], ==) + (2) (++) = 4
            size = strtol(optarg, &endptr, 10);                  // (3) (&, вызов, =)
            if (optarg == endptr || *endptr != '\0' || size < 0) { // (6) (==, ||, *, !=, ||, <)
                fprintf(stderr, "Ошибка: некорректное значение опции -s: '%s'\n", optarg); // (1)
                return 1;                                        // (1)
            }
            isDynamic = 0;                                       // (1)
            // Худшая ветка (s): 1(v) + 1(h) + 1(s) + 4 + 3 + 6 + 1 = 17
        } else if (opt == 'd') {                                 // (1)
            isDynamic = 1;                                       // (1)
        } else {
            fprintf(stderr, "Использование: %s[-s=size | -d] <имя_файла>\n", argv[0]); // (2)
            return 1;                                            // (1)
        }
    } // Цикл while: 3(m+1) + 17m = 20m + 3

    if (optind >= argc) {                                        // (1)
        fprintf(stderr, "Ошибка: не указано имя файла\n");       // (1)
        return 1;                                                // (1)
    }

    char *filename = argv[optind];                               // (2) ([], =)

    if (isDynamic == 1) {                                        // (1)
        size = countNums(filename);                              // (2) (вызов, =)
        printf("Используется динамический массив\n");            // (1)

        float *dynamicArray = malloc(sizeof(float) * size);      // (4) (sizeof, *, вызов, =)
        if (!dynamicArray) {                                     // (1)
            fprintf(stderr, "Ошибка выделения памяти для динамического массива: %s", strerror(errno)); // (2)
            return 1;                                            // (1)
        }

        int n = readFromFile(filename, dynamicArray, size);      // (2) (вызов, =)

        printArray("\nИсходный массив из файла:\n", dynamicArray, n); // (1)
        bucketSort(dynamicArray, n);                                  // (1)
        printArray("\nОтсортированный массив:\n", dynamicArray, n);   // (1)

        free(dynamicArray);                                      // (1)
    // Итоговая сложность ветки if: 1 + 2 + 1 + 4 + 1 + 2 + 1 + 1 + 1 + 1 = 15
    } else if (isDynamic == 0) {                                 // (1)
        printf("Используется статический массив размера %d\n", size); // (1)

        float staticArray[size];                                 // (1)
        int n = readFromFile(filename, staticArray, size);       // (2)

        printArray("\nИсходный массив из файла:\n", staticArray, n); // (1)
        bucketSort(staticArray, n);                                  // (1)
        printArray("\nОтсортированный массив:\n", staticArray, n);   // (1)
    } else {
        fprintf(stderr, "Выберите режим: --static=SIZE или --dynamic\n"); // (1)
        return 1;                                                // (1)
    }
    
    return 0;                                                    // (1)
}
// Итоговая сложность main до вызова функций: 
// 3 (инициализация) + (20m + 3) (цикл) + 1 (if) + 2 (filename) + 15 (ветка if) + 1 (return) = 20m + 25

// ОБЩАЯ СЛОЖНОСТЬ ПРОГРАММЫ (Худший случай - динамический массив):
// T(m, n) = T_main + T_countNums + T_readFromFile + 2 * T_printArray + T_bucketSort
// T(m, n) = (20m + 25) + (5n + 9) + (8n + 12) + 2*(5n + 5) + (64n + 20 + O(n^2))
// T(m, n) = 20m + 87n + 76 + O(n^2)
// Асимптотическая сложность: O(m + n^2)