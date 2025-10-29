#include <algorithm>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, m;
  if (!(std::cin >> n >> m)) return 0;

  std::vector<std::vector<long long>> A(n, std::vector<long long>(m));
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j) std::cin >> A[i][j];

  const long long INF = std::numeric_limits<long long>::max() / 4;

  std::vector<std::vector<long long>> dp(n, std::vector<long long>(m, INF));
  std::vector<std::vector<int>> parent(n, std::vector<int>(m, -1));

  for (int j = 0; j < m; ++j) dp[0][j] = A[0][j];

  for (int i = 1; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      for (int dj = -1; dj <= 1; ++dj) {
        int pj = j + dj;
        if (pj < 0 || pj >= m) continue;
        long long cand = dp[i - 1][pj] + A[i][j];
        if (cand < dp[i][j]) {
          dp[i][j] = cand;
          parent[i][j] = pj;
        }
      }
    }
  }

  int best_j = 0;
  for (int j = 1; j < m; ++j)
    if (dp[n - 1][j] < dp[n - 1][best_j]) best_j = j;

  std::vector<std::pair<int, int>> path;
  int cur_j = best_j;
  for (int i = n - 1; i >= 0; --i) {
    path.emplace_back(i + 1, cur_j + 1);
    cur_j = (i > 0 ? parent[i][cur_j] : cur_j);
  }
  std::reverse(path.begin(), path.end());

  std::cout << dp[n - 1][best_j] << '\n';
  for (size_t k = 0; k < path.size(); ++k) {
    std::cout << '(' << path[k].first << ',' << path[k].second << ')';
    if (k + 1 != path.size()) std::cout << ' ';
  }
  std::cout << '\n';
  return 0;
}
