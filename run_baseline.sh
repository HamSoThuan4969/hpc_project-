#!/bin/bash
# Đo baseline tuần tự trên 5 quy mô, mỗi quy mô chạy 3 lần.
cd ~/hpc_project
echo "=== KET QUA BASELINE ==="
for N in 10 50 100 500 1000; do
    f="data/tmp_${N}st.csv"
    python3 analysis/make_dataset.py --stations $N --len 86400 --res second -o "$f" >/dev/null
    for run in 1 2 3; do
        ./seq "$f" | grep BENCHMARK
    done
    rm -f "$f"        # xóa ngay để khỏi đầy ổ
done