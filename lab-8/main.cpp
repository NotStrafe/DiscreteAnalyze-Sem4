#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct Seg {
  long long L, R;
  int id;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int N;
  if (!(cin >> N)) return 0;

  vector<Seg> a;
  a.reserve(N);
  for (int i = 0; i < N; ++i) {
    long long L, R;
    cin >> L >> R;
    a.push_back({L, R, i});
  }

  long long M;
  cin >> M;

  sort(a.begin(), a.end(), [](const Seg& x, const Seg& y) {
    if (x.L != y.L) return x.L < y.L;
    return x.R > y.R;
  });

  long long cur = 0;
  int i = 0, n = (int)a.size();
  vector<Seg> chosen;

  while (cur < M) {
    long long bestR = cur;
    int bestIdx = -1;
    while (i < n && a[i].L <= cur) {
      if (a[i].R > bestR) {
        bestR = a[i].R;
        bestIdx = i;
      }
      ++i;
    }
    if (bestIdx == -1) {
      cout << 0 << '\n';
      return 0;
    }
    chosen.push_back(a[bestIdx]);
    cur = bestR;
  }

  sort(chosen.begin(), chosen.end(),
       [](const Seg& x, const Seg& y) { return x.id < y.id; });

  cout << chosen.size() << '\n';
  for (const auto& s : chosen) {
    cout << s.L << ' ' << s.R << '\n';
  }
  return 0;
}
