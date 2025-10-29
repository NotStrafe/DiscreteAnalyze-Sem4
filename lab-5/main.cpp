#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct SuffixAutomaton {
  struct State {
    int link = -1;
    int len = 0;
    std::unordered_map<char, int> next;
  };

  std::vector<State> st;
  int last = 0;

  explicit SuffixAutomaton(std::size_t reserve_len = 0) {
    st.reserve(2 * reserve_len + 1);
    st.push_back(State{});
    st[0].link = -1;
    st[0].len = 0;
    last = 0;
  }

  void extend(char c) {
    int cur = static_cast<int>(st.size());
    st.push_back(State{});
    st[cur].len = st[last].len + 1;

    int p = last;
    while (p != -1 && !has_transition(p, c)) {
      st[p].next[c] = cur;
      p = st[p].link;
    }

    if (p == -1) {
      st[cur].link = 0;
    } else {
      int q = st[p].next[c];
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        int clone = static_cast<int>(st.size());
        st.push_back(State{});
        st[clone].len = st[p].len + 1;
        st[clone].next = st[q].next;
        st[clone].link = st[q].link;

        while (p != -1 && st[p].next.count(c) && st[p].next[c] == q) {
          st[p].next[c] = clone;
          p = st[p].link;
        }
        st[q].link = st[cur].link = clone;
      }
    }
    last = cur;
  }

  inline bool has_transition(int v, char c) const {
    return st[v].next.find(c) != st[v].next.end();
  }
};

static inline void strip_cr(std::string& s) {
  if (!s.empty() && s.back() == '\r') s.pop_back();
}
static inline void strip_utf8_bom(std::string& s) {
  if (s.size() >= 3 && static_cast<unsigned char>(s[0]) == 0xEF &&
      static_cast<unsigned char>(s[1]) == 0xBB &&
      static_cast<unsigned char>(s[2]) == 0xBF) {
    s.erase(0, 3);
  }
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string P, T;
  if (!std::getline(std::cin, P)) return 0;
  std::getline(std::cin, T);

  strip_cr(P);
  strip_cr(T);
  strip_utf8_bom(P);

  const int m = static_cast<int>(P.size());
  const int n = static_cast<int>(T.size());
  if (m == 0 || n == 0 || m > n) {
    return 0;
  }

  std::string Pr(P.rbegin(), P.rend());
  SuffixAutomaton sam(Pr.size());
  for (char c : Pr) sam.extend(c);

  std::string Sr(T.rbegin(), T.rend());
  std::vector<int> ms(n, 0);

  int v = 0;
  int l = 0;

  for (int j = 0; j < n; ++j) {
    char c = Sr[j];
    while (v != 0 && !sam.has_transition(v, c)) {
      v = sam.st[v].link;
      l = sam.st[v].len;
    }
    if (sam.has_transition(v, c)) {
      v = sam.st[v].next[c];
      ++l;
    } else {
      v = 0;
      l = 0;
    }
    int i = n - 1 - j;
    ms[i] = l;
  }

  bool printed = false;
  for (int i = 0; i <= n - m; ++i) {
    if (ms[i] >= m) {
      std::cout << (i + 1) << '\n';
      printed = true;
    }
  }

  return 0;
}
