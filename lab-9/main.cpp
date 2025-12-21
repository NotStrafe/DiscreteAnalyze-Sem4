#include <iostream>
#include <limits>
#include <queue>
#include <vector>

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, m, start, finish;
  if (!(std::cin >> n >> m >> start >> finish)) return 0;

  struct Edge {
    int u;
    int v;
    long long w;
  };
  std::vector<Edge> edges;
  edges.reserve(m);
  std::vector<std::vector<int>> adj(n + 1);
  for (int i = 0; i < m; ++i) {
    int u, v;
    long long w;
    std::cin >> u >> v >> w;
    edges.push_back({u, v, w});
    adj[u].push_back(i);
  }

  const long long INF = (1LL << 62);
  std::vector<long long> dist(n + 1, INF);
  dist[start] = 0;

  std::vector<char> reachable(n + 1, false);
  std::queue<int> q;
  reachable[start] = true;
  q.push(start);
  int reachable_count = 0;
  while (!q.empty()) {
    int v = q.front();
    q.pop();
    ++reachable_count;
    for (int idx : adj[v]) {
      const auto& e = edges[idx];
      int to = e.v;
      if (!reachable[to]) {
        reachable[to] = true;
        q.push(to);
      }
    }
  }
  if (!reachable[finish]) {
    std::cout << "No solution";
    return 0;
  }

  std::vector<long long> next = dist;
  std::vector<int> frontier = {start};
  std::vector<int> mark(n + 1, 0);
  int stamp = 1;
  mark[start] = stamp;

  int iterations = reachable_count - 1;
  for (int i = 1; i <= iterations; ++i) {
    next = dist;
    std::vector<int> next_frontier;
    ++stamp;

    for (int v : frontier) {
      for (int idx : adj[v]) {
        const auto& e = edges[idx];
        int to = e.v;
        if (dist[v] != INF && dist[v] + e.w < next[to]) {
          next[to] = dist[v] + e.w;
          if (mark[to] != stamp) {
            mark[to] = stamp;
            next_frontier.push_back(to);
          }
        }
      }
    }

    dist.swap(next);
    frontier.swap(next_frontier);
    if (frontier.empty()) break;
  }

  if (dist[finish] == INF) {
    std::cout << "No solution";
  } else {
    std::cout << dist[finish];
  }
  return 0;
}
