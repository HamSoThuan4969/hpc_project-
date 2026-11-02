/* mpi_fine.c — Song song hoa CHIA MIN bang MPI.
 * Cac tien trinh KHONG chia tram. Tat ca cung xu ly TUNG cua so:
 * moi tien trinh cong don MOT PHAN cua L diem -> A_cuc_bo, roi MPI_Allreduce gop.
 * Du kien CHAM (238.000 lan Allreduce) — day la ket qua can do, khong phai loi.
 *
 * Bien dich: mpicc -O2 -o mpifine mpi_fine.c csv_reader.c compute.c -lm
 * Chay:      mpirun -np 4 ./mpifine data/synth_1000st_sec.csv
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "csv_reader.h"
#include "compute.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc < 2) {
        if (rank == 0) fprintf(stderr, "Dung: mpirun -np N %s <file.csv>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    const char *path = argv[1];
    int L = 720, S = 360, H = 24;

    int N = 0;
    StationData *data = read_csv(path, &N);
    if (!data || N == 0) {
        if (rank == 0) fprintf(stderr, "Doc du lieu that bai\n");
        MPI_Finalize();
        return 1;
    }

    double w[FIT_K]; fit_frequencies(w);
    double DT = fit_dt();

    Result *res = (rank == 0) ? malloc((size_t)N * sizeof(Result)) : NULL;

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    for (int i = 0; i < N; i++) {
        const double *h = data[i].values;
        int n = data[i].count;
        double se = 0, sum_pred = 0; long cnt = 0; int nwin = 0;

        for (int pos = 0; pos + L + H <= n; pos += S) {
            const double *win = &h[pos];

            int base = L / nprocs, rem = L % nprocs;
            int lo  = rank * base + (rank < rem ? rank : rem);
            int cc  = base + (rank < rem ? 1 : 0);
            int hi  = lo + cc;

            double Aloc[FIT_M][FIT_M] = {{0}}, dloc[FIT_M] = {0};
            fit_accumulate_range(win, lo, hi, w, Aloc, dloc);

            double A[FIT_M][FIT_M], d[FIT_M], c[FIT_M];
            MPI_Allreduce(&Aloc[0][0], &A[0][0], FIT_M * FIT_M, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            MPI_Allreduce(dloc,        d,        FIT_M,         MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

            fit_solve(A, d, c);

            for (int hh = 0; hh < H; hh++) {
                double pred   = fit_predict((L + hh) * DT, w, c);
                double actual = h[pos + L + hh];
                double e = pred - actual;
                se += e * e; sum_pred += pred; cnt++;
            }
            nwin++;
        }

        if (rank == 0) {
            res[i].n_windows  = nwin;
            res[i].rmse       = (cnt > 0) ? sqrt(se / cnt) : 0.0;
            res[i].mean_level = (cnt > 0) ? sum_pred / cnt : 0.0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    if (rank == 0) {
        for (int i = 0; i < 3 && i < N; i++)
            printf("Tram %d: rmse=%.6f n_windows=%d\n",
                   data[i].station_id, res[i].rmse, res[i].n_windows);
        printf("BENCHMARK mpi_fine num_procs=%d num_tasks=%d series_len=%d time=%.6f s\n",
               nprocs, N, data[0].count, t1 - t0);
        free(res);
    }

    MPI_Finalize();
    return 0;
}