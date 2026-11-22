#!/usr/bin/env python3
"""
Generates results/page_faults_graph.png from results/locality_results.csv
and results/random_results.csv using matplotlib.
"""
import csv
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def load_csv(path):
    frames, fifo, lru, clock, opt = [], [], [], [], []
    with open(path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            frames.append(int(row["Frames"]))
            fifo.append(int(row["FIFO"]))
            lru.append(int(row["LRU"]))
            clock.append(int(row["Clock"]))
            opt.append(int(row["Optimal"]))
    return frames, fifo, lru, clock, opt


def main():
    loc_path = os.path.join("results", "locality_results.csv")
    rnd_path = os.path.join("results", "random_results.csv")
    out_path = os.path.join("results", "page_faults_graph.png")

    lf, l_fifo, l_lru, l_clock, l_opt = load_csv(loc_path)
    rf, r_fifo, r_lru, r_clock, r_opt = load_csv(rnd_path)

    fig, axes = plt.subplots(1, 2, figsize=(10.0, 4.2), dpi=300)

    series_styles = [
        ("FIFO",    "s", "--", "#444444"),
        ("LRU",     "o", "-",  "#000000"),
        ("Clock",   "^", "-.", "#666666"),
        ("Optimal", "D", ":",  "#111111"),
    ]

    datasets = [
        (axes[0], "Strong-Locality Workload (10,000 Refs)", lf, [l_fifo, l_lru, l_clock, l_opt]),
        (axes[1], "Random-Access Workload (10,000 Refs)",   rf, [r_fifo, r_lru, r_clock, r_opt]),
    ]

    for ax, title, xvals, ylists in datasets:
        for (label, marker, ls, color), yvals in zip(series_styles, ylists):
            ax.plot(xvals, yvals, label=label, marker=marker, linestyle=ls,
                    color=color, linewidth=1.5, markersize=5.5)
        ax.set_title(title, fontsize=10, fontweight="bold", fontfamily="serif")
        ax.set_xlabel("Number of Physical Frames", fontsize=9, fontfamily="serif")
        ax.set_ylabel("Total Page Faults", fontsize=9, fontfamily="serif")
        ax.set_xticks(xvals)
        ax.grid(True, linestyle=":", color="#AAAAAA", linewidth=0.7)
        ax.legend(fontsize=8.5, frameon=True, edgecolor="black")
        for tick in ax.get_xticklabels() + ax.get_yticklabels():
            tick.set_fontfamily("serif")
            tick.set_fontsize(8.5)

    plt.tight_layout(pad=1.0)
    plt.savefig(out_path, dpi=300, bbox_inches="tight", facecolor="white")
    plt.close()
    print(f"Generated graph: {out_path}")


if __name__ == "__main__":
    main()
