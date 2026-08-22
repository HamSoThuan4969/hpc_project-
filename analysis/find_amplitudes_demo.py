import pandas as pd
import numpy as np
from scipy.optimize import curve_fit

# ===== 1. ĐƯỜNG DẪN FILE  =====
FILE = "data/Hg_5_stations_1984-2020.xlsx"
SHEET = "Phú An"

# ===== 2. ĐỌC DỮ LIỆU =====
df = pd.read_excel(FILE, sheet_name=SHEET, header=None, names=["t", "v"])
v = df["v"].values
t = np.arange(len(v), dtype=float)
print(f"Đọc được {len(v)} dòng từ trạm {SHEET}")

# ===== 3. MODEL: nền + 4 sóng =====
P = [12.4206, 12.0, 23.9345, 25.8193]     # M2, S2, K1, O1 : Hằng số thiên văn
def model(t, c, a1, b1, a2, b2, a3, b3, a4, b4):
    return (c
            + a1*np.cos(2*np.pi*t/P[0]) + b1*np.sin(2*np.pi*t/P[0]) #M2
            + a2*np.cos(2*np.pi*t/P[1]) + b2*np.sin(2*np.pi*t/P[1]) #S2
            + a3*np.cos(2*np.pi*t/P[2]) + b3*np.sin(2*np.pi*t/P[2]) #K1
            + a4*np.cos(2*np.pi*t/P[3]) + b4*np.sin(2*np.pi*t/P[3]))    #O1

# ===== 4. TÌM BIÊN ĐỘ =====
popt, _ = curve_fit(model, t, v)

# ===== 5. IN KẾT QUẢ =====
print(f"nền  = {popt[0]:.4f} m")
for i, name in enumerate(["M2", "S2", "K1", "O1"]):
    A = np.hypot(popt[1+2*i], popt[2+2*i])
    print(f"{name}   = {A:.4f} m")


# ===== 6. TÍNH SIGMA NHIỄU (tầng 3) =====
du_doan = model(t, *popt)          # đường triều mô hình dự đoán
nhieu = v - du_doan                # phần thật trừ đi triều = nhiễu
sigma = np.diff(nhieu).std()       # độ lớn thay đổi nhiễu mỗi giờ
print(f"sigma nhiễu = {sigma:.4f} m/giờ")