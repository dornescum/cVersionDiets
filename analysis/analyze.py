#!/usr/bin/env python3
"""
Benchmark Results Analyzer

Parses k6 JSON output files and generates summary statistics and charts.

Usage:
    python analyze.py                          # Analyze latest results
    python analyze.py --run 20260102_164318    # Analyze specific run
    python analyze.py --compare                # Compare all runs
"""

import argparse
import json
import statistics
from pathlib import Path
from datetime import datetime

import pandas as pd
import matplotlib.pyplot as plt

# Project paths
PROJECT_ROOT = Path(__file__).parent.parent
RESULTS_DIR = PROJECT_ROOT / "benchmarks" / "results"
CHARTS_DIR = Path(__file__).parent / "charts"


def parse_k6_json(file_path: Path) -> dict:
    """Parse k6 NDJSON output file and extract metrics."""
    latencies = []
    request_count = 0
    error_count = 0

    with open(file_path, "r") as f:
        for line in f:
            try:
                entry = json.loads(line.strip())

                # Count requests
                if entry.get("metric") == "http_reqs" and entry.get("type") == "Point":
                    request_count += 1

                # Collect latency values
                if entry.get("metric") == "http_req_duration" and entry.get("type") == "Point":
                    latencies.append(entry["data"]["value"])

                # Count errors
                if entry.get("metric") == "http_req_failed" and entry.get("type") == "Point":
                    if entry["data"]["value"] == 1:
                        error_count += 1

            except json.JSONDecodeError:
                continue

    if not latencies:
        return None

    sorted_latencies = sorted(latencies)
    n = len(sorted_latencies)

    return {
        "request_count": request_count,
        "error_count": error_count,
        "error_rate": error_count / request_count if request_count > 0 else 0,
        "avg_ms": statistics.mean(latencies),
        "min_ms": min(latencies),
        "max_ms": max(latencies),
        "p50_ms": sorted_latencies[int(n * 0.50)],
        "p90_ms": sorted_latencies[int(n * 0.90)],
        "p95_ms": sorted_latencies[int(n * 0.95)],
        "p99_ms": sorted_latencies[min(int(n * 0.99), n - 1)],
    }


def get_latest_run() -> Path:
    """Get the most recent benchmark run directory."""
    runs = sorted(RESULTS_DIR.iterdir(), reverse=True)
    return runs[0] if runs else None


def analyze_run(run_dir: Path) -> pd.DataFrame:
    """Analyze all test results in a benchmark run."""
    results = []

    for json_file in run_dir.glob("*.json"):
        test_name = json_file.stem  # e.g., "categories", "foods-list"
        metrics = parse_k6_json(json_file)

        if metrics:
            results.append({
                "test": test_name,
                **metrics
            })

    return pd.DataFrame(results)


def print_summary(df: pd.DataFrame, run_name: str):
    """Print formatted summary table."""
    print(f"\n{'='*70}")
    print(f"  Benchmark Results: {run_name}")
    print(f"{'='*70}\n")

    # Format the dataframe for display
    display_df = df[["test", "request_count", "avg_ms", "p50_ms", "p95_ms", "p99_ms", "error_rate"]].copy()
    display_df.columns = ["Test", "Requests", "Avg (ms)", "P50 (ms)", "P95 (ms)", "P99 (ms)", "Errors"]

    # Round numeric columns
    for col in ["Avg (ms)", "P50 (ms)", "P95 (ms)", "P99 (ms)"]:
        display_df[col] = display_df[col].round(2)
    display_df["Errors"] = (display_df["Errors"] * 100).round(2).astype(str) + "%"

    print(display_df.to_string(index=False))
    print()


def generate_latency_chart(df: pd.DataFrame, run_name: str, output_path: Path):
    """Generate latency comparison bar chart."""
    fig, ax = plt.subplots(figsize=(10, 6))

    tests = df["test"].tolist()
    x = range(len(tests))
    width = 0.25

    ax.bar([i - width for i in x], df["p50_ms"], width, label="P50", color="#2ecc71")
    ax.bar([i for i in x], df["p95_ms"], width, label="P95", color="#f39c12")
    ax.bar([i + width for i in x], df["p99_ms"], width, label="P99", color="#e74c3c")

    ax.set_xlabel("Endpoint")
    ax.set_ylabel("Latency (ms)")
    ax.set_title(f"Response Latency by Endpoint - {run_name}")
    ax.set_xticks(x)
    ax.set_xticklabels(tests, rotation=45, ha="right")
    ax.legend()
    ax.grid(axis="y", alpha=0.3)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    plt.close()
    print(f"Chart saved: {output_path}")


def generate_requests_chart(df: pd.DataFrame, run_name: str, output_path: Path):
    """Generate requests count bar chart."""
    fig, ax = plt.subplots(figsize=(10, 6))

    colors = ["#3498db", "#9b59b6", "#1abc9c", "#e74c3c"]
    ax.bar(df["test"], df["request_count"], color=colors[:len(df)])

    ax.set_xlabel("Endpoint")
    ax.set_ylabel("Total Requests")
    ax.set_title(f"Request Count by Endpoint - {run_name}")
    ax.tick_params(axis="x", rotation=45)
    ax.grid(axis="y", alpha=0.3)

    # Add value labels on bars
    for i, v in enumerate(df["request_count"]):
        ax.text(i, v + 50, str(v), ha="center", fontsize=9)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    plt.close()
    print(f"Chart saved: {output_path}")


def compare_runs(run_dirs: list[Path]) -> pd.DataFrame:
    """Compare metrics across multiple runs."""
    all_results = []

    for run_dir in run_dirs:
        run_name = run_dir.name
        df = analyze_run(run_dir)
        df["run"] = run_name
        all_results.append(df)

    return pd.concat(all_results, ignore_index=True)


def export_markdown(df: pd.DataFrame, run_name: str, output_path: Path):
    """Export results as markdown table."""
    md = f"# Benchmark Results: {run_name}\n\n"
    md += f"Generated: {datetime.now().isoformat()}\n\n"
    md += "## Summary\n\n"
    md += "| Test | Requests | Avg (ms) | P50 (ms) | P95 (ms) | P99 (ms) | Errors |\n"
    md += "|------|----------|----------|----------|----------|----------|--------|\n"

    for _, row in df.iterrows():
        md += f"| {row['test']} | {row['request_count']} | {row['avg_ms']:.2f} | "
        md += f"{row['p50_ms']:.2f} | {row['p95_ms']:.2f} | {row['p99_ms']:.2f} | "
        md += f"{row['error_rate']*100:.2f}% |\n"

    with open(output_path, "w") as f:
        f.write(md)
    print(f"Markdown saved: {output_path}")


def main():
    parser = argparse.ArgumentParser(description="Analyze k6 benchmark results")
    parser.add_argument("--run", help="Specific run directory name (e.g., 20260102_164318)")
    parser.add_argument("--compare", action="store_true", help="Compare all runs")
    parser.add_argument("--no-charts", action="store_true", help="Skip chart generation")
    args = parser.parse_args()

    CHARTS_DIR.mkdir(parents=True, exist_ok=True)

    if args.compare:
        # Compare all runs
        run_dirs = sorted(RESULTS_DIR.iterdir())
        if len(run_dirs) < 2:
            print("Need at least 2 runs to compare")
            return

        df = compare_runs(run_dirs)
        print("\nComparison of all runs:")
        print(df.to_string(index=False))

    else:
        # Analyze single run
        if args.run:
            run_dir = RESULTS_DIR / args.run
            if not run_dir.exists():
                print(f"Run not found: {run_dir}")
                return
        else:
            run_dir = get_latest_run()
            if not run_dir:
                print("No benchmark runs found")
                return

        run_name = run_dir.name
        df = analyze_run(run_dir)

        if df.empty:
            print(f"No valid results in {run_dir}")
            return

        # Print summary
        print_summary(df, run_name)

        # Generate charts
        if not args.no_charts:
            generate_latency_chart(df, run_name, CHARTS_DIR / f"latency_{run_name}.png")
            generate_requests_chart(df, run_name, CHARTS_DIR / f"requests_{run_name}.png")

        # Export markdown
        export_markdown(df, run_name, CHARTS_DIR / f"results_{run_name}.md")


if __name__ == "__main__":
    main()
