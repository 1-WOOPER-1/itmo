#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct QNode {
    float value;
    struct QNode *next;
} QNode;

typedef struct Queue {
    QNode *head;
    QNode *tail;
    size_t size;
} Queue;

void qInit(Queue *q) {
    q->head = q->tail = NULL;
    q->size = 0;
}

void enqueue(Queue *q, float value) {
    QNode *node = malloc(sizeof(QNode));
    if (!node) {
        fprintf(stderr, "Ошибка: не удалось выделить память для узла очереди.\n");
        exit(1);
    }
    node->value = value;
    node->next  = NULL;
    if (q->size == 0) {
        q->head = q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;
}

bool dequeue(Queue *q, float *out) {
    if (q->size == 0) return false;
    QNode *old = q->head;
    *out = old->value;
    q->head = old->next;
    if (q->head == NULL) q->tail = NULL;
    free(old);
    q->size--;
    return true;
}

void printQ(const Queue *q) {
    if (q->size == 0) { printf("[ пусто ]\n"); return; }
    QNode *cur = q->head;
    printf("[ ");
    while (cur) {
        printf("%g", cur->value);
        if (cur->next) printf(" -> ");
        cur = cur->next;
    }
    printf(" ] (size=%zu)\n", q->size);
}

void insertionSortQueue(Queue *q)
{
    if (q->size <= 1)
        return;

    QNode *sorted = NULL;
    QNode *cur = q->head;

    while (cur != NULL) {
        QNode *next = cur->next;

        if (sorted == NULL || cur->value <= sorted->value) {
            cur->next = sorted;
            sorted = cur;
        }
        else {
            QNode *tmp = sorted;

            while (tmp->next != NULL &&
                   tmp->next->value < cur->value)
            {
                tmp = tmp->next;
            }

            cur->next = tmp->next;
            tmp->next = cur;
        }

        cur = next;
    }

    q->head = sorted;

    q->tail = sorted;
    while (q->tail != NULL &&
           q->tail->next != NULL)
    {
        q->tail = q->tail->next;
    }
}

void bucketSort(Queue *q) {
    size_t n = q->size;
    if (n <= 1) return;

    float min, max;
    QNode *cur = q->head;

    min = max = cur->value;
    cur = cur->next;

    while (cur) {
        if (cur->value < min) min = cur->value;
        if (cur->value > max) max = cur->value;
        cur = cur->next;
    }
    if (min == max) return;

    int nb = (int)n;
    Queue *buckets = malloc(nb * sizeof(Queue));
    if (!buckets) {
        fprintf(stderr, "Ошибка: не удалось выделить память для корзин.\n");
        exit(1);
    }
    for (int i = 0; i < nb; i++) qInit(&buckets[i]);

    float val;
    while (dequeue(q, &val)) {
        int idx = (int)((val - min) * (nb - 1) / (max - min));
        if (idx < 0)    idx = 0;
        if (idx >= nb)  idx = nb - 1;
        enqueue(&buckets[idx], val);
    }

    for (int i = 0; i < nb; i++) {
        if (buckets[i].size == 0)
            continue;

        insertionSortQueue(&buckets[i]);

        while (buckets[i].size > 0) {
            dequeue(&buckets[i], &val);
            enqueue(q, val);
        }
    }
    free(buckets);
}

static int *buildPrefixTable(const char *pattern, size_t m) {
    int *pi = malloc(m * sizeof(int));
    if (!pi) { fprintf(stderr, "Ошибка malloc KMP.\n"); exit(1); }
    pi[0] = 0;
    size_t k = 0;
    for (size_t i = 1; i < m; i++) {
        while (k > 0 && pattern[k] != pattern[i]) k = (size_t)pi[k - 1];
        if (pattern[k] == pattern[i]) k++;
        pi[i] = (int)k;
    }
    return pi;
}

int kmpSearch(const char *text, const char *pattern) {
    size_t n = strlen(text);
    size_t m = strlen(pattern);
    if (m == 0) return 0;
    if (m > n)  return -1;

    int *pi = buildPrefixTable(pattern, m);
    size_t q = 0;
    int result = -1;
    for (size_t i = 0; i < n; i++) {
        while (q > 0 && pattern[q] != text[i]) q = (size_t)pi[q - 1];
        if (pattern[q] == text[i]) q++;
        if (q == m) {
            result = (int)(i - m + 1);
            break;
        }
    }
    free(pi);
    return result;
}


typedef enum { RED, BLACK } Color;

typedef struct RBNode {
    float key;
    Color color;
    struct RBNode *left, *right, *parent;
} RBNode;

typedef struct RBTree {
    RBNode *root;
    RBNode *nil;
} RBTree;

static RBNode *rbNewNode(RBTree *t, float key) {
    RBNode *n = malloc(sizeof(RBNode));
    if (!n) { fprintf(stderr, "Ошибка malloc RBNode.\n"); exit(1); }
    n->key    = key;
    n->color  = RED;
    n->left   = n->right = n->parent = t->nil;
    return n;
}

void rbInit(RBTree *t) {
    t->nil = malloc(sizeof(RBNode));
    if (!t->nil) { fprintf(stderr, "Ошибка malloc nil.\n"); exit(1); }
    t->nil->color  = BLACK;
    t->nil->key    = 0;
    t->nil->left   = t->nil->right = t->nil->parent = t->nil;
    t->root        = t->nil;
}

static void leftRotate(RBTree *t, RBNode *x) {
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left != t->nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == t->nil)          t->root = y;
    else if (x == x->parent->left)    x->parent->left  = y;
    else                              x->parent->right = y;
    y->left    = x;
    x->parent  = y;
}

static void rightRotate(RBTree *t, RBNode *x) {
    RBNode *y = x->left;
    x->left = y->right;
    if (y->right != t->nil) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == t->nil)          t->root = y;
    else if (x == x->parent->right)   x->parent->right = y;
    else                              x->parent->left  = y;
    y->right   = x;
    x->parent  = y;
}

static void rbInsertFixup(RBTree *t, RBNode *z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode *y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color          = BLACK;
                y->color                  = BLACK;
                z->parent->parent->color  = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(t, z);
                }
                z->parent->color          = BLACK;
                z->parent->parent->color  = RED;
                rightRotate(t, z->parent->parent);
            }
        } else {
            RBNode *y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color          = BLACK;
                y->color                  = BLACK;
                z->parent->parent->color  = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(t, z);
                }
                z->parent->color          = BLACK;
                z->parent->parent->color  = RED;
                leftRotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = BLACK;
}

void rbInsert(RBTree *t, float key) {
    RBNode *z = rbNewNode(t, key);
    RBNode *y = t->nil;
    RBNode *x = t->root;
    while (x != t->nil) {
        y = x;
        if (z->key < x->key) x = x->left;
        else                  x = x->right;
    }
    z->parent = y;
    if (y == t->nil)          t->root   = z;
    else if (z->key < y->key) y->left   = z;
    else                      y->right  = z;
    rbInsertFixup(t, z);
}

static void rbPrintHelper(const RBTree *t,
                          const RBNode *n,
                          const char *prefix,
                          int isLeft)
{
    if (n == t->nil)
        return;

    char newPrefix[512];

    snprintf(newPrefix, sizeof(newPrefix), "%s%s",
             prefix, isLeft ? "│   " : "    ");

    rbPrintHelper(t, n->right, newPrefix, 0);

    printf("%s%s%s %g\n",
           prefix,
           isLeft ? "└── " : "┌── ",
           n->color == RED ? "[R]" : "[B]",
           n->key);

    snprintf(newPrefix, sizeof(newPrefix), "%s%s",
             prefix, isLeft ? "    " : "│   ");

    rbPrintHelper(t, n->left, newPrefix, 1);
}

void rbPrint(const RBTree *t) {
    if (t->root == t->nil) { printf("(дерево пусто)\n"); return; }
    rbPrintHelper(t, t->root->right, "    ", 0);
    printf("%s %g\n",
           t->root->color == RED ? "[R]" : "[B]",
           t->root->key);
    rbPrintHelper(t, t->root->left,  "    ", 1);
}

static void rbFreeNode(RBTree *t, RBNode *n) {
    if (n == t->nil) return;
    rbFreeNode(t, n->left);
    rbFreeNode(t, n->right);
    free(n);
}

void rbFree(RBTree *t) {
    rbFreeNode(t, t->root);
    free(t->nil);
    t->root = t->nil = NULL;
}

char *readFileToString(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", filename);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    if (!buf) { fclose(f); fprintf(stderr, "Ошибка malloc.\n"); exit(1); }
    size_t rb = fread(buf, 1, len, f);
    buf[rb] = '\0';
    fclose(f);
    return buf;
}

void parseBufferToQueue(const char *buf, Queue *q) {
    const char *ptr = buf;
    float tmp;
    int offset;
    while (sscanf(ptr, "%f%n", &tmp, &offset) == 1) {
        enqueue(q, tmp);
        ptr += offset;
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла> [подстрока]\n", argv[0]);
        return 1;
    }

    char *fileContent = readFileToString(argv[1]);

    Queue q;
    qInit(&q);
    parseBufferToQueue(fileContent, &q);

    printf("Исходный текст файла:\n%s\n", fileContent);
    printf("--- Исходные данные очереди ---\n");
    printQ(&q);

    char pattern[256];
    if (argc >= 3) {
        strncpy(pattern, argv[2], sizeof(pattern) - 1);
        pattern[sizeof(pattern) - 1] = '\0';
    } else {
        printf("Введите подстроку для поиска методом Кнута–Морриса–Пратта: ");
        if (fgets(pattern, sizeof(pattern), stdin))
            pattern[strcspn(pattern, "\n")] = '\0';
    }

    printf("Поиск подстроки \"%s\" (КМП)...\n", pattern);
    int pos = kmpSearch(fileContent, pattern);
    if (pos != -1) printf("Подстрока найдена на позиции: %d\n", pos);
    else           printf("Подстрока не найдена.\n");

    bucketSort(&q);
    printf("\n--- Отсортированные данные очереди ---\n");
    printQ(&q);

    RBTree tree;
    rbInit(&tree);

    float val;
    Queue tmp;
    qInit(&tmp);
    QNode *cur = q.head;
    while (cur) { enqueue(&tmp, cur->value); cur = cur->next; }

    while (dequeue(&tmp, &val)) rbInsert(&tree, val);

    printf("\n--- Красно-чёрное дерево (in-order, формат: значение [цвет]) ---\n");
    rbPrint(&tree);

    free(fileContent);
    while (dequeue(&q, &val)) {}
    rbFree(&tree);

    return 0;
}