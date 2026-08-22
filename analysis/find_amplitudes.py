"""
KHÂU 1 — Trích tham số triều từ dữ liệu thật
Đọc 6 trạm, dùng phân tích điều hòa (curve_fit) tìm:
  base, biên độ 4 sóng (M2,S2,K1,O1), pha 4 sóng, sigma nhiễu
Xuất ra: results/tide_params.csv
"""

import pandas as pd
import numpy as np
from scipy.optimize import curve_fit

# ===== CẤU HÌNH =====
FILE = "data/Hg_5_stations_1984-2020.xlsx"   # sửa cho khớp tên file thật
P = [12.4206, 12.0, 23.9345, 25.8193]         # chu kỳ M2,S2,K1,O1 (giờ) — hằng số thiên văn
NAMES = ["M2", "S2", "K1", "O1"]

# ===== MODEL: nền + 4 sóng (mỗi sóng 1 cặp cos/sin) =====
def model(t, c, a1, b1, a2, b2, a3, b3, a4, b4):
    return (c
            + a1*np.cos(2*np.pi*t/P[0]) + b1*np.sin(2*np.pi*t/P[0])
            + a2*np.cos(2*np.pi*t/P[1]) + b2*np.sin(2*np.pi*t/P[1])
            + a3*np.cos(2*np.pi*t/P[2]) + b3*np.sin(2*np.pi*t/P[2])
            + a4*np.cos(2*np.pi*t/P[3]) + b4*np.sin(2*np.pi*t/P[3]))

# ===== XỬ LÝ TỪNG TRẠM =====
sheets = pd.ExcelFile(FILE).sheet_names
rows = []

for sheet in sheets:
    df = pd.read_excel(FILE, sheet_name=sheet, header=None, names=["t", "v"])
    v = df["v"].values
    t = np.arange(len(v), dtype=float)

    # tìm 9 hệ số
    popt, _ = curve_fit(model, t, v)

    # ghép biên độ A = sqrt(a^2+b^2) và pha phi = atan2(b,a) cho từng sóng
    amps, phases = [], []
    for i in range(4):
        a = popt[1 + 2*i]
        b = popt[2 + 2*i]
        amps.append(np.hypot(a, b))
        phases.append(np.arctan2(b, a))

    # sigma nhiễu = độ lệch của phần dư giữa các giờ liên tiếp
    residual = v - model(t, *popt)
    sigma = np.diff(residual).std()

    rows.append({
        "station":   sheet,
        "n_records": len(v),
        "base":  round(popt[0], 4),
        "M2":    round(amps[0], 4),   "M2_phase": round(phases[0], 4),
        "S2":    round(amps[1], 4),   "S2_phase": round(phases[1], 4),
        "K1":    round(amps[2], 4),   "K1_phase": round(phases[2], 4),
        "O1":    round(amps[3], 4),   "O1_phase": round(phases[3], 4),
        "sigma": round(sigma, 4),
    })
    print(f"xong {sheet}")

# ===== XUẤT CSV + IN BẢNG =====
out = pd.DataFrame(rows)
out.to_csv("results/tide_params.csv", index=False)

print()
# in gọn: chỉ biên độ và sigma cho dễ đọc
cols_show = ["station", "base", "M2", "S2", "K1", "O1", "sigma"]
print(out[cols_show].to_string(index=False))
print("\nĐã lưu results/tide_params.csv (kèm cả pha 4 sóng)")