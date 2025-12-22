#include <iostream>
#include <limits>
#include <vector>

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, m, start, finish;
  if (!(std::cin >> n >> m >> start >> finish)) return 0;
  --start;
  --finish;

  struct Edge {
    int from;
    int to;
    long long weight;
  };
  std::vector<Edge> edges;
  edges.reserve(m);
  for (int i = 0; i < m; ++i) {
    int u, v;
    long long w;
    std::cin >> u >> v >> w;
    u -= 1;
    v -= 1;
    edges.push_back({u, v, w});
  }

  const long long INF = std::numeric_limits<long long>::max();
  std::vector<long long> dist(n, INF);
  dist[start] = 0;

  for (int iter = 0; iter < n - 1; ++iter) {
    bool updated = false;
    for (const auto& e : edges) {
      if (dist[e.from] == INF) continue;
      long long candidate = dist[e.from] + e.weight;
      if (candidate < dist[e.to]) {
        dist[e.to] = candidate;
        updated = true;
      }
    }
    if (!updated) break;
  }

  if (dist[finish] == INF) {
    std::cout << "No solution\n";
  } else {
    std::cout << dist[finish] << '\n';
  }
  return 0;
}
