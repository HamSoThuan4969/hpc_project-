"""So mean/std dữ liệu tổng hợp với dữ liệu thật."""
import sys
import numpy as np
import pandas as pd

REAL_FILE = "data/Hg_5_stations_1984-2020.xlsx"   # sửa nếu tên file thật khác

def real_stats():
    xls = pd.ExcelFile(REAL_FILE)
    out = []
    for sheet in xls.sheet_names:
        v = pd.read_excel(REAL_FILE, sheet_name=sheet, header=None,
                          names=["t", "v"])["v"].values
        v = v[~np.isnan(v)]                      # bỏ ô trống
        out.append((sheet, v.mean(), v.std()))
    return out                                   # [(tên, mean, std)] theo thứ tự 6 trạm

def synth_stats(csv):
    df = pd.read_csv(csv)
    g = df.groupby("station_id")["value"]
    return g.mean(), g.std()

if __name__ == "__main__":
    csv = sys.argv[1] if len(sys.argv) > 1 else "data/synth_10st_sec.csv"
    real = real_stats()
    smean, sstd = synth_stats(csv)
    print(f"{'tram':>6}{'synth_mean':>12}{'real_mean':>11}{'synth_std':>11}{'real_std':>10}")
    for sid in smean.index:
        rname, rmean, rstd = real[sid % len(real)]
        print(f"{sid:>6}{smean[sid]:>12.3f}{rmean:>11.3f}{sstd[sid]:>11.3f}{rstd:>10.3f}")