#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct SuffixTree {
  struct Node {
    std::unordered_map<char, int> next;
    int start = -1;
    int* end = nullptr;
    int link = -1;
    int parent = -1;
    int depth = 0;
  };

  std::string s;
  std::vector<Node> st;
  int root = 0;
  int active_node = 0, active_edge = -1, active_length = 0;
  int remaining = 0, last_new_node = -1, leaf_end = -1;
  std::vector<int> fixed_ends;

  explicit SuffixTree(const std::string& str) {
    build(str);
    finalize_depths();
  }

  int edge_length(int v) const {
    return (v == root) ? 0 : (*(st[v].end) - st[v].start + 1);
  }

  int new_node(int start, int* end, int parent) {
    Node v;
    v.start = start;
    v.end = end;
    v.link = -1;
    v.parent = parent;
    v.depth = 0;
    st.push_back(v);
    return (int)st.size() - 1;
  }

  bool walk_down(int v) {
    int el = edge_length(v);
    if (active_length >= el) {
      active_edge += el;
      active_length -= el;
      active_node = v;
      return true;
    }
    return false;
  }

  void build(const std::string& str) {
    s = str;
    st.reserve(2 * (int)s.size() + 5);
    fixed_ends.reserve(2 * (int)s.size() + 5);
    int* root_end = new int(-1);
    st.push_back(Node{});
    root = 0;
    st[0].start = -1;
    st[0].end = root_end;
    st[0].link = -1;
    st[0].parent = -1;
    st[0].depth = 0;
    active_node = root;
    active_edge = -1;
    active_length = 0;
    remaining = 0;
    last_new_node = -1;
    leaf_end = -1;
    for (int pos = 0; pos < (int)s.size(); ++pos) extend(pos);
  }

  void extend(int pos) {
    leaf_end = pos;
    last_new_node = -1;
    ++remaining;
    while (remaining > 0) {
      if (active_length == 0) active_edge = pos;
      char c = s[active_edge];
      auto it = st[active_node].next.find(c);
      if (it == st[active_node].next.end()) {
        int leaf = new_node(pos, &leaf_end, active_node);
        st[active_node].next[c] = leaf;
        if (last_new_node != -1) {
          st[last_new_node].link = active_node;
          last_new_node = -1;
        }
      } else {
        int nxt = it->second;
        if (walk_down(nxt)) continue;
        if (s[st[nxt].start + active_length] == s[pos]) {
          ++active_length;
          if (last_new_node != -1) {
            st[last_new_node].link = active_node;
            last_new_node = -1;
          }
          break;
        }
        fixed_ends.push_back(st[nxt].start + active_length - 1);
        int* split_end = &fixed_ends.back();
        int split = new_node(st[nxt].start, split_end, active_node);
        st[active_node].next[c] = split;

        st[nxt].start += active_length;
        st[nxt].parent = split;
        st[split].next[s[st[nxt].start]] = nxt;

        int leaf = new_node(pos, &leaf_end, split);
        st[split].next[s[pos]] = leaf;

        if (last_new_node != -1) st[last_new_node].link = split;
        last_new_node = split;
      }
      --remaining;
      if (active_node == root && active_length > 0) {
        --active_length;
        active_edge = pos - remaining + 1;
      } else {
        active_node =
            (st[active_node].link == -1) ? root : st[active_node].link;
      }
    }
  }

  void finalize_depths() {
    std::vector<int> stack;
    stack.push_back(root);
    st[root].depth = 0;
    while (!stack.empty()) {
      int v = stack.back();
      stack.pop_back();
      for (auto& kv : st[v].next) {
        int u = kv.second;
        st[u].depth = st[v].depth + edge_length(u);
        stack.push_back(u);
      }
    }
  }

  int child_of(int v, char c) const {
    auto it = st[v].next.find(c);
    return (it == st[v].next.end()) ? -1 : it->second;
  }

  struct Pos {
    int node, child, off;
  };

  bool advance(Pos& p, char c) const {
    if (p.child == -1) {
      int nxt = child_of(p.node, c);
      if (nxt == -1) return false;
      p.child = nxt;
      p.off = 1;
      if (p.off == edge_length(p.child)) {
        p.node = p.child;
        p.child = -1;
        p.off = 0;
      }
      return true;
    } else {
      if (s[st[p.child].start + p.off] != c) return false;
      ++p.off;
      if (p.off == edge_length(p.child)) {
        p.node = p.child;
        p.child = -1;
        p.off = 0;
      }
      return true;
    }
  }

  Pos descend(int v, int l, int r) const {
    Pos p{v, -1, 0};
    if (l > r) return p;
    int pos = l;
    while (true) {
      int nxt = child_of(p.node, s[pos]);
      int el = edge_length(nxt);
      if (pos + el - 1 <= r) {
        p.node = nxt;
        pos += el;
        if (pos > r) {
          p.child = -1;
          p.off = 0;
          return p;
        }
      } else {
        p.child = nxt;
        p.off = r - pos + 1;
        return p;
      }
    }
  }

  int pos_length(const Pos& p) const {
    if (p.child == -1) return st[p.node].depth;
    return st[st[p.child].parent].depth + p.off;
  }

  void normalize_to_edge(Pos& p) const {
    if (p.child == -1 && p.node != root) {
      int child = p.node;
      p.node = st[child].parent;
      p.child = child;
      p.off = edge_length(child);
    }
  }

  void retreat_one(Pos& p, int& l) const {
    normalize_to_edge(p);
    if (p.child == -1) {
      if (p.node == root) {
        l = 0;
        return;
      }
      int v = p.node;
      int linkv = (st[v].link == -1 ? root : st[v].link);
      p = Pos{linkv, -1, 0};
      l = st[linkv].depth;
      return;
    } else {
      int child = p.child;
      int parent = st[child].parent;
      int u = (parent == root)
                  ? root
                  : (st[parent].link == -1 ? root : st[parent].link);
      int lpos = st[child].start + 1;
      int rpos = st[child].start + p.off - 1;
      if (lpos > rpos) {
        p = Pos{u, -1, 0};
        l = st[u].depth;
      } else {
        p = descend(u, lpos, rpos);
        l = pos_length(p);
      }
    }
  }

  std::vector<int> compute_lend_over_T(const std::string& T) const {
    int n = (int)T.size();
    std::vector<int> lend(n, 0);
    Pos p{root, -1, 0};
    int l = 0;
    for (int j = 0; j < n; ++j) {
      char c = T[j];
      while (true) {
        if (advance(p, c)) {
          ++l;
          break;
        }
        if (p.child == -1 && p.node == root) {
          l = 0;
          break;
        }
        retreat_one(p, l);
        if (p.child == -1 && p.node == root && child_of(p.node, c) == -1) {
          l = 0;
          break;
        }
      }
      lend[j] = l;
    }
    return lend;
  }
};

static inline void strip_cr(std::string& s) {
  if (!s.empty() && s.back() == '\r') s.pop_back();
}
static inline void strip_bom(std::string& s) {
  if (s.size() >= 3 && (unsigned char)s[0] == 0xEF &&
      (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF)
    s.erase(0, 3);
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string P, T;
  if (!std::getline(std::cin, P)) return 0;
  std::getline(std::cin, T);
  strip_cr(P);
  strip_cr(T);
  strip_bom(P);
  if (P.empty() || T.empty() || P.size() > T.size()) return 0;

  char term = '\1';
  if (P.find(term) != std::string::npos || T.find(term) != std::string::npos)
    term = '\2';
  std::string S = P;
  S.push_back(term);

  SuffixTree tree(S);

  std::vector<int> lend = tree.compute_lend_over_T(T);

  int n = (int)T.size(), m = (int)P.size();
  std::vector<int> best(n, 0);
  for (int j = 0; j < n; ++j) {
    int L = lend[j];
    if (L > 0) {
      int s = j - L + 1;
      if (best[s] < L) best[s] = L;
    }
  }
  int cur = 0;
  for (int i = 0; i < n; ++i) {
    if (cur > 0) --cur;
    if (best[i] > cur) cur = best[i];
    if (i + m <= n && cur >= m) std::cout << (i + 1) << '\n';
  }
  return 0;
}
