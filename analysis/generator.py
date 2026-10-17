"""
Bộ sinh dữ liệu thủy văn tổng hợp — dùng cho benchmark.
Sinh chuỗi cho MỘT trạm, ở độ phân giải tùy ý (giờ/phút/giây).
"""
import numpy as np
import pandas as pd

P = [12.4206, 12.0, 23.9345, 25.8193]     # chu kỳ M2,S2,K1,O1 (giờ)

def load_params(csv="results/tide_params.csv"):
    """Đọc bảng tham số 6 trạm."""
    return pd.read_csv(csv)

def generate_station(row, n_points, dt=1.0, seed=42, noise_scale=0.6):
    """
    Sinh chuỗi mực nước cho 1 trạm.
      row         : một dòng tham số (từ load_params)
      n_points    : số điểm cần sinh
      dt          : bước thời gian tính bằng GIỜ
                    dt=1     -> mỗi giờ
                    dt=1/60  -> mỗi phút
                    dt=1/3600-> mỗi giây
      noise_scale : hệ số điều chỉnh nhiễu (0.6 : đã thử qua 0.9;0.8;0.7 => chọn 0.6)
    Trả về: mảng numpy n_points giá trị.
    """
    base   = row["base"]
    amps   = [row["M2"], row["S2"], row["K1"], row["O1"]]
    phases = [row["M2_phase"], row["S2_phase"], row["K1_phase"], row["O1_phase"]]
    sigma  = row["sigma"] * noise_scale * np.sqrt(dt)   # chia nhiễu theo bước thời gian

    rng = np.random.default_rng(seed)
    values = np.zeros(n_points)
    noise = 0.0

    for i in range(n_points):
        t = i * dt
        tide = sum(amps[k] * np.cos(2*np.pi*t/P[k] - phases[k]) for k in range(4))
        noise = 0.7*noise + rng.normal(0, sigma)
        values[i] = base + tide + noise

    return values

# ===== TEST NHANH =====
# if __name__ == "__main__":
#     params = load_params()
#     row = params[params["station"] == "Phú An"].iloc[0]

#     # sinh 1 giờ dữ liệu ở mức GIÂY (3600 điểm)
#     data = generate_station(row, n_points=86400, dt=1/3600) # 1 ngày = 86400
#     print(f"Sinh {len(data)} điểm (mức giây)")
#     print(f"  trung bình = {data.mean():.3f}")
#     print(f"  độ lệch    = {data.std():.3f}")
#     print(f"  min/max    = {data.min():.3f} / {data.max():.3f}")



#sinh file csv để kiểm chức - không dùng trong thực tế đề tài vì nghẽn RAM
# ===== SINH, LƯU FILE, VẼ ĐỒ THỊ =====
if __name__ == "__main__":
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    params = load_params()
    row = params[params["station"] == "Phú An"].iloc[0]

    # --- chọn cấu hình sinh ---
    n_points = 86400      # số điểm
    dt = 1/3600           # bước = 1 giây (tính theo giờ)

    data = generate_station(row, n_points=n_points, dt=dt)

    # --- LƯU RA CSV ---
    times = np.arange(n_points) * dt        # cột thời gian (giờ)
    df = pd.DataFrame({"hour": times, "value": data})
    df.to_csv("results/synthetic_phuan.csv", index=False)
    print(f"Đã lưu results/synthetic_phuan.csv ({n_points} dòng)")

    # --- VẼ ĐỒ THỊ ---
    fig, ax = plt.subplots(figsize=(12, 4))
    ax.plot(times, data, lw=0.5, color="tab:blue")
    ax.set_title(f"Dữ liệu sinh mức giây — Phú An ({n_points} điểm = 1 ngày)")
    ax.set_xlabel("Giờ")
    ax.set_ylabel("Mực nước (m)")
    ax.grid(alpha=0.3)
    plt.tight_layout()
    plt.savefig("results/figures/synthetic_phuan.png", dpi=120)
    print("Đã lưu results/figures/synthetic_phuan.png")

    # --- thống kê ---
    print(f"\ntrung bình = {data.mean():.3f}")
    print(f"độ lệch    = {data.std():.3f}")
    print(f"min/max    = {data.min():.3f} / {data.max():.3f}")