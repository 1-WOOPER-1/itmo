#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Макрос IDX переводит двумерный индекс (i, j) матрицы в одномерный индекс
 * массива при строчно-мажорном (row-major) хранении, как принято в языке C.
 * Параметр ld (leading dimension) — количество столбцов в строке матрицы.
 * Пример: для матрицы 3x4 элемент [1][2] => IDX(1,2,4) = 1*4+2 = 6. */
#define IDX(i,j,ld) ((i)*(ld) + (j))

/* Порог отсечения собственных значений матрицы Грамиана при построении POD-базиса.
 * Собственные значения λ_k <= EPS_EIG считаются численным шумом
 * и соответствующие им POD-моды в базис не включаются. */
#define EPS_EIG 1e-12

/* Максимальное число итераций метода вращений Якоби.
 * Используется при нахождении собственных значений и векторов
 * симметричной матрицы Грамиана G. При достижении предела
 * алгоритм завершается с текущей точностью. */
#define MAX_JACOBI_ITERS 50000

/* Максимальное число итераций метода Ньютона-Клейнмана.
 * Если за MAX_NEWTON_ITERS шагов алгоритм не достиг заданного
 * допуска tol, итерации прерываются с предупреждением. */
#define MAX_NEWTON_ITERS 50

/* Флаг управления выводом предупреждений в stderr.
 *   1 — выводить предупреждения (режим по умолчанию);
 *   0 — подавить все предупреждения (тихий режим, флаг -s/--silent).
 * Предупреждения информируют о росте невязки, несходимости метода
 * Ньютона, несимметричности или неположительности результата P. */
static int show_warnings = 0;



/* xcalloc — безопасное выделение обнулённого блока памяти размером nbytes байт.
 * В отличие от стандартного calloc, при нехватке памяти выводит сообщение
 * об ошибке и немедленно завершает программу, исключая разыменование NULL. */
static void *xcalloc(size_t nbytes) {
    void *p = calloc(1, nbytes);
    if (!p) {
        fprintf(stderr, "Ошибка: Недостаточно оперативной памяти.\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

/* fro_norm — норма Фробениуса матрицы (или вектора), переданной как
 * одномерный массив A длины n:
 *   ||A||_F = sqrt( sum_{i=0}^{n-1} A[i]^2 ).
 * Для матрицы M (k x l) вызывается как fro_norm(M, k*l). */
static double fro_norm(const double *A, int n) {
    double s = 0.0;
    for (int i = 0; i < n; ++i) s += A[i] * A[i];
    return sqrt(s);
}

/* symmetrize — приведение квадратной матрицы A (n x n) к симметричному виду
 * по формуле A := (A + A^T) / 2.
 * Применяется после каждого шага метода Ньютона-Клейнмана для компенсации
 * численной несимметрии, накапливаемой при решении уравнений Ляпунова. */
static void symmetrize(double *A, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double v = 0.5 * (A[IDX(i,j,n)] + A[IDX(j,i,n)]);
            A[IDX(i,j,n)] = v;
            A[IDX(j,i,n)] = v;
        }
    }
}

/* matmul — умножение матриц общего вида: C = A * B.
 * Размерности: A (n x m), B (m x k), результат C (n x k).
 * Внутри используется пропуск нулевых элементов A для ускорения
 * операции на блочно-разреженных матрицах (например, матрица A
 * системы 2-го порядка содержит блок нулей). */
static void matmul(const double *A, const double *B, double *C,
                   int n, int m, int k) {
    for (int i = 0; i < n * k; ++i) C[i] = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int t = 0; t < m; ++t) {
            double a = A[IDX(i,t,m)];
            if (a == 0.0) continue;
            for (int j = 0; j < k; ++j)
                C[IDX(i,j,k)] += a * B[IDX(t,j,k)];
        }
    }
}

/* at_mul_b — умножение транспонированной матрицы A^T на матрицу B: C = A^T * B.
 * Транспонирование выполняется неявно (без копирования), что экономит память.
 * Размерности: A хранится как (n x m), используется как (m x n);
 * B (n x k), результат C (m x k).
 * Применяется для вычисления:
 *   Al = Vl^T * AV (проецирование матрицы динамики),
 *   Bl = Vl^T * B  (проецирование матрицы входов),
 *   AP = A^T * P   (слагаемое невязки CARE). */
static void at_mul_b(const double *A, const double *B, double *C,
                     int n, int m, int k) {
    for (int i = 0; i < m * k; ++i) C[i] = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int t = 0; t < m; ++t) {
            double a = A[IDX(i,t,m)];
            if (a == 0.0) continue;
            for (int j = 0; j < k; ++j)
                C[IDX(t,j,k)] += a * B[IDX(i,j,k)];
        }
    }
}

/* matmul_right_transpose — умножение матрицы A на транспонированную B^T: C = A * B^T.
 * Транспонирование B выполняется неявно.
 * Размерности: A (n x m), B хранится как (k x m), используется как (m x k);
 * результат C (n x k).
 * Применяется для вычисления:
 *   S = B * B^T (матрица управляющей ковариации),
 *   P^(N) = tmp * Vl^T (восстановление полноразмерного решения). */
static void matmul_right_transpose(const double *A, const double *B,
                                   double *C, int n, int m, int k) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            double s = 0.0;
            for (int t = 0; t < m; ++t)
                s += A[IDX(i,t,m)] * B[IDX(j,t,m)];
            C[IDX(i,j,k)] = s;
        }
    }
}

/* compute_bbt — вычисление матрицы S = B * B^T для матрицы входов B (n x m).
 * Результат S (n x n) — симметричная положительно полуопределённая матрица.
 * Соответствует весовой матрице управления в уравнении Риккати:
 *   член -P * B * B^T * P отвечает за «стоимость» управляющего воздействия. */
static void compute_bbt(const double *B, int n, int m, double *S) {
    matmul_right_transpose(B, B, S, n, m, n);
}

/* compute_ctc — вычисление матрицы Q = C^T * C для матрицы выходов C (p x n).
 * Результат Q (n x n) — симметричная положительно полуопределённая матрица.
 * Соответствует штрафной матрице состояния в функционале LQR:
 *   J = integral( x^T Q x + u^T u ) dt,  Q = C^T C. */
static void compute_ctc(const double *C, int p, int n, double *Q) {
    at_mul_b(C, C, Q, p, n, n);
}



/* apply_AT — умножение вектора x на транспонированную матрицу системы: y = A^T * x.
 * Используется как правая часть сопряжённой системы dx/dt = A^T * x
 * при построении траекторий для POD-базиса. */
static void apply_AT(const double *A, const double *x,
                     double *y, int n) {
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j)
            s += A[IDX(j,i,n)] * x[j];
        y[i] = s;
    }
}

/* rk4_step_AT — один шаг численного интегрирования сопряжённой линейной системы
 *   dx/dt = A^T * x
 * классическим методом Рунге-Кутты 4-го порядка (RK4).
 *
 * Формула одного шага:
 *   k1 = A^T * x(t)
 *   k2 = A^T * (x + 0.5*dt*k1)
 *   k3 = A^T * (x + 0.5*dt*k2)
 *   k4 = A^T * (x + dt*k3)
 *   x(t+dt) = x(t) + (dt/6) * (k1 + 2*k2 + 2*k3 + k4)
 *
 * Рабочие буферы k1..k4 и xtmp передаются снаружи, чтобы не
 * выделять память заново на каждом из тысяч временных шагов. */
static void rk4_step_AT(const double *A, const double *x,
                        double dt, double *xnext, int n,
                        double *k1, double *k2,
                        double *k3, double *k4, double *xtmp) {
    /* Первый наклон: k1 = A^T * x(t) */
    apply_AT(A, x, k1, n);
    for (int i = 0; i < n; ++i) xtmp[i] = x[i] + 0.5 * dt * k1[i];

    /* Второй наклон: k2 = A^T * (x + dt/2 * k1) */
    apply_AT(A, xtmp, k2, n);
    for (int i = 0; i < n; ++i) xtmp[i] = x[i] + 0.5 * dt * k2[i];

    /* Третий наклон: k3 = A^T * (x + dt/2 * k2) */
    apply_AT(A, xtmp, k3, n);
    for (int i = 0; i < n; ++i) xtmp[i] = x[i] + dt * k3[i];

    /* Четвёртый наклон: k4 = A^T * (x + dt * k3) */
    apply_AT(A, xtmp, k4, n);

    /* Итоговое взвешенное обновление состояния */
    for (int i = 0; i < n; ++i)
        xnext[i] = x[i] + (dt / 6.0) *
                   (k1[i] + 2.0*k2[i] + 2.0*k3[i] + k4[i]);
}



/* jacobi_sym_eigen — нахождение полного спектра симметричной матрицы A (n x n)
 * итерационным методом вращений Якоби.
 *
 * На каждой итерации:
 *   1. Находится максимальный по модулю внедиагональный элемент A[p,q].
 *   2. Вычисляются параметры плоского вращения Якоби (c, s):
 *        tau = (A[q,q] - A[p,p]) / (2 * A[p,q])
 *        t   = sign(tau) / (|tau| + sqrt(1 + tau^2))
 *        c   = 1/sqrt(1+t^2),   s = t*c
 *   3. Применяется ортогональное преобразование подобия, обнуляющее A[p,q].
 *   4. Накапливается произведение поворотов в матрице собственных векторов V.
 *
 * Алгоритм сходится, когда max |A[i,j]| < 1e-14 для i != j.
 *
 * Выход:
 *   w[i] — собственные значения (считываются с диагонали после сходимости),
 *   V    — матрица собственных векторов (k-й столбец — вектор для λ_k). */
static void jacobi_sym_eigen(double *A, double *V, double *w, int n) {
    /* Инициализация V = I (единичная матрица — нейтральный элемент для поворотов) */
    for (int i = 0; i < n * n; ++i) V[i] = 0.0;
    for (int i = 0; i < n; ++i) V[IDX(i,i,n)] = 1.0;

    for (int iter = 0; iter < MAX_JACOBI_ITERS; ++iter) {
        /* Поиск максимального внедиагонального элемента */
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
        /* Критерий сходимости: все внедиагональные элементы пренебрежимо малы */
        if (max_off < 1e-14) break;

        double app = A[IDX(p,p,n)];
        double aqq = A[IDX(q,q,n)];
        double apq = A[IDX(p,q,n)];
        /* Пропуск вырожденного случая (теоретически невозможен при max_off > 0) */
        if (fabs(apq) < 1e-30) continue;

        /* Параметры вращения Якоби в плоскости (p, q) */
        double tau = (aqq - app) / (2.0 * apq);
        double t   = (tau >= 0.0 ? 1.0 : -1.0) /
                     (fabs(tau) + sqrt(1.0 + tau * tau));
        double c   = 1.0 / sqrt(1.0 + t * t);  /* косинус угла поворота */
        double s   = t * c;                     /* синус угла поворота  */

        /* Обновление строк и столбцов k != p, q в матрице A */
        for (int k = 0; k < n; ++k) {
            if (k == p || k == q) continue;
            double aik    = A[IDX(k,p,n)];
            double akq    = A[IDX(k,q,n)];
            double new_kp = c * aik - s * akq;
            double new_kq = s * aik + c * akq;
            A[IDX(k,p,n)] = A[IDX(p,k,n)] = new_kp;
            A[IDX(k,q,n)] = A[IDX(q,k,n)] = new_kq;
        }

        /* Обновление диагональных элементов p и q, обнуление A[p,q] */
        double new_app = c*c*app - 2.0*s*c*apq + s*s*aqq;
        double new_aqq = s*s*app + 2.0*s*c*apq + c*c*aqq;
        A[IDX(p,p,n)] = new_app;
        A[IDX(q,q,n)] = new_aqq;
        A[IDX(p,q,n)] = A[IDX(q,p,n)] = 0.0;

        /* Накопление поворота в матрице собственных векторов V */
        for (int k = 0; k < n; ++k) {
            double vip = V[IDX(k,p,n)];
            double viq = V[IDX(k,q,n)];
            V[IDX(k,p,n)] = c * vip - s * viq;
            V[IDX(k,q,n)] = s * vip + c * viq;
        }
    }

    /* После сходимости A диагональна: читаем собственные значения */
    for (int i = 0; i < n; ++i) w[i] = A[IDX(i,i,n)];
}

/* sort_eigs_desc — сортировка собственных пар (λ_k, v_k) по убыванию λ_k.
 * Используется selection sort: для каждой позиции i ищем наибольшее
 * оставшееся собственное значение и переставляем пары местами.
 * После сортировки:
 *   w[0] >= w[1] >= ... >= w[n-1],
 *   V[:,0] — направление наибольшей дисперсии (главная POD-мода). */
static void sort_eigs_desc(double *V, double *w, int n) {
    for (int i = 0; i < n - 1; ++i) {
        int best = i;
        for (int j = i + 1; j < n; ++j)
            if (w[j] > w[best]) best = j;
        if (best != i) {
            /* Перестановка собственных значений */
            double tw = w[i]; w[i] = w[best]; w[best] = tw;
            /* Синхронная перестановка столбцов матрицы V */
            for (int k = 0; k < n; ++k) {
                double tmp       = V[IDX(k,i,n)];
                V[IDX(k,i,n)]   = V[IDX(k,best,n)];
                V[IDX(k,best,n)] = tmp;
            }
        }
    }
}

/* orthonormalize_columns — ортонормирование первых r столбцов матрицы V (n x ld)
 * с помощью модифицированного процесса Грама-Шмидта.
 *
 * Для каждого k-го столбца:
 *   1. Вычитаются проекции на все предыдущие ортонормированные столбцы j < k.
 *   2. Вектор нормируется до единичной длины.
 *
 * Если норма вектора после ортогонализации меньше 1e-14, столбец обнуляется
 * (вырожденный вектор, линейно зависимый от предыдущих).
 *
 * Применяется после формирования POD-мод для компенсации численных ошибок,
 * накопленных при линейной комбинации большого числа снимков. */
static void orthonormalize_columns(double *V, int n, int r, int ld) {
    for (int k = 0; k < r; ++k) {
        /* Вычитание проекций на все предыдущие ортонормированные столбцы */
        for (int j = 0; j < k; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i)
                dot += V[IDX(i,j,ld)] * V[IDX(i,k,ld)];
            for (int i = 0; i < n; ++i)
                V[IDX(i,k,ld)] -= dot * V[IDX(i,j,ld)];
        }
        /* Нормировка текущего столбца */
        double norm = 0.0;
        for (int i = 0; i < n; ++i)
            norm += V[IDX(i,k,ld)] * V[IDX(i,k,ld)];
        norm = sqrt(norm);
        if (norm < 1e-14) {
            /* Вырожденный вектор — обнуляем */
            for (int i = 0; i < n; ++i) V[IDX(i,k,ld)] = 0.0;
        } else {
            for (int i = 0; i < n; ++i) V[IDX(i,k,ld)] /= norm;
        }
    }
}



/* build_pod_basis — построение ортонормированного проекционного POD-базиса
 * методом собственных ортогональных компонент (Proper Orthogonal Decomposition).
 *
 * Алгоритм (метод снимков Сирович):
 *   1. Для каждого выхода out = 0..p-1:
 *        - Начальное условие x(0) = C[out,:]^T (строка матрицы выходов).
 *        - Интегрируем сопряжённую систему dx/dt = A^T*x методом RK4
 *          на интервале [0, Tfinal] с шагом dt = Tfinal/steps.
 *        - Записываем (steps+1) снимков состояния в матрицу Y (N x S),
 *          где S = p*(steps+1) — общее число снимков.
 *   2. Вычисляем взвешенную матрицу Грамиана G (S x S):
 *        G[a,b] = dt * w_a * w_b * <Y[:,a], Y[:,b]>,
 *      где w = 0.5 для граничных снимков и 1.0 для внутренних (правило трапеций).
 *   3. Спектральный анализ G методом Якоби: G * U_k = λ_k * U_k.
 *      Сортировка пар по убыванию λ_k.
 *   4. Отбираем r_eff мод с λ_k > EPS_EIG.
 *      POD-мода: V[:,k] = Y * U_k / sqrt(λ_k).
 *   5. Ортонормируем столбцы V процедурой Грама-Шмидта.
 *
 * Возвращает r_eff — эффективный ранг базиса (число значимых мод). */
static int build_pod_basis(const double *A, const double *C,
                           int n, int p, int steps, double Tfinal,
                           double *V, int rmax) {
    int sn = steps + 1;    /* число снимков для одного начального условия */
    int S  = p * sn;       /* суммарное число снимков со всех выходов      */
    double dt = Tfinal / (double)steps;

    /* Выделение рабочих буферов */
    double *Y    = (double *)xcalloc((size_t)n * (size_t)S * sizeof(double));
    double *G    = (double *)xcalloc((size_t)S * (size_t)S * sizeof(double));
    double *U    = (double *)xcalloc((size_t)S * (size_t)S * sizeof(double));
    double *w    = (double *)xcalloc((size_t)S * sizeof(double));
    double *x    = (double *)xcalloc((size_t)n * sizeof(double));
    double *xnext= (double *)xcalloc((size_t)n * sizeof(double));
    double *k1   = (double *)xcalloc((size_t)n * sizeof(double));
    double *k2   = (double *)xcalloc((size_t)n * sizeof(double));
    double *k3   = (double *)xcalloc((size_t)n * sizeof(double));
    double *k4   = (double *)xcalloc((size_t)n * sizeof(double));
    double *xtmp = (double *)xcalloc((size_t)n * sizeof(double));

    /* Шаг 1: генерация снимков сопряжённой системы для каждого выхода */
    for (int out = 0; out < p; ++out) {
        /* Начальное условие: out-я строка матрицы C транспонированная в столбец */
        for (int i = 0; i < n; ++i) x[i] = C[IDX(out,i,n)];

        for (int s = 0; s < sn; ++s) {
            /* Запись текущего состояния в s-й столбец матрицы снимков Y */
            int col = out * sn + s;
            for (int i = 0; i < n; ++i) Y[IDX(i,col,S)] = x[i];

            /* Интегрирование одного шага (кроме последнего) */
            if (s + 1 < sn) {
                rk4_step_AT(A, x, dt, xnext, n, k1, k2, k3, k4, xtmp);
                memcpy(x, xnext, (size_t)n * sizeof(double));
            }
        }
    }

    /* Шаг 2: вычисление взвешенной матрицы Грамиана G.
     * Используется правило трапеций: граничные снимки имеют вес 0.5,
     * внутренние — 1.0. Только верхний треугольник вычисляется явно,
     * нижний заполняется симметрично. */
    for (int a = 0; a < S; ++a) {
        int ta = a % sn;
        double wa = (ta == 0 || ta == steps) ? 0.5 : 1.0;
        for (int b = a; b < S; ++b) {
            int tb = b % sn;
            double wb = (tb == 0 || tb == steps) ? 0.5 : 1.0;
            double dot = 0.0;
            for (int i = 0; i < n; ++i)
                dot += Y[IDX(i,a,S)] * Y[IDX(i,b,S)];
            dot *= dt * wa * wb;
            G[IDX(a,b,S)] = G[IDX(b,a,S)] = dot;
        }
    }

    /* Шаг 3: спектральный анализ Грамиана G */
    jacobi_sym_eigen(G, U, w, S);
    sort_eigs_desc(U, w, S);

    /* Шаги 4-5: формирование POD-мод и их ортонормирование */
    int r_eff = 0;
    for (int k = 0; k < rmax && k < S; ++k) {
        /* Отсечение мод с малыми собственными значениями (численный шум) */
        if (w[k] <= EPS_EIG) break;
        double invs = 1.0 / sqrt(w[k]);
        /* V[:,k] = Y * U[:,k] / sqrt(λ_k) */
        for (int i = 0; i < n; ++i) {
            double s = 0.0;
            for (int j = 0; j < S; ++j)
                s += Y[IDX(i,j,S)] * U[IDX(j,k,S)];
            V[IDX(i,k,rmax)] = s * invs;
        }
        ++r_eff;
    }

    /* Дополнительная ортонормализация для устранения накопленных ошибок */
    orthonormalize_columns(V, n, r_eff, rmax);

    free(Y); free(G); free(U); free(w);
    free(x); free(xnext);
    free(k1); free(k2); free(k3); free(k4); free(xtmp);
    return r_eff;
}



/* gauss_solve — решение системы линейных уравнений A*x = b методом Гаусса
 * с частичным выбором ведущего элемента по столбцу (partial pivoting).
 *
 * Частичный выбор ведущего элемента:
 *   На каждом шаге k ищем строку piv >= k с наибольшим |A[piv,k]|
 *   и переставляем строки k и piv. Это предотвращает деление на малые числа
 *   и повышает численную устойчивость алгоритма.
 *
 * ВАЖНО: матрица A и вектор b разрушаются в процессе вычислений.
 * Результат записывается в массив x.
 * При обнаружении вырожденной матрицы (pivot < 1e-18) программа завершается.
 *
 * Вычислительная сложность: O(n^3) операций. */
static void gauss_solve(double *A, double *b, double *x, int n) {
    /* Прямой ход: приведение к верхнетреугольному виду */
    for (int k = 0; k < n; ++k) {
        /* Поиск ведущего (максимального) элемента в k-м столбце */
        int piv = k;
        double maxv = fabs(A[IDX(k,k,n)]);
        for (int i = k + 1; i < n; ++i) {
            double v = fabs(A[IDX(i,k,n)]);
            if (v > maxv) { maxv = v; piv = i; }
        }
        if (maxv < 1e-18) {
            fprintf(stderr, "Ошибка: Сингулярная матрица в методе Гаусса.\n");
            exit(EXIT_FAILURE);
        }
        /* Перестановка строк k и piv */
        if (piv != k) {
            for (int j = k; j < n; ++j) {
                double tmp      = A[IDX(k,j,n)];
                A[IDX(k,j,n)]   = A[IDX(piv,j,n)];
                A[IDX(piv,j,n)] = tmp;
            }
            double tb = b[k]; b[k] = b[piv]; b[piv] = tb;
        }
        /* Исключение неизвестных под диагональю */
        double akk = A[IDX(k,k,n)];
        for (int i = k + 1; i < n; ++i) {
            double f = A[IDX(i,k,n)] / akk;
            A[IDX(i,k,n)] = 0.0;
            for (int j = k + 1; j < n; ++j)
                A[IDX(i,j,n)] -= f * A[IDX(k,j,n)];
            b[i] -= f * b[k];
        }
    }
    /* Обратный ход: нахождение вектора решения x */
    for (int i = n - 1; i >= 0; --i) {
        double s = b[i];
        for (int j = i + 1; j < n; ++j) s -= A[IDX(i,j,n)] * x[j];
        x[i] = s / A[IDX(i,i,n)];
    }
}

/* solve_lyapunov — решение непрерывного уравнения Ляпунова:
 *   A^T * X + X * A = -Q
 * методом Кронекеровой векторизации.
 *
 * Ключевая идея: применяем операцию vec() к обеим частям уравнения.
 * Используя тождество vec(AXB) = (B^T ⊗ A) * vec(X), получаем:
 *   vec(A^T X I) + vec(I X A) = -vec(Q)
 *   (I ⊗ A^T + A^T ⊗ I) * vec(X) = -vec(Q)
 *   K * vec(X) = -vec(Q),
 * где K — матрица Кронекеровой суммы размера n^2 x n^2.
 *
 * Заполнение K в строчно-мажорной адресации (строка row = i*n+j):
 *   K[row, k*n+j] += A[k,i]   — вклад от слагаемого I ⊗ A^T
 *   K[row, i*n+k] += A[k,j]   — вклад от слагаемого A^T ⊗ I
 *
 * СЛАУ решается методом Гаусса. Решение vec(X) переформируется в X.
 *
 * Вычислительная сложность: O(n^6) из-за метода Гаусса для n^2 x n^2 матрицы.
 * Поэтому метод применяется только к редуцированным системам малой размерности. */
static void solve_lyapunov(const double *A, const double *Q,
                           double *X, int n) {
    int N = n * n;   /* размер векторизованной системы */
    double *K   = (double *)xcalloc((size_t)N * (size_t)N * sizeof(double));
    double *rhs = (double *)xcalloc((size_t)N * sizeof(double));

    /* Формирование матрицы Кронекеровой суммы K и правой части rhs = -vec(Q) */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int row = i * n + j;     /* строка, соответствующая элементу X[i,j] */
            rhs[row] = -Q[IDX(i,j,n)];
            for (int k = 0; k < n; ++k) {
                /* Вклад от I ⊗ A^T: связывает X[i,j] с X[k,j] */
                K[IDX(row, k*n+j, N)] += A[IDX(k,i,n)];
                /* Вклад от A^T ⊗ I: связывает X[i,j] с X[i,k] */
                K[IDX(row, i*n+k, N)] += A[IDX(k,j,n)];
            }
        }
    }

    double *sol = (double *)xcalloc((size_t)N * sizeof(double));
    gauss_solve(K, rhs, sol, N);

    /* Переформирование vec(X) -> X (совпадает с прямым копированием
     * т.к. оба хранятся в строчно-мажорном формате) */
    for (int i = 0; i < N; ++i) X[i] = sol[i];

    free(K); free(rhs); free(sol);
}



/* care_residual_matrix — вычисление матрицы невязки непрерывного уравнения Риккати:
 *   F = A^T * P + P * A - P * B * B^T * P + C^T * C.
 * Используется для оценки точности найденного решения P. При точном решении F = 0. */
static void care_residual_matrix(const double *A, const double *B,
                                 const double *C, const double *P,
                                 int n, int m, int p, double *F) {
    double *Q   = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *S   = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *AP  = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *PA  = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *PS  = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *PSP = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));

    compute_ctc(C, p, n, Q);           /* Q   = C^T * C       */
    compute_bbt(B, n, m, S);           /* S   = B * B^T       */
    at_mul_b(A, P, AP, n, n, n);       /* AP  = A^T * P       */
    matmul(P, A, PA, n, n, n);         /* PA  = P * A         */
    matmul(P, S, PS, n, n, n);         /* PS  = P * S         */
    matmul(PS, P, PSP, n, n, n);       /* PSP = P * S * P     */

    /* F = A^T*P + P*A - P*B*B^T*P + C^T*C */
    for (int i = 0; i < n * n; ++i)
        F[i] = AP[i] + PA[i] - PSP[i] + Q[i];

    free(Q); free(S); free(AP); free(PA); free(PS); free(PSP);
}

/* care_residual_norm — норма Фробениуса невязки уравнения Риккати:
 *   ||A^T P + P A - P B B^T P + C^T C||_F.
 * Используется как критерий качества приближения. */
static double care_residual_norm(const double *A, const double *B,
                                 const double *C, const double *P,
                                 int n, int m, int p) {
    double *F = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    care_residual_matrix(A, B, C, P, n, m, p, F);
    double nrm = fro_norm(F, n * n);
    free(F);
    return nrm;
}

/* check_symmetric — проверка симметричности матрицы P (n x n) с допуском tol.
 * Стабилизирующее решение CARE должно быть симметричным: P = P^T.
 * Возвращает 1 если |P[i,j] - P[j,i]| <= tol для всех i != j, иначе 0. */
static int check_symmetric(const double *P, int n, double tol) {
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (fabs(P[IDX(i,j,n)] - P[IDX(j,i,n)]) > tol)
                return 0;
    return 1;
}

/* check_diag_nonneg — необходимое (но не достаточное) условие P >= 0:
 * все диагональные элементы P[i,i] должны быть неотрицательны.
 * Возвращает 1 если P[i,i] >= -1e-10 для всех i, иначе 0.
 * Отрицательные диагональные элементы гарантируют, что P не является
 * положительно полуопределённой. */
static int check_diag_nonneg(const double *P, int n) {
    for (int i = 0; i < n; ++i)
        if (P[IDX(i,i,n)] < -1e-10)
            return 0;
    return 1;
}

/* check_residual_monotone — предупреждение при нарушении монотонного убывания
 * нормы невязки ||R(P^(N))||_F по мере роста l.
 * Рост невязки более чем на 1% сигнализирует о возможной численной
 * нестабильности POD-базиса или неудачном начальном приближении. */
static void check_residual_monotone(double prev_res,
                                    double res, int l) {
    if (prev_res > 0.0 && res > prev_res * 1.01) {
        if (show_warnings) {
            fprintf(stderr,
                "ПРЕДУПРЕЖДЕНИЕ: l=%d — невязка уравнения Риккати выросла "
                "(%.6e -> %.6e). Возможна численная нестабильность.\n",
                l, prev_res, res);
        }
    }
}



/* solve_care_newton — решение непрерывного алгебраического уравнения Риккати (CARE):
 *   A^T P + P A - P B B^T P + C^T C = 0
 * методом Ньютона-Клейнмана (итерация по уравнениям Ляпунова).
 *
 * Итерационная схема:
 *   Дано P_k. Вычисляем:
 *     F(P_k) = A^T P_k + P_k A - P_k B B^T P_k + C^T C  (невязка)
 *     A_cl   = A - B B^T P_k                              (замкнутая матрица)
 *   Находим поправку Delta_k из уравнения Ляпунова:
 *     A_cl^T Delta_k + Delta_k A_cl = -F(P_k)
 *   Обновляем: P_{k+1} = P_k + Delta_k.
 *
 * Инициализация (выбор P_0):
 *   Если K_init == NULL (устойчивая A):
 *     P_0 — решение уравнения Ляпунова: A^T P_0 + P_0 A = -C^T C.
 *   Если K_init != NULL (неустойчивая A, колебательная система):
 *     Используем стабилизирующую обратную связь K_init.
 *     A_cl_init = A - B * K_init  (замкнутая матрица — устойчива по построению)
 *     Q_eff = C^T C + K_init^T K_init  (расширенный штраф)
 *     P_0 — решение: A_cl_init^T P_0 + P_0 A_cl_init = -Q_eff.
 *
 * Критерий остановки: ||F(P_k)||_F < tol или достигнуто maxit итераций.
 * Возвращает число выполненных итераций (< maxit при сходимости). */
static int solve_care_newton(const double *A, const double *B,
                             const double *C, const double *K_init, double *P,
                             int n, int m, int p,
                             int maxit, double tol) {
    double *Q     = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *S     = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *F     = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *Acl   = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *Delta = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *AP    = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *PA    = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *PS    = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    double *PSP   = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));

    compute_ctc(C, p, n, Q);   /* Q = C^T C */
    compute_bbt(B, n, m, S);   /* S = B B^T */

    if (K_init) {
        /* --- Стабилизирующая инициализация для неустойчивых систем ---
         *
         * Стандартное уравнение A^T P0 + P0 A = -Q не имеет положительного
         * решения если A неустойчива (есть собственные значения с Re > 0).
         * Вместо этого используем замкнутую устойчивую матрицу A_cl_init.
         *
         * Расширяем штраф: Q_eff = Q + K_init^T * K_init,
         * чтобы учесть стоимость начального управления K_init. */
        double *Q_eff = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
        memcpy(Q_eff, Q, (size_t)n*(size_t)n*sizeof(double));
        /* Q_eff += K_init^T * K_init */
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                double sum = 0.0;
                for (int k = 0; k < m; ++k)
                    sum += K_init[IDX(k,i,n)] * K_init[IDX(k,j,n)];
                Q_eff[IDX(i,j,n)] += sum;
            }
        }
        /* A_cl_init = A - B * K_init (устойчивая замкнутая матрица) */
        double *A_cl = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
        double *BK   = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
        memcpy(A_cl, A, (size_t)n*(size_t)n*sizeof(double));
        matmul(B, K_init, BK, n, m, n);
        for (int i = 0; i < n * n; ++i) A_cl[i] -= BK[i];

        /* P_0 из уравнения Ляпунова на замкнутой устойчивой A_cl */
        solve_lyapunov(A_cl, Q_eff, P, n);
        free(Q_eff); free(A_cl); free(BK);
    } else {
        /* Стандартная инициализация для устойчивых систем:
         * A^T P_0 + P_0 A = -C^T C */
        solve_lyapunov(A, Q, P, n);
    }
    symmetrize(P, n);  /* симметризация начального приближения */

    double prev_newton_res = -1.0;
    int it;

    /* Итерационный цикл метода Ньютона-Клейнмана */
    for (it = 0; it < maxit; ++it) {
        /* Вычисление невязки F = A^T P + P A - P S P + Q */
        at_mul_b(A, P, AP, n, n, n);
        matmul(P, A, PA, n, n, n);
        matmul(P, S, PS, n, n, n);
        matmul(PS, P, PSP, n, n, n);
        for (int i = 0; i < n * n; ++i)
            F[i] = AP[i] + PA[i] - PSP[i] + Q[i];

        double res = fro_norm(F, n * n);

        /* Контроль монотонности сходимости внутри цикла Ньютона:
         * резкий рост невязки (>2x) указывает на расходимость */
        if (show_warnings &&
            prev_newton_res > 0.0 && res > prev_newton_res * 2.0) {
            fprintf(stderr,
                "ПРЕДУПРЕЖДЕНИЕ: итерация Ньютона %d — "
                "невязка резко возросла (%.6e -> %.6e).\n",
                it, prev_newton_res, res);
        }
        prev_newton_res = res;

        if (res < tol) break;  /* критерий сходимости достигнут */

        /* Замкнутая матрица текущей итерации: Acl = A - S * P = A - B B^T P_k */
        matmul(S, P, Acl, n, n, n);
        for (int i = 0; i < n * n; ++i)
            Acl[i] = A[i] - Acl[i];

        /* Поправка Delta_k: решение уравнения Ляпунова
         *   Acl^T Delta + Delta Acl = -F(P_k) */
        solve_lyapunov(Acl, F, Delta, n);

        /* Обновление приближения и симметризация для устойчивости */
        for (int i = 0; i < n * n; ++i)
            P[i] += Delta[i];
        symmetrize(P, n);
    }

    /* Предупреждения о качестве итогового решения */
    if (show_warnings && it >= maxit) {
        fprintf(stderr,
            "ПРЕДУПРЕЖДЕНИЕ: метод Ньютона не сошёлся "
            "за %d итераций (n=%d).\n", maxit, n);
    }
    if (show_warnings && !check_symmetric(P, n, 1e-8)) {
        fprintf(stderr,
            "ПРЕДУПРЕЖДЕНИЕ: решение P несимметрично (n=%d).\n", n);
    }
    if (show_warnings && !check_diag_nonneg(P, n)) {
        fprintf(stderr,
            "ПРЕДУПРЕЖДЕНИЕ: решение P имеет отрицательные "
            "диагональные элементы (n=%d).\n", n);
    }

    free(Q); free(S); free(F); free(Acl); free(Delta);
    free(AP); free(PA); free(PS); free(PSP);
    return it;
}



/* copy_first_columns — копирование первых l столбцов матрицы Vsrc (n x ldsrc)
 * в матрицу Vdst (n x l).
 * Используется для формирования проектора V_l из полного POD-базиса V
 * на каждом шаге цикла по размерности подпространства l. */
static void copy_first_columns(const double *Vsrc, int n,
                               int ldsrc, int l, double *Vdst) {
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < l; ++j)
            Vdst[IDX(i,j,l)] = Vsrc[IDX(i,j,ldsrc)];
}

/* project_system — гэлёркинова проекция полноразмерной системы (A,B,C)
 * на подпространство, натянутое на столбцы проектора V_l (n x l):
 *   Al = V_l^T * A * V_l  — редуцированная матрица динамики (l x l),
 *   Bl = V_l^T * B        — редуцированная матрица входов  (l x m),
 *   Cl = C * V_l          — редуцированная матрица выходов (p x l).
 * Редуцированная система Al,Bl,Cl при l << N решается значительно быстрее. */
static void project_system(const double *A, const double *B,
                           const double *C, const double *V,
                           int n, int m, int p, int l,
                           double *Al, double *Bl, double *Cl) {
    /* Промежуточное произведение AV = A * V_l (n x l) */
    double *AV = (double *)xcalloc((size_t)n*(size_t)l*sizeof(double));
    matmul(A, V, AV, n, n, l);         /* AV = A * V_l         */
    at_mul_b(V, AV, Al, n, l, l);      /* Al = V_l^T * AV      */
    at_mul_b(V, B, Bl, n, l, m);       /* Bl = V_l^T * B       */
    matmul(C, V, Cl, p, n, l);         /* Cl = C * V_l         */
    free(AV);
}

/* projected_full_residual — оценка качества редуцированного решения P_l
 * относительно исходного полноразмерного уравнения Риккати.
 *
 * Алгоритм:
 *   1. Восстанавливаем полноразмерное приближение:
 *        P^(N) = V_l * P_l * V_l^T  (N x N).
 *   2. Вычисляем норму невязки исходного уравнения:
 *        ||A^T P^(N) + P^(N) A - P^(N) B B^T P^(N) + C^T C||_F.
 *
 * Монотонное убывание этой нормы при росте l подтверждает корректность
 * алгоритма. При l = N норма должна упасть до уровня машинной точности. */
static double projected_full_residual(const double *A,
                                      const double *B,
                                      const double *C,
                                      const double *V,
                                      const double *Pl,
                                      int n, int m, int p, int l) {
    double *tmp   = (double *)xcalloc((size_t)n*(size_t)l*sizeof(double));
    double *Pfull = (double *)xcalloc((size_t)n*(size_t)n*sizeof(double));
    matmul(V, Pl, tmp, n, l, l);                    /* tmp   = V_l * P_l   */
    matmul_right_transpose(tmp, V, Pfull, n, l, n); /* Pfull = tmp * V_l^T */
    double res = care_residual_norm(A, B, C, Pfull, n, m, p);
    free(tmp); free(Pfull);
    return res;
}



/* main — точка входа программы.
 *
 * Аргументы командной строки:
 *   [имя_файла]    — файл с входными данными (опционально, иначе stdin).
 *   -s | --silent  — подавить все предупреждения (тихий режим).
 *
 * Формат входных данных:
 *   N m p r steps T
 *   матрица A (N x N, построчно)
 *   матрица B (N x m, построчно)
 *   матрица C (p x N, построчно)
 *
 * Программа выводит таблицу: для каждого l от 1 до r_use —
 * число итераций Ньютона, норму невязки, признаки симметричности
 * и положительной полуопределённости решения P_l.
 * После таблицы выводятся финальные матрицы P_l и P^(N) при l = r_use. */
int main(int argc, char *argv[])
{
    FILE *in = stdin;
    char *filename = NULL;

    /* --- Разбор аргументов командной строки ---
     * Поддерживаемые форматы вызова:
     *   ./lab5                        — ввод с клавиатуры
     *   ./lab5 input.txt              — ввод из файла
     *   ./lab5 input.txt -s           — файл + тихий режим
     *   ./lab5 -s input.txt           — тихий режим + файл */
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-s") == 0 ||
            strcmp(argv[i], "--silent") == 0) {
            show_warnings = 0;  /* подавляем предупреждения */
        } else {
            filename = argv[i]; /* запоминаем имя файла */
        }
    }

    if (argc > 3 || (argc == 3 && filename == NULL)) {
        fprintf(stderr,
            "Использование: %s [имя_файла] [-s|--silent]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (filename) {
        in = fopen(filename, "r");
        if (!in) {
            perror(filename);
            return EXIT_FAILURE;
        }
        printf("Чтение данных из файла: %s\n", filename);
    } else {
        printf("Файл входных данных не указан. "
               "Введите данные в стандартный поток (stdin):\n");
    }

    int N, m, p, r, steps;
    double Tfinal;

    /* Считывание шести параметров задачи:
     *   N     — размерность пространства состояний исходной системы,
     *   m     — число входов (управлений),
     *   p     — число выходов (измерений),
     *   r     — целевой ранг POD-подпространства,
     *   steps — число шагов интегрирования траектории,
     *   T     — конечное время интегрирования. */
    if (fscanf(in, "%d %d %d %d %d %lf",
               &N, &m, &p, &r, &steps, &Tfinal) != 6) {
        fprintf(stderr, "Ошибка чтения параметров задачи.\n");
        if (in != stdin) fclose(in);
        return EXIT_FAILURE;
    }

    printf("Параметры задачи: N=%d (полный размер), m=%d (входы), p=%d (выходы),\n"
           "                  r=%d (целевой ранг), steps=%d, T=%.4f\n\n",
           N, m, p, r, steps, Tfinal);

    /* Базовая проверка корректности параметров */
    if (N <= 0 || m <= 0 || p <= 0 || r <= 0
        || steps <= 0 || Tfinal <= 0.0) {
        fprintf(stderr, "Ошибка: некорректные параметры.\n");
        if (in != stdin) fclose(in);
        return EXIT_FAILURE;
    }

    /* Выделение памяти и считывание матриц системы */
    double *A = (double *)xcalloc((size_t)N*(size_t)N*sizeof(double));
    double *B = (double *)xcalloc((size_t)N*(size_t)m*sizeof(double));
    double *C = (double *)xcalloc((size_t)p*(size_t)N*sizeof(double));

    for (int i = 0; i < N * N; ++i) {
        if (fscanf(in, "%lf", &A[i]) != 1) {
            fprintf(stderr, "Ошибка чтения матрицы A.\n");
            free(A); free(B); free(C);
            if (in != stdin) fclose(in);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < N * m; ++i) {
        if (fscanf(in, "%lf", &B[i]) != 1) {
            fprintf(stderr, "Ошибка чтения матрицы B.\n");
            free(A); free(B); free(C);
            if (in != stdin) fclose(in);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < p * N; ++i) {
        if (fscanf(in, "%lf", &C[i]) != 1) {
            fprintf(stderr, "Ошибка чтения матрицы C.\n");
            free(A); free(B); free(C);
            if (in != stdin) fclose(in);
            return EXIT_FAILURE;
        }
    }

    if (in != stdin) fclose(in);

    /* --- Автоматическое определение системы 2-го порядка ---
     * Механические системы (маятник, балка, ротор) имеют уравнения движения:
     *   M*q'' + D*q' + K*q = B2*u,
     * которые в пространстве состояний [q; q'] дают блочную матрицу:
     *   A = | 0   I  |
     *       | A22 0  |
     * Признак: верхний левый блок (N/2 x N/2) нулевой,
     *          верхний правый блок — единичная матрица.
     * Для таких неустойчивых систем используется PD-стабилизирующая инициализация. */
    int is_second_order = (N % 2 == 0);
    int half = N / 2;
    if (is_second_order) {
        for (int i = 0; i < half; ++i) {
            for (int j = 0; j < half; ++j) {
                /* Проверка нулевого блока (позиции и скорости не связаны через A) */
                if (fabs(A[IDX(i, j, N)]) > 1e-5) is_second_order = 0;
                /* Проверка единичного блока (скорость = производная позиции) */
                if (i == j) {
                    if (fabs(A[IDX(i, j + half, N)] - 1.0) > 1e-5)
                        is_second_order = 0;
                } else {
                    if (fabs(A[IDX(i, j + half, N)]) > 1e-5)
                        is_second_order = 0;
                }
            }
        }
    }

    /* rmax — реальный верхний предел ранга (не может превышать N) */
    int rmax = (r < N) ? r : N;

    /* Матрица V будет содержать rmax ортонормированных POD-мод */
    double *V = (double *)xcalloc((size_t)N*(size_t)rmax*sizeof(double));

    /* Построение POD-базиса; r_eff — фактическое число значимых мод */
    int r_eff = build_pod_basis(A, C, N, p, steps, Tfinal, V, rmax);

    /* r_use — число мод, которое будет использовано в расчёте
     * (не больше запрошенного r и не больше r_eff) */
    int r_use = (r < r_eff) ? r : r_eff;

    printf("Эффективный ранг POD-базиса: %d\n", r_eff);
    printf("Используемый ранг:            %d\n\n", r_use);

    if (r_eff == 0) {
        fprintf(stderr,
            "Ошибка: POD-базис пуст. Все собственные значения "
            "Грамиана ниже порога %.0e. Увеличьте T или steps.\n",
            EPS_EIG);
        free(A); free(B); free(C); free(V);
        return EXIT_FAILURE;
    }

    /* Заголовок сводной таблицы результатов */
    printf("%-5s  %-13s  %-22s  %-8s  %-8s  %s\n",
           "l", "Итер. Ньютона", "||R(P_l)||_F",
           "Симм.", "P>=0", "Статус");
    printf("%-5s  %-13s  %-22s  %-8s  %-8s  %s\n",
           "-----", "-------------", "----------------------",
           "--------", "--------", "-------");

    double prev_res = -1.0;

    /* Буферы для сохранения финального решения при l = r_use */
    double *Pl_final = (double *)xcalloc(
        (size_t)r_use * (size_t)r_use * sizeof(double));
    double *Vl_final = (double *)xcalloc(
        (size_t)N * (size_t)r_use * sizeof(double));

    /* Главный цикл по размерности редуцированного подпространства l = 1..r_use.
     * При l = r_use подпространство содержит максимальное число POD-мод.
     * При l = N и V = I система совпадает с исходной (невязка ~ машинная точность). */
    for (int l = 1; l <= r_use; ++l) {

        /* Формирование проектора V_l: первые l столбцов из полного базиса V */
        double *Vl = (double *)xcalloc((size_t)N*(size_t)l*sizeof(double));
        copy_first_columns(V, N, rmax, l, Vl);

        /* Проецирование полноразмерных матриц (A,B,C) на подпространство V_l */
        double *Al = (double *)xcalloc((size_t)l*(size_t)l*sizeof(double));
        double *Bl = (double *)xcalloc((size_t)l*(size_t)m*sizeof(double));
        double *Cl = (double *)xcalloc((size_t)p*(size_t)l*sizeof(double));
        project_system(A, B, C, Vl, N, m, p, l, Al, Bl, Cl);

        /* --- Формирование стабилизирующей обратной связи K_l ---
         * Для колебательных систем 2-го порядка строим PD-регулятор
         * в пространстве редуцированных координат:
         *   K_l[i,j] = alpha * V_l[позиция_i, j]
         *            + beta  * V_l[скорость_i, j]
         * где alpha — пропорциональный коэффициент (жёсткость),
         *     beta  — дифференциальный коэффициент (демпфирование).
         * Замкнутая матрица A_cl_init = Al - Bl * K_l будет устойчивой,
         * что обеспечивает корректную инициализацию метода Ньютона. */
        double *Kl = NULL;
        if (is_second_order) {
            Kl = (double *)xcalloc((size_t)m * (size_t)l * sizeof(double));
            double alpha = 20.0;  /* коэффициент жёсткости   */
            double beta  = 15.0;  /* коэффициент демпфирования */
            for (int i = 0; i < m; ++i) {
                for (int j = 0; j < l; ++j) {
                    /* Вклад от позиционных компонент (строки 0..half-1) */
                    Kl[IDX(i, j, l)] = alpha * Vl[IDX(i, j, l)]
                    /* Вклад от скоростных компонент (строки half..N-1) */
                                     + beta  * Vl[IDX(i + half, j, l)];
                }
            }
        }

        /* Решение редуцированного уравнения Риккати методом Ньютона-Клейнмана */
        double *Pl = (double *)xcalloc((size_t)l*(size_t)l*sizeof(double));
        int nit = solve_care_newton(Al, Bl, Cl, Kl, Pl, l, m, p,
                                    MAX_NEWTON_ITERS, 1e-10);

        /* Оценка невязки восстановленного полноразмерного решения P^(N) */
        double res = projected_full_residual(A, B, C, Vl, Pl, N, m, p, l);

        /* Проверки корректности найденного решения P_l */
        int sym_ok  = check_symmetric(Pl, l, 1e-8);
        int pdef_ok = check_diag_nonneg(Pl, l);

        /* Определение статуса строки таблицы */
        const char *status = "OK";
        if (nit >= MAX_NEWTON_ITERS)
            status = "НЕ СОШЁЛСЯ";      /* метод не достиг допуска */
        else if (!sym_ok || !pdef_ok)
            status = "P НЕКОРРЕКТНА";   /* нарушены физические условия */
        else if (res < 1e-12)
            status = "МАШ. ТОЧНОСТЬ";   /* достигнута точность вычислений */

        /* Проверка монотонности убывания невязки по l */
        check_residual_monotone(prev_res, res, l);

        printf("%-5d  %-13d  %-22.6e  %-8s  %-8s  %s\n",
               l, nit, res,
               sym_ok  ? "ДА" : "НЕТ",
               pdef_ok ? "ДА" : "НЕТ",
               status);

        prev_res = res;

        /* Сохранение финального решения для вывода после цикла */
        if (l == r_use) {
            memcpy(Pl_final, Pl,
                   (size_t)r_use * (size_t)r_use * sizeof(double));
            memcpy(Vl_final, Vl,
                   (size_t)N * (size_t)r_use * sizeof(double));
        }

        if (Kl) free(Kl);
        free(Vl); free(Al); free(Bl); free(Cl); free(Pl);
    }

    printf("\n"
           "=========================================================================\n"
           "Финальные результаты редуцированного уравнения Риккати (l = %d):\n"
           "=========================================================================\n",
           r_use);

    /* Вывод редуцированной матрицы P_l (размер r_use x r_use).
     * Это решение уравнения Риккати в подпространстве POD-базиса. */
    printf("Редуцированная матрица P_l (%d x %d):\n", r_use, r_use);
    for (int i = 0; i < r_use; ++i) {
        for (int j = 0; j < r_use; ++j)
            printf("%13.6e ", Pl_final[IDX(i, j, r_use)]);
        printf("\n");
    }
    printf("\n");

    /* Восстановление полноразмерного решения P^(N) = V_l * P_l * V_l^T.
     * Это приближение к истинному решению уравнения Риккати в R^{N x N}. */
    double *tmp_rc = (double *)xcalloc((size_t)N*(size_t)r_use*sizeof(double));
    double *P_full = (double *)xcalloc((size_t)N*(size_t)N*sizeof(double));
    matmul(Vl_final, Pl_final, tmp_rc, N, r_use, r_use); /* tmp = V_l * P_l */
    matmul_right_transpose(tmp_rc, Vl_final, P_full, N, r_use, N); /* P^(N) = tmp * V_l^T */

    /* Вывод восстановленной полноразмерной матрицы P^(N) (N x N).
     * При l = N эта матрица совпадает с точным решением (с точностью машинной арифметики). */
    printf("Восстановленная полноразмерная матрица P^(N) (%d x %d):\n", N, N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            printf("%13.6e ", P_full[IDX(i, j, N)]);
        printf("\n");
    }
    printf("\n");

    free(Pl_final); free(Vl_final);
    free(tmp_rc);   free(P_full);

    free(A); free(B); free(C); free(V);
    return EXIT_SUCCESS;
}