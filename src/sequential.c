#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "csv_reader.h"
#include "compute.h"

int main(int argc, char** argv){
    const char* file = argc>1 ? argv[1] : "data/synth_10st_sec.csv";
    int L=720, S=360, H=24;          // cửa sổ, bước trượt, tầm dự đoán
    int n; StationData* st=read_csv(file,&n);
    if(!st) return 1;
    Result* res=malloc(n*sizeof(Result));
    struct timespec t0,t1; clock_gettime(CLOCK_MONOTONIC,&t0);
    for(int i=0;i<n;i++) res[i]=compute_station(st[i].values,st[i].count,L,S,H);
    clock_gettime(CLOCK_MONOTONIC,&t1);
    double el=(t1.tv_sec-t0.tv_sec)+(t1.tv_nsec-t0.tv_nsec)/1e9;
    int show=n<3?n:3;
    for(int i=0;i<show;i++)
        printf("tram %d: windows=%d rmse=%.6f mean=%.4f\n",
               st[i].station_id,res[i].n_windows,res[i].rmse,res[i].mean_level);
    printf("BENCHMARK sequential  num_tasks=%d  series_len=%d  time=%.6f s\n",
           n,(n>0?st[0].count:0),el);
    free(res); free_stations(st,n); return 0;
}