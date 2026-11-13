#!/usr/bin/env python3
"""Побудова графіків положення та швидкості з CSV-файлів вузла delay_sim.

Використання:
  ros2 run lab3_control plot_results.py out.png no_delay.csv:"Без затримок" delay.csv:"Затримка 150 мс"
"""
import csv
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa: E402


def load(path):
    with open(path) as f:
        rows = list(csv.DictReader(f))
    return {k: [float(r[k]) for r in rows] for k in rows[0]}


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    out = sys.argv[1]
    target = 0.05
    fig, (ax_x, ax_v) = plt.subplots(2, 1, figsize=(10, 7), sharex=True)
    for spec in sys.argv[2:]:
        path, _, label = spec.partition(':')
        d = load(path)
        ax_x.plot(d['t'], d['x'], label=label or path)
        ax_v.plot(d['t'], d['v'], label=label or path)
    ax_x.axhline(target, color='k', linestyle='--', linewidth=0.8, label='x_target = 0.05 рад')
    ax_x.set_ylabel('Положення x, рад')
    ax_v.set_ylabel('Швидкість v, рад/с')
    ax_v.set_xlabel('Час, с')
    for ax in (ax_x, ax_v):
        ax.grid(True, alpha=0.3)
        ax.legend(loc='upper right', fontsize=9)
    fig.tight_layout()
    fig.savefig(out, dpi=130)
    print(f'Графік збережено: {out}')


if __name__ == '__main__':
    main()
