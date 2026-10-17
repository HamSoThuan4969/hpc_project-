#include <math.h>
#include "compute.h"

#define K 4              // số sóng triều
#define M (2*K+1)        // số ẩn = 9
#define DT 1.0           // bước thời gian mỗi điểm (đơn vị: giờ) — xem ghi chú

// chu kỳ 4 sóng (cùng đơn vị với DT): M2, S2, K1, O1
static const double PERIOD[K] = {12.4206, 12.0, 23.9345, 25.8193};

static void basis(double t, const double* w, double* X) {
    X[0] = 1.0;
    for (int i = 0; i < K; i++) { X[1+2*i]=cos(w[i]*t); X[2+2*i]=sin(w[i]*t); }
}

static void solve_gauss(double A[M][M], double d[M], double c[M]) {
    for (int col = 0; col < M; col++) {
        int piv = col; double best = fabs(A[col][col]);
        for (int r = col+1; r < M; r++)
            if (fabs(A[r][col]) > best) { best = fabs(A[r][col]); piv = r; }
        if (piv != col) {
            for (int k = 0; k < M; k++){double t=A[col][k];A[col][k]=A[piv][k];A[piv][k]=t;}
            double t=d[col]; d[col]=d[piv]; d[piv]=t;
        }
        for (int r = col+1; r < M; r++) {
            double f = A[r][col]/A[col][col];
            for (int k = col; k < M; k++) A[r][k] -= f*A[col][k];
            d[r] -= f*d[col];
        }
    }
    for (int i = M-1; i >= 0; i--) {
        double s = d[i];
        for (int k = i+1; k < M; k++) s -= A[i][k]*c[k];
        c[i] = s/A[i][i];
    }
}

// khớp trên L điểm (thời gian cục bộ 0..(L-1)*DT) → hệ số c
static void fit_window(const double* h, int L, const double* w, double c[M]) {
    double A[M][M] = {{0}}, d[M] = {0}, X[M];
    for (int j = 0; j < L; j++) {
        basis(j*DT, w, X);
        for (int p = 0; p < M; p++) {
            for (int q = 0; q < M; q++) A[p][q] += X[p]*X[q];
            d[p] += X[p]*h[j];
        }
    }
    solve_gauss(A, d, c);
}

static double predict(double t, const double* w, const double c[M]) {
    double X[M]; basis(t, w, X);
    double s = 0; for (int p = 0; p < M; p++) s += c[p]*X[p];
    return s;
}

Result compute_station(const double* h, int n, int L, int S, int H) {
    double w[K]; for (int i = 0; i < K; i++) w[i] = 2*M_PI/PERIOD[i];
    Result r = {0, 0.0, 0.0};
    double se = 0, sum_pred = 0; long cnt = 0;

    // trượt cửa sổ: khớp trên [pos, pos+L), dự đoán H điểm kế rồi so với thật
    for (int pos = 0; pos + L + H <= n; pos += S) {
        double c[M];
        fit_window(&h[pos], L, w, c);              // khớp
        for (int hh = 0; hh < H; hh++) {
            double pred = predict((L+hh)*DT, w, c); // dự đoán ngoài cửa sổ
            double actual = h[pos + L + hh];
            double e = pred - actual;
            se += e*e; sum_pred += pred; cnt++;
        }
        r.n_windows++;
    }
    if (cnt > 0) { r.rmse = sqrt(se/cnt); r.mean_level = sum_pred/cnt; }
    return r;
}