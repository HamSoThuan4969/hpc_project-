#include <math.h>
#include "compute.h"

Result compute_station(const double* x, int n, int W, double k) {
    Result r;
    double sum = 0.0;  r.max = x[0];  r.min = x[0];
    for (int i = 0; i < n; i++) {              // (a) thống kê toàn chuỗi
        sum += x[i];
        if (x[i] > r.max) r.max = x[i];
        if (x[i] < r.min) r.min = x[i];
    }
    r.mean = sum / n;

    double sq = 0.0;
    for (int i = 0; i < n; i++) { double d = x[i]-r.mean; sq += d*d; }
    r.std = sqrt(sq / n);

    r.max_roll_std = 0.0;                      // (b) cửa sổ trượt
    for (int i = 0; i + W <= n; i++) {
        double wsum = 0.0;
        for (int j = i; j < i+W; j++) wsum += x[j];
        double wmean = wsum / W, wsq = 0.0;
        for (int j = i; j < i+W; j++) { double d = x[j]-wmean; wsq += d*d; }
        double wstd = sqrt(wsq / W);
        if (wstd > r.max_roll_std) r.max_roll_std = wstd;
    }

    double th = r.mean + k * r.std;            // (c) đếm peak
    r.count_peaks = 0;
    for (int i = 0; i < n; i++) if (x[i] > th) r.count_peaks++;
    return r;
}