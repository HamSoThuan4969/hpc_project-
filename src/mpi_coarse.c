/* mpi_coarse.c — Song song hoa CHIA THO bang MPI.
 * Khac pthreads: moi TIEN TRINH co bo nho RIENG, khong co res[] dung chung.
 * Nen sau khi tinh xong, cac tien trinh phai GUI ket qua ve tien trinh 0
 * bang MPI_Gatherv (message passing).
 *
 * Bien dich:
 *   mpicc -O2 -o mpi mpi_coarse.c csv_reader.c compute.c -lm
 * Chay (vi du 4 tien trinh):
 *   mpirun -np 4 ./mpi data/synth_1000st_sec.csv
 * Neu so tien trinh > so nhan (vi du 16 tren 8 nhan), them --oversubscribe:
 *   mpirun --oversubscribe -np 16 ./mpi data/synth_1000st_sec.csv
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "csv_reader.h"
#include "compute.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);     /* so thu tu cua tien trinh nay: 0,1,2,... */
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);   /* tong so tien trinh */

    if (argc < 2) {
        if (rank == 0) fprintf(stderr, "Dung: mpirun -np N %s <file.csv>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    const char *path = argv[1];
    int L = 720, S = 360, H = 24;

    /* === MOI tien trinh TU doc CSV (doc re, ~0% thoi gian) === */
    int N = 0;
    StationData *data = read_csv(path, &N);
    if (!data || N == 0) {
        if (rank == 0) fprintf(stderr, "Doc du lieu that bai\n");
        MPI_Finalize();
        return 1;
    }

    /* === Chia N tram cho nprocs tien trinh (chia deu, du rai cho rank dau) === */
    int base = N / nprocs, rem = N % nprocs;
    int start = rank * base + (rank < rem ? rank : rem);  /* tram dau cua rank nay */
    int cnt   = base + (rank < rem ? 1 : 0);              /* so tram rank nay lo   */

    /* res CUC BO: chi chua ket qua cua cum rank nay (khong phai ca N) */
    Result *local = malloc((size_t)cnt * sizeof(Result));

    /* === DO GIO phan tinh toan === */
    MPI_Barrier(MPI_COMM_WORLD);              /* doi tat ca cung xuat phat */
    double t0 = MPI_Wtime();

    for (int i = 0; i < cnt; i++)
        local[i] = compute_station(data[start + i].values, data[start + i].count,
                                   L, S, H);

    MPI_Barrier(MPI_COMM_WORLD);              /* doi tat ca cung ve dich */
    double t1 = MPI_Wtime();

    /* === GOM ket qua ve tien trinh 0 bang message passing === */
    /* Gui moi Result nhu mot khoi byte (struct POD, moi may cung kien truc). */
    Result *res = NULL;                       /* chi tien trinh 0 can mang day du */
    int *counts = NULL, *displs = NULL;
    if (rank == 0) {
        res    = malloc((size_t)N * sizeof(Result));
        counts = malloc((size_t)nprocs * sizeof(int));  /* moi rank gui bao nhieu byte */
        displs = malloc((size_t)nprocs * sizeof(int));  /* ghep vao vi tri nao          */
    }

    int mybytes = cnt * (int)sizeof(Result);
    /* B1: moi rank bao cho rank 0 biet no se gui bao nhieu byte */
    MPI_Gather(&mybytes, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    /* B2: rank 0 tinh vi tri ghep cua tung rank */
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < nprocs; i++)
            displs[i] = displs[i - 1] + counts[i - 1];
    }
    /* B3: moi rank GUI local; rank 0 NHAN va ghep vao res dung cho */
    MPI_Gatherv(local, mybytes, MPI_BYTE,
                res, counts, displs, MPI_BYTE,
                0, MPI_COMM_WORLD);

    /* === Chi tien trinh 0 in ket qua === */
    if (rank == 0) {
        for (int i = 0; i < 3 && i < N; i++)
            printf("Tram %d: rmse=%.6f n_windows=%d\n",
                   data[i].station_id, res[i].rmse, res[i].n_windows);

        double elapsed = t1 - t0;   /* nho 2 barrier → phan anh tien trinh cham nhat */
        printf("BENCHMARK mpi_coarse num_procs=%d num_tasks=%d series_len=%d time=%.6f s\n",
               nprocs, N, data[0].count, elapsed);

        free(res); free(counts); free(displs);
    }

    free(local);
    MPI_Finalize();
    return 0;
}