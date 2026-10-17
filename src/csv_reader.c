#include <stdio.h>
#include <stdlib.h>
#include "csv_reader.h"

StationData* read_csv(const char* filename, int* n_stations) {
    FILE* f = fopen(filename, "r");
    if (!f) { perror("fopen"); return NULL; }
    char line[128];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return NULL; }  // bỏ header

    StationData* st = NULL; int n = 0, cap = 0;   // mảng trạm
    int cur_id = -1;                              // id trạm đang gom
    double* vals = NULL; int vc = 0, vcap = 0;    // mảng giá trị trạm hiện tại
    int sid; double val;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d,%lf", &sid, &val) != 2) continue;
        if (sid != cur_id) {                       // gặp trạm mới
            if (cur_id != -1) {                     // lưu trạm vừa gom xong
                if (n == cap) { cap = cap ? cap*2 : 16;
                    st = realloc(st, cap*sizeof(StationData)); }
                st[n].station_id = cur_id; st[n].values = vals; st[n].count = vc; n++;
            }
            cur_id = sid; vals = NULL; vc = 0; vcap = 0;   // reset cho trạm mới
        }
        if (vc == vcap) { vcap = vcap ? vcap*2 : 1024;
            vals = realloc(vals, vcap*sizeof(double)); }
        vals[vc++] = val;
    }
    if (cur_id != -1) {                            // này là  trạm cuối cùng
        if (n == cap) { cap = cap ? cap*2 : 16;
            st = realloc(st, cap*sizeof(StationData)); }
        st[n].station_id = cur_id; st[n].values = vals; st[n].count = vc; n++;
    }
    fclose(f);
    *n_stations = n;
    return st;
}

void free_stations(StationData* s, int n) {
    for (int i = 0; i < n; i++) free(s[i].values);
    free(s);
}