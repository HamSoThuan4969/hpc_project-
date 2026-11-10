#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "compute.h"
#include "csv_stream.h"
#define BATCH 256                      /* so tram moi lo (RAM dinh = 1 lo) */
typedef struct { double** vals; int* cnt; Result* res; int start,end,L,S,H; } Arg;
static void* worker(void* a_){
    Arg* a=(Arg*)a_;
    for(int i=a->start;i<a->end;i++) a->res[i]=compute_station(a->vals[i],a->cnt[i],a->L,a->S,a->H);
    return NULL;
}
static double now(){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
int main(int argc,char** argv){
    if(argc<2){ fprintf(stderr,"Dung: %s file.csv [so_luong]\n",argv[0]); return 1; }
    int T=(argc>=3)?atoi(argv[2]):4; if(T<1)T=1;
    int L=720,S=360,H=24;
    CsvStream s; if(!csv_open(&s,argv[1])){ fprintf(stderr,"Khong mo duoc file\n"); return 1; }

    double* vals[BATCH]; int cnt[BATCH], ids[BATCH]; Result res[BATCH];
    pthread_t th[64]; Arg arg[64]; if(T>64)T=64;
    Result r3[3]; int id3[3], shown=0, N=0; double comp=0;
    int more=1;
    while(more){
        /* 1) DOC mot lo (ngoai vung do gio) */
        int b=0;
        for(; b<BATCH; b++){ if(!csv_next(&s,&ids[b],&vals[b],&cnt[b])){ more=0; break; } }
        if(b==0) break;
        /* 2) TINH song song ca lo (trong vung do gio) */
        double c0=now();
        int tt=(T<b)?T:b, base=b/tt, rem=b%tt, pos=0;
        for(int t=0;t<tt;t++){ int c=base+(t<rem?1:0);
            arg[t]=(Arg){vals,cnt,res,pos,pos+c,L,S,H}; pos+=c;
            pthread_create(&th[t],NULL,worker,&arg[t]); }
        for(int t=0;t<tt;t++) pthread_join(th[t],NULL);
        comp+=now()-c0;
        /* 3) ghi 3 mau + giai phong lo */
        for(int i=0;i<b;i++){ if(shown<3){ r3[shown]=res[i]; id3[shown]=ids[i]; shown++; } free(vals[i]); N++; }
    }
    csv_close(&s);
    for(int i=0;i<shown;i++) printf("tram %d: windows=%d rmse=%.6f mean=%.4f\n",id3[i],r3[i].n_windows,r3[i].rmse,r3[i].mean_level);
    printf("BENCHMARK pthreads_stream num_threads=%d num_tasks=%d time=%.6f s\n",T,N,comp);
    return 0;
}