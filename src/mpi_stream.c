#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "compute.h"
#include "csv_stream.h"
/* MPI streaming: MOI tien trinh doc file theo dong (RAM = 1 tram), nhung chi
 * TINH cac tram co chi so g % size == rank (chia vong tron). Cuoi cung gop
 * ket qua ve rank 0 bang MPI_Gatherv va xep lai theo chi so. */
int main(int argc,char** argv){
    MPI_Init(&argc,&argv);
    int rank,size; MPI_Comm_rank(MPI_COMM_WORLD,&rank); MPI_Comm_size(MPI_COMM_WORLD,&size);
    if(argc<2){ if(rank==0) fprintf(stderr,"Dung: mpirun -np K %s file.csv\n",argv[0]); MPI_Finalize(); return 1; }
    int L=720,S=360,H=24;
    CsvStream s; if(!csv_open(&s,argv[1])){ if(rank==0) fprintf(stderr,"Khong mo duoc file\n"); MPI_Finalize(); return 1; }

    int cap=1024,k=0; int* lidx=malloc(cap*sizeof(int)); int* lid=malloc(cap*sizeof(int));
    Result* lres=malloc(cap*sizeof(Result));
    int g=0,id,cnt; double* vals; double comp=0;

    MPI_Barrier(MPI_COMM_WORLD);
    while(csv_next(&s,&id,&vals,&cnt)){
        if(g%size==rank){                          /* tram nay cua rank minh */
            double c0=MPI_Wtime();
            Result r=compute_station(vals,cnt,L,S,H);
            comp+=MPI_Wtime()-c0;
            if(k>=cap){ cap*=2; lidx=realloc(lidx,cap*sizeof(int)); lid=realloc(lid,cap*sizeof(int)); lres=realloc(lres,cap*sizeof(Result)); }
            lidx[k]=g; lid[k]=id; lres[k]=r; k++;
        }
        free(vals); g++;
    }
    csv_close(&s);
    int N=g;                                       /* moi rank deu quet het nen g bang nhau */

    int *counts=NULL,*displs=NULL,*bcnt=NULL,*bdsp=NULL;
    int *all_idx=NULL,*all_id=NULL; Result* all_res=NULL;
    if(rank==0){ counts=malloc(size*4); displs=malloc(size*4); bcnt=malloc(size*4); bdsp=malloc(size*4); }
    MPI_Gather(&k,1,MPI_INT,counts,1,MPI_INT,0,MPI_COMM_WORLD);
    if(rank==0){
        displs[0]=0; for(int p=1;p<size;p++) displs[p]=displs[p-1]+counts[p-1];
        for(int p=0;p<size;p++){ bcnt[p]=counts[p]*(int)sizeof(Result); bdsp[p]=displs[p]*(int)sizeof(Result); }
        all_idx=malloc((size_t)N*4); all_id=malloc((size_t)N*4); all_res=malloc((size_t)N*sizeof(Result));
    }
    MPI_Gatherv(lidx,k,MPI_INT, all_idx,counts,displs,MPI_INT, 0,MPI_COMM_WORLD);
    MPI_Gatherv(lid ,k,MPI_INT, all_id ,counts,displs,MPI_INT, 0,MPI_COMM_WORLD);
    MPI_Gatherv(lres,k*(int)sizeof(Result),MPI_BYTE, all_res,bcnt,bdsp,MPI_BYTE, 0,MPI_COMM_WORLD);
    double comp_max; MPI_Reduce(&comp,&comp_max,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(rank==0){
        Result* res=malloc((size_t)N*sizeof(Result)); int* sid=malloc((size_t)N*4);
        for(int i=0;i<N;i++){ int gi=all_idx[i]; res[gi]=all_res[i]; sid[gi]=all_id[i]; }
        for(int i=0;i<3&&i<N;i++)
            printf("tram %d: windows=%d rmse=%.6f mean=%.4f\n",sid[i],res[i].n_windows,res[i].rmse,res[i].mean_level);
        double sum=0; for(int i=0;i<N;i++) sum+=res[i].rmse;
        printf("CHECK stations=%d sum_rmse=%.9f\n",N,sum);
        printf("BENCHMARK mpi_stream num_procs=%d num_tasks=%d time=%.6f s\n",size,N,comp_max);
        free(res); free(sid);
    }
    free(lidx); free(lid); free(lres);
    MPI_Finalize(); return 0;
}