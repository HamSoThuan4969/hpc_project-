#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "csv_reader.h"
#include "compute.h"

int main(int argc, char** argv) {
    const char* file = argc > 1 ? argv[1] : "data/synth_10st_sec.csv"; // nhập tham số
    int W = 60; double k = 2.0;

    int n;
    StationData* st = read_csv(file, &n); // đọc 
    if (!st) return 1;

    Result* res = malloc(n * sizeof(Result));     // mảng chứa kết quả

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);          // BẮT ĐẦU đo
    for (int i = 0; i < n; i++)
        res[i] = compute_station(st[i].values, st[i].count, W, k);  // chỉ tính
    clock_gettime(CLOCK_MONOTONIC, &t1);          // KẾT THÚC đo

    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    int show = n < 3 ? n : 3;                     // in 3 trạm đầu làm mẫu
    for (int i = 0; i < show; i++)
        printf("tram %d: mean=%.4f std=%.4f roll=%.4f peaks=%d\n",
               st[i].station_id, res[i].mean, res[i].std,
               res[i].max_roll_std, res[i].count_peaks);

    printf("BENCHMARK sequential  num_tasks=%d  series_len=%d  time=%.6f s\n",
           n, (n > 0 ? st[0].count : 0), elapsed);

    free(res);
    free_stations(st, n);
    return 0;
}