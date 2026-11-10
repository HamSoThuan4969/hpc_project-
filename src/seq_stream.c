#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "compute.h"
#include "csv_stream.h"
static double now(){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
int main(int argc,char** argv){
    if(argc<2){ fprintf(stderr,"Dung: %s file.csv\n",argv[0]); return 1; }
    int L=720,S=360,H=24;
    CsvStream s; if(!csv_open(&s,argv[1])){ fprintf(stderr,"Khong mo duoc file\n"); return 1; }
    Result r3[3]; int id3[3], shown=0, N=0;
    int id,cnt; double* vals; double comp=0;
    while(csv_next(&s,&id,&vals,&cnt)){
        double c0=now(); Result r=compute_station(vals,cnt,L,S,H); comp+=now()-c0;
        free(vals);
        if(shown<3){ r3[shown]=r; id3[shown]=id; shown++; } N++;
    }
    csv_close(&s);
    for(int i=0;i<shown;i++) printf("tram %d: windows=%d rmse=%.6f mean=%.4f\n",id3[i],r3[i].n_windows,r3[i].rmse,r3[i].mean_level);
    printf("BENCHMARK seq_stream num_tasks=%d time=%.6f s\n",N,comp);
    return 0;
}