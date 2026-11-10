#define _GNU_SOURCE
#include <stdlib.h>
#include <ctype.h>
#include "csv_stream.h"
int csv_open(CsvStream* s, const char* path){
    s->f=fopen(path,"r"); s->have_pending=0;
    if(!s->f) return 0;
    long pos=ftell(s->f); int c=fgetc(s->f);          /* bo dong tieu de neu co */
    if(c!=EOF && !(c=='-'||isdigit(c))){ while((c=fgetc(s->f))!=EOF && c!='\n'); }
    else fseek(s->f,pos,SEEK_SET);
    return 1;
}
int csv_next(CsvStream* s, int* out_id, double** vals, int* n_out){
    int id; double val; int cap=1024,n=0; double* v=malloc(cap*sizeof(double)); int cur;
    if(s->have_pending){ cur=s->pending_id; v[n++]=s->pending_val; s->have_pending=0; }
    else { if(fscanf(s->f,"%d,%lf",&id,&val)!=2){ free(v); return 0; } cur=id; v[n++]=val; }
    while(fscanf(s->f,"%d,%lf",&id,&val)==2){
        if(id!=cur){ s->have_pending=1; s->pending_id=id; s->pending_val=val; break; }
        if(n>=cap){ cap*=2; v=realloc(v,cap*sizeof(double)); }
        v[n++]=val;
    }
    *out_id=cur; *vals=v; *n_out=n; return 1;
}
void csv_close(CsvStream* s){ if(s->f) fclose(s->f); }