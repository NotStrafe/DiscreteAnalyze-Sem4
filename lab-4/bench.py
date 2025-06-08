import itertools
import random
import string
import subprocess
import csv
import time
import os
import sys
random.seed(42)

N_LIST = [10**3, 10**4, 10**5]
M_LIST = [5, 50, 500]
ALPH = string.ascii_lowercase


def word():
    return ''.join(random.choice(ALPH) for _ in range(random.randint(1, 16)))


def gen_case(n, m, fname):
    text = [' '.join(word() for _ in range(n))]
    pat = ' '.join(word() for _ in range(m))
    with open(fname, 'w') as f:
        f.write(pat + '\n')
        f.write('\n'.join(text))


def run(cmd, infile):
    t0 = time.perf_counter()
    subprocess.run(cmd, stdin=open(infile), stdout=subprocess.DEVNULL)
    return (time.perf_counter() - t0) * 1000  # мс


rows = []
for n, m in itertools.product(N_LIST, M_LIST):
    name = f'case_{n}_{m}.txt'
    gen_case(n, m, name)
    naive = run(['./naive'], name)
    kmp = run(['./kmp'], name)
    rows.append([n, m, f'{naive:.1f}', f'{kmp:.1f}'])
    print(f'N={n:>6}, M={m:>3} → naive {naive:6.1f}  ms ,  kmp {kmp:6.1f}  ms')

with open('result.csv', 'w', newline='') as f:
    csv.writer(f).writerows([['N', 'M', 'naive_ms', 'kmp_ms']] + rows)
print('\nCSV сохранён в result.csv')
