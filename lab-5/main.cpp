#include <cstring>
#include <iostream>
#include <string>
#include <vector>

struct SuffixTree {
  struct Node {
    std::vector<int> next;
    int start = -1;
    int* end = nullptr;
    int parent = -1;
  };

  std::string s;
  const char* sp = nullptr;
  std::vector<Node> st;
  int root = 0, active_node = 0, active_edge = -1, active_length = 0;
  int remaining = 0, last_new_node = -1, leaf_end = -1;

  int sigma = 0;
  int rank_[256];

  explicit SuffixTree(const std::string& str) { build(str); }

  inline int edge_length(int v) const {
    return (v == root) ? 0 : (*(st[v].end) - st[v].start + 1);
  }

  void init_alphabet() {
    bool used[256] = {false};
    for (unsigned char c : s) used[c] = true;
    sigma = 0;
    for (int i = 0; i < 256; ++i) rank_[i] = used[i] ? sigma++ : -1;
  }

  int new_node(int start, int* end, int parent) {
    Node v;
    v.start = start;
    v.end = end;
    v.parent = parent;
    v.next.assign(sigma, -1);
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
    sp = s.data();
    init_alphabet();

    st.reserve(2 * (int)s.size() + 5);
    int* root_end = new int(-1);
    root = new_node(-1, root_end, -1);

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
      int rc = rank_[(unsigned char)sp[active_edge]];
      int nxt = (rc < 0) ? -1 : st[active_node].next[rc];

      if (nxt == -1) {
        int leaf = new_node(pos, &leaf_end, active_node);
        if (rc >= 0) st[active_node].next[rc] = leaf;
        if (last_new_node != -1) {
          last_new_node = -1;
        }
      } else {
        if (walk_down(nxt)) continue;
        if (sp[st[nxt].start + active_length] == sp[pos]) {
          ++active_length;
          break;
        }
        int* split_end = new int(st[nxt].start + active_length - 1);
        int split = new_node(st[nxt].start, split_end, active_node);
        if (rc >= 0) st[active_node].next[rc] = split;

        st[nxt].start += active_length;
        st[nxt].parent = split;
        {
          int rc2 = rank_[(unsigned char)sp[st[nxt].start]];
          if (rc2 >= 0) st[split].next[rc2] = nxt;
        }

        int leaf = new_node(pos, &leaf_end, split);
        {
          int rpos = rank_[(unsigned char)sp[pos]];
          if (rpos >= 0) st[split].next[rpos] = leaf;
        }
      }

      --remaining;
      if (active_node == root && active_length > 0) {
        --active_length;
        active_edge = pos - remaining + 1;
      } else {
        active_node =
            (st[active_node].parent == -1) ? root : st[active_node].parent;
      }
    }
  }

  inline int child_of(int v, unsigned char c) const {
    int r = rank_[c];
    return (r < 0) ? -1 : st[v].next[r];
  }

  struct Pos {
    int node, child, off;
  };

  int advance_block(Pos& p, const char* tp, int rem) const {
    if (p.child == -1) return 0;
    int child = p.child;
    int el = edge_length(child);
    int avail = el - p.off;
    if (avail <= 0) {
      p.node = child;
      p.child = -1;
      p.off = 0;
      return 0;
    }
    int take = (rem < avail) ? rem : avail;

    const char* a = sp + st[child].start + p.off;
    int used = 0;
    if (take > 0) {
      if (std::memcmp(a, tp, take) == 0)
        used = take;
      else {
        while (used < take && a[used] == tp[used]) ++used;
      }
    }
    p.off += used;
    if (p.off == el) {
      p.node = child;
      p.child = -1;
      p.off = 0;
    }
    return used;
  }

  void rescan_from_root(Pos& p, const std::string& T, int lpos,
                        int rpos) const {
    p = Pos{root, -1, 0};
    if (lpos > rpos) return;
    const char* tp = T.data();
    int i = lpos;
    while (i <= rpos) {
      int nxt = child_of(p.node, (unsigned char)tp[i]);
      if (nxt == -1) {
        p = Pos{root, -1, 0};
        return;
      }
      int el = edge_length(nxt);
      const char* a = sp + st[nxt].start;
      int remain = rpos - i + 1;
      int take = (remain < el) ? remain : el;

      int used = 0;
      if (take > 0) {
        if (std::memcmp(a, tp + i, take) == 0)
          used = take;
        else {
          while (used < take && a[used] == tp[i + used]) ++used;
          if (used == 0) {
            p = Pos{root, -1, 0};
            return;
          }
        }
      }
      if (used == el) {
        p.node = nxt;
        p.child = -1;
        p.off = 0;
      } else {
        p.child = nxt;
        p.off = used;
      }
      i += used;
    }
  }

  std::vector<int> matching_statistics_start(const std::string& T) const {
    int n = (int)T.size();
    std::vector<int> ms(n, 0);
    Pos p{root, -1, 0};
    const char* tp = T.data();
    int i = 0, j = 0;

    while (i < n) {
      while (j < n) {
        if (p.child == -1) {
          int nxt = child_of(p.node, (unsigned char)tp[j]);
          if (nxt == -1) break;
          p.child = nxt;
          p.off = 0;
        }
        int used = advance_block(p, tp + j, n - j);
        if (used == 0) break;
        j += used;
      }
      ms[i] = j - i;
      if (i + 1 <= j - 1) {
        Pos np;
        rescan_from_root(np, T, i + 1, j - 1);
        p = np;
      } else {
        p = Pos{root, -1, 0};
      }
      if (j < i + 1) j = i + 1;
      ++i;
    }
    return ms;
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
static inline bool all_same_char(const std::string& p) {
  if (p.empty()) return true;
  unsigned char c = (unsigned char)p[0];
  for (char ch : p)
    if ((unsigned char)ch != c) return false;
  return true;
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
  strip_bom(T);
  if (P.empty() || T.empty() || P.size() > T.size()) return 0;

  int n = (int)T.size(), m = (int)P.size();

  if (all_same_char(P)) {
    unsigned char c = (unsigned char)P[0];
    int run = 0;
    for (int i = 0; i < n; ++i) {
      if ((unsigned char)T[i] == c)
        ++run;
      else
        run = 0;
      if (i + 1 - m >= 0 && run >= m) std::cout << (i - m + 2) << '\n';
    }
    return 0;
  }

  std::string S = P;
  S.push_back('\1');
  SuffixTree tree(S);

  std::vector<int> ms = tree.matching_statistics_start(T);

  const char* tp = T.data();
  const char* pp = P.data();
  for (int i = 0; i + m <= n; ++i) {
    if (ms[i] >= m && std::memcmp(tp + i, pp, m) == 0) {
      std::cout << (i + 1) << '\n';
    }
  }
  return 0;
}
