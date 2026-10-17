/* pthreads_coarse.c — Song song hoa CHIA THO bang pthreads.
 * Moi luong lo mot cum tram doc lap, chay tron 4 khoi cho cum do.
 * Khong can mutex: cac luong ghi vao cac doan chi so RIENG BIET cua res[].
 *
 * Bien dich:
 *   gcc -O2 -o pth pthreads_coarse.c csv_reader.c compute.c -lm -lpthread
 * Chay (vi du 4 luong):
 *   ./pth data/bo_du_lieu.csv 4
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "csv_reader.h"
#include "compute.h"

/* Goi gui cho moi luong: no can biet lam tram nao den tram nao,
 * va dia chi hai mang dung chung (data de doc, res de ghi). */
typedef struct {
    StationData *data;   /* mang tram  (dung chung, CHI DOC)            */
    Result      *res;    /* mang ket qua (dung chung, ghi doan rieng)   */
    int start, end;      /* doan tram luong nay phu trach: [start, end) */
    int L, S, H;
} ThreadArg;

/* Than cua moi luong: dung y het vong lap cua ban tuan tu,
 * chi khac la no chi chay tren doan [start, end) cua rieng no. */
static void *worker(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    for (int i = a->start; i < a->end; i++) {
        a->res[i] = compute_station(a->data[i].values, a->data[i].count,
                                    a->L, a->S, a->H);
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Dung: %s <file.csv> [so_luong]\n", argv[0]);
        return 1;
    }
    const char *path = argv[1];
    int T = (argc >= 3) ? atoi(argv[2]) : 4;   /* so luong, mac dinh 4 */

    int L = 720, S = 360, H = 24;              /* giong ban tuan tu */

    /* === DOC DU LIEU (tuan tu, 1 lan) — khop voi sequential.c cua ban === */
    int N = 0;
    StationData *data = read_csv(path, &N);
    if (!data || N == 0) { fprintf(stderr, "Doc du lieu that bai\n"); return 1; }
    printf("Doc duoc %d tram\n", N);

    Result *res = malloc((size_t)N * sizeof(Result));
    if (!res) { fprintf(stderr, "Thieu bo nho\n"); return 1; }

    if (T > N) T = N;                          /* khong can nhieu luong hon tram */

    pthread_t *tid  = malloc((size_t)T * sizeof(pthread_t));
    ThreadArg *args = malloc((size_t)T * sizeof(ThreadArg));

    /* === DO GIO: chi tinh phan song song (tao + chay + gom luong) === */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    /* Chia N tram cho T luong: chia deu, phan du rai cho cac luong dau. */
    int base = N / T, rem = N % T, pos = 0;
    for (int t = 0; t < T; t++) {
        int cnt = base + (t < rem ? 1 : 0);    /* so tram luong t nhan */
        args[t].data  = data;
        args[t].res   = res;
        args[t].start = pos;
        args[t].end   = pos + cnt;
        args[t].L = L; args[t].S = S; args[t].H = H;
        pos += cnt;
        pthread_create(&tid[t], NULL, worker, &args[t]);  /* tha luong chay */
    }
    for (int t = 0; t < T; t++)
        pthread_join(tid[t], NULL);            /* doi tat ca luong xong */

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    /* === IN KET QUA (khop voi sequential.c) === */
    for (int i = 0; i < 3 && i < N; i++)
        printf("Tram %d: rmse=%.6f n_windows=%d\n",
               data[i].station_id, res[i].rmse, res[i].n_windows);

    printf("BENCHMARK pthreads_coarse num_threads=%d num_tasks=%d series_len=%d time=%.6f s\n",
           T, N, (N > 0 ? data[0].count : 0), elapsed);

    free(tid); free(args); free(res);
    return 0;
}