#!/usr/bin/env python3
import argparse
import csv
import random
import statistics
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SRC = ROOT / "main.cpp"
BIN = ROOT / "main_bench"


@dataclass
class Case:
    text: str


def build_binary() -> None:
    cmd = ["g++", "-std=c++17", "-O2", str(SRC), "-o", str(BIN)]
    subprocess.run(cmd, check=True)


def run_binary(stdin_data: str) -> tuple[float, str]:
    t0 = time.perf_counter()
    proc = subprocess.run(
        [str(BIN)],
        input=stdin_data,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=True,
    )
    t1 = time.perf_counter()
    return t1 - t0, proc.stdout


def make_random_text(n: int, seed: int) -> str:
    rng = random.Random(seed)
    letters = "abcdefghijklmnopqrstuvwxyz"
    return "".join(rng.choice(letters) for _ in range(n))

def generate_cases(sizes: list[int]) -> list[Case]:
    return [Case(make_random_text(n, seed=2026 + n)) for n in sizes]


def benchmark_case(case: Case, repeats: int) -> dict:
    compress_times = []
    decompress_times = []
    compressed_output = ""

    # Warm-up run to reduce cold-start noise.
    _, warm_comp = run_binary(f"compress\n{case.text}\n")
    _, warm_decomp = run_binary(f"decompress\n{warm_comp}")
    if warm_decomp.rstrip("\n") != case.text:
        raise RuntimeError(f"Round-trip failed for warm-up case n={len(case.text)}")

    for _ in range(repeats):
        t_comp, comp_out = run_binary(f"compress\n{case.text}\n")
        compress_times.append(t_comp)
        compressed_output = comp_out

        t_decomp, decomp_out = run_binary(f"decompress\n{compressed_output}")
        decompress_times.append(t_decomp)

        if decomp_out.rstrip("\n") != case.text:
            raise RuntimeError(f"Round-trip failed for case n={len(case.text)}")

    return {
        "n": len(case.text),
        "comp_ms": statistics.mean(compress_times) * 1000.0,
        "decomp_ms": statistics.mean(decompress_times) * 1000.0,
    }


def print_report(rows: list[dict]) -> None:
    header = (
        f"{'n':>8} {'comp ms':>12} {'decomp ms':>12}"
    )
    print(header)
    print("-" * len(header))
    for r in rows:
        print(
            f"{r['n']:>8} {r['comp_ms']:>12.3f} {r['decomp_ms']:>12.3f}"
        )


def save_csv(rows: list[dict], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fields = [
        "n",
        "comp_ms",
        "decomp_ms",
    ]
    with out_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description="Benchmark for BWT+MTF+RLE solution")
    parser.add_argument(
        "--sizes",
        type=int,
        nargs="+",
        default=[1000, 10000, 50000, 100000],
        help="Input lengths to test",
    )
    parser.add_argument(
        "--repeats",
        type=int,
        default=3,
        help="Repeats per test case",
    )
    parser.add_argument(
        "--csv",
        type=Path,
        default=ROOT / "bench_results.csv",
        help="Path to save CSV report",
    )
    args = parser.parse_args()

    build_binary()
    cases = generate_cases(args.sizes)
    rows = [benchmark_case(case, args.repeats) for case in cases]
    print_report(rows)
    save_csv(rows, args.csv)
    print(f"\nCSV saved to: {args.csv}")


if __name__ == "__main__":
    main()
