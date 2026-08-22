"""Tạo bộ dữ liệu benchmark: N trạm tổng hợp, mỗi trạm 1 chuỗi cùng độ dài."""
import argparse, time
import numpy as np
from generator import load_params, generate_station

RES = {"second": 1/3600, "minute": 1/60, "hour": 1.0}  # dt tính bằng GIỜ, generator : dt được tính bằng giừo 

def make_dataset(n_stations, series_len, dt, out_path):
    params = load_params()          # 6 trạm thật
    n_real = len(params)
    t0 = time.time()
    with open(out_path, "w") as f:
        f.write("station_id,value\n")
        for s in range(n_stations):
            row = params.iloc[s % n_real]           # xoay vòng 6 bộ tham số 
            values = generate_station(row, n_points=series_len, dt=dt, seed=s) # có seed để tránh trùng lập dữ liệu nếu trạm được sinh ra cùng 1 trạm gốc 
            block = np.column_stack((np.full(series_len, s), values))
            np.savetxt(f, block, fmt=["%d", "%.4f"], delimiter=",")  # ghi rồi bỏ, không giữ hết trong RAM
    print(f"{out_path}: {n_stations} tram x {series_len} diem = "
          f"{n_stations*series_len:,} dong | {time.time()-t0:.1f}s")

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--stations", type=int, required=True)
    ap.add_argument("--len", type=int, default=86400, help="so diem moi tram")
    ap.add_argument("--res", choices=list(RES), default="second")
    ap.add_argument("-o", "--out", required=True)
    a = ap.parse_args()
    make_dataset(a.stations, a.len, RES[a.res], a.out)