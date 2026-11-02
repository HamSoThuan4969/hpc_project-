/* pthreads_fine.c — Song song hoa CHIA MIN bang pthreads.
 *
 * Giong mpi_fine ve y tuong (chia L diem cua moi cua so, roi GOP), nhung:
 *  - Tao T luong MOT lan (khong tao/huy moi cua so).
 *  - Gop partial trong BO NHO CHUNG (cong mang partial), dong bo bang barrier.
 *    Barrier re hon nhieu so voi MPI_Allreduce -> ky vong KHONG sup nhu MPI.
 *
 * Bien dich:
 *   gcc -O2 -o pthfine pthreads_fine.c csv_reader.c compute.c -lm -lpthread
 * Chay:
 *   ./pthfine data/synth_1000st_sec.csv 4
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include "csv_reader.h"
#include "compute.h"

typedef struct {
    int tid, T;
    StationData *data;
    int N, L, S, H;
    const double *w;
    double DT;
    pthread_barrier_t *bar;
    double *partialA;   /* [T * FIT_M * FIT_M] — moi luong mot slice */
    double *partiald;   /* [T * FIT_M]                              */
    Result *res;        /* chung, chi luong 0 ghi                   */
} FineArg;

static void *worker(void *arg) {
    FineArg *a = (FineArg *)arg;
    int tid = a->tid, T = a->T, L = a->L, S = a->S, H = a->H;
    const double *w = a->w; double DT = a->DT;

    /* slice partial cua rieng luong nay (ep ve dang A[FIT_M][FIT_M]) */
    double (*myA)[FIT_M] = (double (*)[FIT_M]) &a->partialA[tid * FIT_M * FIT_M];
    double  *myd         =                     &a->partiald[tid * FIT_M];

    for (int i = 0; i < a->N; i++) {
        const double *h = a->data[i].values;
        int n = a->data[i].count;
        double se = 0, sum_pred = 0; long cnt = 0; int nwin = 0;

        for (int pos = 0; pos + L + H <= n; pos += S) {
            const double *win = &h[pos];

            /* chia L diem cho T luong */
            int base = L / T, rem = L % T;
            int lo = tid * base + (tid < rem ? tid : rem);
            int cc = base + (tid < rem ? 1 : 0);
            int hi = lo + cc;

            /* 1) xoa partial cua minh roi cong don phan diem [lo,hi) */
            for (int p = 0; p < FIT_M; p++) {
                myd[p] = 0;
                for (int q = 0; q < FIT_M; q++) myA[p][q] = 0;
            }
            fit_accumulate_range(win, lo, hi, w, myA, myd);

            pthread_barrier_wait(a->bar);      /* doi moi luong xong partial */

            /* 2) luong 0 GOP tat ca partial (trong bo nho chung), giai, du doan */
            if (tid == 0) {
                double A[FIT_M][FIT_M], d[FIT_M], c[FIT_M];
                for (int p = 0; p < FIT_M; p++) {
                    d[p] = 0;
                    for (int q = 0; q < FIT_M; q++) A[p][q] = 0;
                }
                for (int t = 0; t < T; t++) {
                    double (*tA)[FIT_M] = (double (*)[FIT_M]) &a->partialA[t * FIT_M * FIT_M];
                    double  *td         =                     &a->partiald[t * FIT_M];
                    for (int p = 0; p < FIT_M; p++) {
                        d[p] += td[p];
                        for (int q = 0; q < FIT_M; q++) A[p][q] += tA[p][q];
                    }
                }
                fit_solve(A, d, c);
                for (int hh = 0; hh < H; hh++) {
                    double pred   = fit_predict((L + hh) * DT, w, c);
                    double actual = h[pos + L + hh];
                    double e = pred - actual;
                    se += e * e; sum_pred += pred; cnt++;
                }
            }

            pthread_barrier_wait(a->bar);      /* doi luong 0 xong moi ghi de partial cua su sau */
            nwin++;
        }

        if (tid == 0) {
            a->res[i].n_windows  = nwin;
            a->res[i].rmse       = (cnt > 0) ? sqrt(se / cnt) : 0.0;
            a->res[i].mean_level = (cnt > 0) ? sum_pred / cnt : 0.0;
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Dung: %s <file.csv> [so_luong]\n", argv[0]);
        return 1;
    }
    const char *path = argv[1];
    int T = (argc >= 3) ? atoi(argv[2]) : 4;
    if (T < 1) T = 1;
    int L = 720, S = 360, H = 24;

    int N = 0;
    StationData *data = read_csv(path, &N);
    if (!data || N == 0) { fprintf(stderr, "Doc du lieu that bai\n"); return 1; }
    printf("Doc duoc %d tram\n", N);

    double w[FIT_K]; fit_frequencies(w);
    double DT = fit_dt();

    Result *res = malloc((size_t)N * sizeof(Result));
    double *partialA = calloc((size_t)T * FIT_M * FIT_M, sizeof(double));
    double *partiald = calloc((size_t)T * FIT_M,         sizeof(double));

    pthread_barrier_t bar;
    pthread_barrier_init(&bar, NULL, T);

    pthread_t *tid  = malloc((size_t)T * sizeof(pthread_t));
    FineArg   *args = malloc((size_t)T * sizeof(FineArg));

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int t = 0; t < T; t++) {
        args[t] = (FineArg){ t, T, data, N, L, S, H, w, DT, &bar, partialA, partiald, res };
        pthread_create(&tid[t], NULL, worker, &args[t]);
    }
    for (int t = 0; t < T; t++) pthread_join(tid[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    for (int i = 0; i < 3 && i < N; i++)
        printf("Tram %d: rmse=%.6f n_windows=%d\n",
               data[i].station_id, res[i].rmse, res[i].n_windows);
    printf("BENCHMARK pthreads_fine num_threads=%d num_tasks=%d series_len=%d time=%.6f s\n",
           T, N, (N > 0 ? data[0].count : 0), elapsed);

    pthread_barrier_destroy(&bar);
    free(tid); free(args); free(res); free(partialA); free(partiald);
    return 0;
}