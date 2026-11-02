#ifndef COMPUTE_H
#define COMPUTE_H

typedef struct {
    int    n_windows;   // số cửa sổ đã khớp
    double rmse;        // sai số dự đoán trung bình của trạm
    double mean_level;  // mực nước trung bình dự đoán (tham khảo)
} Result;

Result compute_station(const double* h, int n, int L, int S, int H);

/* ===== Lõi khớp LỘ RA cho bản CHIA MỊN (pthreads/MPI) ===== */
#define FIT_K 4              // số sóng triều
#define FIT_M (2*FIT_K+1)    // số ẩn = 9

// Điền tần số góc w[FIT_K] (dùng chung).
void   fit_frequencies(double w[FIT_K]);

// Bước thời gian mỗi điểm (DT).
double fit_dt(void);

// Cộng dồn các điểm [lo,hi) của cửa sổ h vào A (FIT_M×FIT_M) và d (FIT_M).
// KHÔNG tự xóa A,d — người gọi tự khởi tạo/gộp. Thời gian điểm j = j*DT.
void   fit_accumulate_range(const double* h, int lo, int hi, const double* w,
                            double A[FIT_M][FIT_M], double d[FIT_M]);

// Giải hệ FIT_M×FIT_M (phá hủy A,d) → hệ số c.
void   fit_solve(double A[FIT_M][FIT_M], double d[FIT_M], double c[FIT_M]);

// Dự đoán giá trị tại thời gian cục bộ t.
double fit_predict(double t, const double* w, const double c[FIT_M]);

#endif