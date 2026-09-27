#ifndef CSV_READER_H
#define CSV_READER_H

typedef struct {
    int     station_id;   // id của trạm
    double* values;       // mảng giá trị (cấp phát động)
    int     count;        // số điểm của trạm
} StationData;

// đọc file csv để lấy dữ liệu của các trạm  
StationData* read_csv(const char* filename, int* n_stations);
void free_stations(StationData* s, int n); // chưa viết 

#endif