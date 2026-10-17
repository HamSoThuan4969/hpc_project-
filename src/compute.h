#ifndef COMPUTE_H
#define COMPUTE_H

typedef struct {
    int    n_windows;   // số cửa sổ đã khớp
    double rmse;        // sai số dự đoán trung bình của trạm
    double mean_level;  // mực nước trung bình dự đoán (tham khảo)
} Result;

// h: chuỗi 1 trạm | n: số điểm | L: độ dài cửa sổ khớp
// S: bước trượt | H: tầm dự đoán (số điểm dự đoán sau mỗi cửa sổ)
Result compute_station(const double* h, int n, int L, int S, int H);

#endif