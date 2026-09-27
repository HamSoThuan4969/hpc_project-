#ifndef COMPUTE_H
#define COMPUTE_H

typedef struct {
    double mean, max, min, std;   // thống kê toàn chuỗi
    double max_roll_std;          // std cửa sổ trượt lớn nhất
    int    count_peaks;           // số điểm vượt ngưỡng
} Result;

Result compute_station(const double* x, int n, int W, double k);

#endif