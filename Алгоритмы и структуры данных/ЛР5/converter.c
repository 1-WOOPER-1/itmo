#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IDX(i,j,ld) ((i)*(ld) + (j))
#define MAX_JACOBI_ITERS 50000

/* Метод вращений Якоби для нахождения собственных значений и векторов матрицы Q */
static void jacobi_sym_eigen(double *A, double *V, double *w, int n) {
    for (int i = 0; i < n * n; ++i) V[i] = 0.0;
    for (int i = 0; i < n; ++i) V[IDX(i,i,n)] = 1.0;

    for (int iter = 0; iter < MAX_JACOBI_ITERS; ++iter) {
        int p = 0, q = 1;
        double max_off = fabs(A[IDX(0,1,n)]);
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double val = fabs(A[IDX(i,j,n)]);
                if (val > max_off) {
                    max_off = val;
                    p = i; q = j;
                }
            }
        }
        if (max_off < 1e-14) break;

        double app = A[IDX(p,p,n)];
        double aqq = A[IDX(q,q,n)];
        double apq = A[IDX(p,q,n)];
        if (fabs(apq) < 1e-30) continue;

        double tau = (aqq - app) / (2.0 * apq);
        double t   = (tau >= 0.0 ? 1.0 : -1.0) / (fabs(tau) + sqrt(1.0 + tau * tau));
        double c   = 1.0 / sqrt(1.0 + t * t);
        double s   = t * c;

        for (int k = 0; k < n; ++k) {
            if (k == p || k == q) continue;
            double aik    = A[IDX(k,p,n)];
            double akq    = A[IDX(k,q,n)];
            double new_kp = c * aik - s * akq;
            double new_kq = s * aik + c * akq;
            A[IDX(k,p,n)] = A[IDX(p,k,n)] = new_kp;
            A[IDX(k,q,n)] = A[IDX(q,k,n)] = new_kq;
        }

        double new_app = c*c*app - 2.0*s*c*apq + s*s*aqq;
        double new_aqq = s*s*app + 2.0*s*c*apq + c*c*aqq;
        A[IDX(p,p,n)] = new_app;
        A[IDX(q,q,n)] = new_aqq;
        A[IDX(p,q,n)] = A[IDX(q,p,n)] = 0.0;

        for (int k = 0; k < n; ++k) {
            double vip = V[IDX(k,p,n)];
            double viq = V[IDX(k,q,n)];
            V[IDX(k,p,n)] = c * vip - s * viq;
            V[IDX(k,q,n)] = s * vip + c * viq;
        }
    }

    for (int i = 0; i < n; ++i) w[i] = A[IDX(i,i,n)];
}

/* Функция для динамического чтения одномерного массива чисел из файла */
static double* read_double_array(const char *filepath, int *count) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Ошибка открытия файла: %s\n", filepath);
        return NULL;
    }
    int capacity = 128;
    int size = 0;
    double *arr = malloc(capacity * sizeof(double));
    if (!arr) {
        fclose(f);
        return NULL;
    }
    double val;
    while (fscanf(f, "%lf", &val) == 1) {
        if (size >= capacity) {
            capacity *= 2;
            double *temp = realloc(arr, capacity * sizeof(double));
            if (!temp) {
                free(arr);
                fclose(f);
                return NULL;
            }
            arr = temp;
        }
        arr[size++] = val;
    }
    fclose(f);
    *count = size;
    return arr;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <путь_к_подпапке_с_тестом>\n", argv[0]);
        printf("Пример: %s TEST/case1\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *folder = argv[1];
    char path_A[1024], path_B[1024], path_Q[1024], path_out[1024];

    sprintf(path_A, "%s/A.dat", folder);
    sprintf(path_B, "%s/B.dat", folder);
    sprintf(path_Q, "%s/Q.dat", folder);
    sprintf(path_out, "%s/formatted_input.txt", folder);

    int count_A = 0, count_B = 0, count_Q = 0;
    double *raw_A = read_double_array(path_A, &count_A);
    double *raw_B = read_double_array(path_B, &count_B);
    double *raw_Q = read_double_array(path_Q, &count_Q);

    if (!raw_A || !raw_B || !raw_Q) {
        fprintf(stderr, "Ошибка чтения входных матриц из папки %s\n", folder);
        free(raw_A); free(raw_B); free(raw_Q);
        return EXIT_FAILURE;
    }

    /* 1. Вычисляем размерность N по матрице A (размер N x N) */
    double sqrt_N = sqrt(count_A);
    int N = (int)round(sqrt_N);
    if (N * N != count_A) {
        fprintf(stderr, "Ошибка: матрица A в %s не является квадратной (элементов: %d)\n", path_A, count_A);
        free(raw_A); free(raw_B); free(raw_Q);
        return EXIT_FAILURE;
    }

    /* 2. Вычисляем размерность m по матрице B (размер N x m) */
    if (count_B % N != 0) {
        fprintf(stderr, "Ошибка: размерность B (%d) не согласуется с N (%d)\n", count_B, N);
        free(raw_A); free(raw_B); free(raw_Q);
        return EXIT_FAILURE;
    }
    int m = count_B / N;

    /* 3. Проверяем размерность Q (размер N x N) */
    if (count_Q != N * N) {
        fprintf(stderr, "Ошибка: размерность Q (%d) должна быть %d x %d\n", count_Q, N, N);
        free(raw_A); free(raw_B); free(raw_Q);
        return EXIT_FAILURE;
    }

    /* 4. Восстановление матрицы C из Q. Q = C^T * C.
     * Используем разложение: Q = U * L * U^T. Тогда C = sqrt(L) * U^T. */
    double *U = calloc(N * N, sizeof(double));
    double *L = calloc(N, sizeof(double));
    double *C = calloc(N * N, sizeof(double));

    if (!U || !L || !C) {
        fprintf(stderr, "Ошибка выделения памяти для разложения Q.\n");
        free(raw_A); free(raw_B); free(raw_Q);
        free(U); free(L); free(C);
        return EXIT_FAILURE;
    }

    // Метод Якоби модифицирует входную матрицу, передаем копию raw_Q
    jacobi_sym_eigen(raw_Q, U, L, N);

    for (int i = 0; i < N; ++i) {
        double val = L[i];
        if (val < 0.0) val = 0.0; // фильтрация отрицательного шума
        double sqrt_val = sqrt(val);
        for (int j = 0; j < N; ++j) {
            // C = sqrt(L) * U^T
            C[IDX(i, j, N)] = sqrt_val * U[IDX(j, i, N)];
        }
    }

    /* 5. Запрос параметров запуска у пользователя */
    int r = N;
    int steps = 100;
    double T = 1.0;

    printf("\nОбнаружена система размерности N=%d, m=%d\n", N, m);
    printf("Введите r (желаемый ранг редукции, по умолчанию %d): ", r);
    char buf[128];
    if (fgets(buf, sizeof(buf), stdin) && buf[0] != '\n') {
        sscanf(buf, "%d", &r);
    }
    printf("Введите steps (шаги интегрирования, по умолчанию %d): ", steps);
    if (fgets(buf, sizeof(buf), stdin) && buf[0] != '\n') {
        sscanf(buf, "%d", &steps);
    }
    printf("Введите T (конечное время интегрирования, по умолчанию %.2f): ", T);
    if (fgets(buf, sizeof(buf), stdin) && buf[0] != '\n') {
        sscanf(buf, "%lf", &T);
    }

    /* 6. Запись в выходной файл в формате лабораторной */
    FILE *out = fopen(path_out, "w");
    if (!out) {
        fprintf(stderr, "Не удалось создать файл вывода %s\n", path_out);
        free(raw_A); free(raw_B); free(raw_Q);
        free(U); free(L); free(C);
        return EXIT_FAILURE;
    }

    // Строка параметров: N m p r steps T (p устанавливаем равным N)
    fprintf(out, "%d %d %d %d %d %.6f\n", N, m, N, r, steps, T);

    // Запись матрицы A
    for (int i = 0; i < N * N; ++i) {
        fprintf(out, "%.15e%c", raw_A[i], (i + 1) % N == 0 ? '\n' : ' ');
    }
    fprintf(out, "\n");

    // Запись матрицы B
    for (int i = 0; i < N * m; ++i) {
        fprintf(out, "%.15e%c", raw_B[i], (i + 1) % m == 0 ? '\n' : ' ');
    }
    fprintf(out, "\n");

    // Запись матрицы C
    for (int i = 0; i < N * N; ++i) {
        fprintf(out, "%.15e%c", C[i], (i + 1) % N == 0 ? '\n' : ' ');
    }

    fclose(out);
    printf("\nФайл успешно сохранен: %s\n", path_out);

    free(raw_A); free(raw_B); free(raw_Q);
    free(U); free(L); free(C);
    return EXIT_SUCCESS;
}