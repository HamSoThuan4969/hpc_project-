#ifndef CSV_STREAM_H
#define CSV_STREAM_H
#include <stdio.h>
/* Bo doc CSV theo dong: moi lan tra ve TRON mot tram. Giu 1 dong "nhin truoc". */
typedef struct { FILE* f; int have_pending; int pending_id; double pending_val; } CsvStream;
int  csv_open (CsvStream* s, const char* path);              /* 1=ok, 0=loi */
int  csv_next (CsvStream* s, int* id, double** vals, int* n);/* 1=co tram, 0=het; nguoi goi free(*vals) */
void csv_close(CsvStream* s);
#endif