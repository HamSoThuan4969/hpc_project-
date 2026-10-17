import pandas as pd
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# ===== 1. ĐỌC THAM SỐ (giờ có thêm pha) =====
params = pd.read_csv("results/tide_params.csv")
row = params[params["station"] == "Phú An"].iloc[0]

base   = row["base"]
amps   = [row["M2"], row["S2"], row["K1"], row["O1"]]
phases = [row["M2_phase"], row["S2_phase"], row["K1_phase"], row["O1_phase"]]  # ← MỚI
sigma  = row["sigma"]
P      = [12.4206, 12.0, 23.9345, 25.8193]

N = 1000

# ===== 2. SINH DỮ LIỆU =====
np.random.seed(42)
values = np.zeros(N)
noise = 0.0

for i in range(N):
    t = float(i)

    # tầng triều — pha: cos(2πt/P - phase) # được tính như này để không lêhcj pha
    tide = (amps[0]*np.cos(2*np.pi*t/P[0] - phases[0])
          + amps[1]*np.cos(2*np.pi*t/P[1] - phases[1])
          + amps[2]*np.cos(2*np.pi*t/P[2] - phases[2])
          + amps[3]*np.cos(2*np.pi*t/P[3] - phases[3]))

    # tầng nhiễu
    noise = 0.6*noise + np.random.normal(0, sigma)

    values[i] = base + tide + noise

# ===== 3. ĐỌC DỮ LIỆU THẬT =====
df = pd.read_excel("data/Hg_5_stations_1984-2020.xlsx",
                   sheet_name="Phú An", header=None, names=["t", "v"])
real = df["v"].values[:N]

# ===== 4. SO SÁNH THỐNG KÊ =====
print(f"{'':6s} {'trung bình':>12s} {'độ lệch':>10s} {'min':>8s} {'max':>8s}")
print(f"{'THẬT':6s} {real.mean():12.3f} {real.std():10.3f} {real.min():8.3f} {real.max():8.3f}")
print(f"{'SINH':6s} {values.mean():12.3f} {values.std():10.3f} {values.min():8.3f} {values.max():8.3f}")

# ===== 5. VẼ ĐỒ THỊ =====
fig, ax = plt.subplots(2, 1, figsize=(12, 6), sharey=True)
ax[0].plot(real[:200], color="black", lw=1)
ax[0].set_title("Dữ liệu THẬT — Phú An (200 giờ đầu)")
ax[1].plot(values[:200], color="tab:blue", lw=1)
ax[1].set_title("Dữ liệu SINH (mô hình 3 tầng, có pha)")
for a in ax:
    a.set_ylabel("Mực nước (m)")
    a.grid(alpha=0.3)
ax[1].set_xlabel("Giờ")
plt.tight_layout()
plt.savefig("results/figures/compare_real_synth.png", dpi=120)
print("\nĐã lưu results/figures/compare_real_synth.png")