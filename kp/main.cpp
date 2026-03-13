#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace {

inline int symbol_to_id(char ch) { return (ch == '$') ? 0 : (ch - 'a' + 1); }

std::vector<int> cyclic_suffix_array(const std::string& s) {
  const int n = static_cast<int>(s.size());
  std::vector<int> order(n), cls(n);

  {
    const int alpha = 256;
    std::vector<int> freq(alpha, 0);
    for (char ch : s) ++freq[static_cast<unsigned char>(ch)];
    for (int i = 1; i < alpha; ++i) freq[i] += freq[i - 1];
    for (int i = 0; i < n; ++i) {
      order[--freq[static_cast<unsigned char>(s[i])]] = i;
    }
    cls[order[0]] = 0;
    int groups = 1;
    for (int i = 1; i < n; ++i) {
      if (s[order[i]] != s[order[i - 1]]) ++groups;
      cls[order[i]] = groups - 1;
    }
  }

  std::vector<int> shifted(n), next_cls(n);
  for (int h = 0; (1 << h) < n; ++h) {
    for (int i = 0; i < n; ++i) {
      shifted[i] = order[i] - (1 << h);
      if (shifted[i] < 0) shifted[i] += n;
    }

    std::vector<int> freq(n, 0);
    for (int i = 0; i < n; ++i) ++freq[cls[shifted[i]]];
    for (int i = 1; i < n; ++i) freq[i] += freq[i - 1];
    for (int i = n - 1; i >= 0; --i) {
      order[--freq[cls[shifted[i]]]] = shifted[i];
    }

    next_cls[order[0]] = 0;
    int groups = 1;
    for (int i = 1; i < n; ++i) {
      int cur_l = cls[order[i]];
      int cur_r = cls[(order[i] + (1 << h)) % n];
      int prv_l = cls[order[i - 1]];
      int prv_r = cls[(order[i - 1] + (1 << h)) % n];
      if (cur_l != prv_l || cur_r != prv_r) ++groups;
      next_cls[order[i]] = groups - 1;
    }
    cls.swap(next_cls);
  }

  return order;
}

std::string bwt_encode(const std::string& text) {
  std::string src = text;
  src.push_back('$');
  const std::vector<int> order = cyclic_suffix_array(src);
  const int n = static_cast<int>(src.size());

  std::string last_col(n, '$');
  for (int i = 0; i < n; ++i) {
    int prev = order[i] - 1;
    if (prev < 0) prev += n;
    last_col[i] = src[prev];
  }
  return last_col;
}

std::string bwt_decode(const std::string& last_col) {
  const int n = static_cast<int>(last_col.size());
  std::array<int, 27> freq{};
  std::vector<int> occ_idx(n, 0);

  for (int i = 0; i < n; ++i) {
    int id = symbol_to_id(last_col[i]);
    occ_idx[i] = freq[id];
    ++freq[id];
  }

  std::array<int, 27> first_pos{};
  for (int i = 1; i < 27; ++i) {
    first_pos[i] = first_pos[i - 1] + freq[i - 1];
  }

  std::vector<int> lf_map(n, 0);
  int sentinel_row = -1;
  for (int i = 0; i < n; ++i) {
    int id = symbol_to_id(last_col[i]);
    lf_map[i] = first_pos[id] + occ_idx[i];
    if (last_col[i] == '$') sentinel_row = i;
  }

  std::string with_sentinel(n, '$');
  int row = sentinel_row;
  for (int i = n - 1; i >= 0; --i) {
    with_sentinel[i] = last_col[row];
    row = lf_map[row];
  }

  with_sentinel.pop_back();
  return with_sentinel;
}

std::vector<int> mtf_encode(const std::string& input) {
  std::vector<char> dict(27);
  dict[0] = '$';
  for (int i = 1; i <= 26; ++i) dict[i] = static_cast<char>('a' + i - 1);

  std::vector<int> out;
  out.reserve(input.size());
  for (char ch : input) {
    int idx = 0;
    while (dict[idx] != ch) ++idx;
    out.push_back(idx);
    for (int i = idx; i > 0; --i) dict[i] = dict[i - 1];
    dict[0] = ch;
  }
  return out;
}

std::string mtf_decode(const std::vector<int>& codes) {
  std::vector<char> dict(27);
  dict[0] = '$';
  for (int i = 1; i <= 26; ++i) dict[i] = static_cast<char>('a' + i - 1);

  std::string text;
  text.reserve(codes.size());
  for (int idx : codes) {
    char ch = dict[idx];
    text.push_back(ch);
    for (int i = idx; i > 0; --i) dict[i] = dict[i - 1];
    dict[0] = ch;
  }
  return text;
}

struct Run {
  int count;
  int value;
};

std::vector<Run> rle_encode(const std::vector<int>& input) {
  std::vector<Run> runs;
  if (input.empty()) return runs;

  int current = input[0];
  int length = 1;
  for (std::size_t i = 1; i < input.size(); ++i) {
    if (input[i] == current) {
      ++length;
    } else {
      runs.push_back({length, current});
      current = input[i];
      length = 1;
    }
  }
  runs.push_back({length, current});
  return runs;
}

std::vector<int> rle_decode(const std::vector<Run>& runs) {
  std::vector<int> out;
  for (const Run& run : runs) {
    for (int i = 0; i < run.count; ++i) out.push_back(run.value);
  }
  return out;
}

}  // namespace

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string mode;
  if (!(std::cin >> mode)) return 0;

  if (mode == "compress") {
    std::string text;
    std::cin >> text;
    const std::string bwt = bwt_encode(text);
    const std::vector<int> mtf = mtf_encode(bwt);
    const std::vector<Run> rle = rle_encode(mtf);

    for (const Run& run : rle) {
      std::cout << run.count << ' ' << run.value << '\n';
    }
    return 0;
  }

  if (mode == "decompress") {
    std::vector<Run> runs;
    int cnt, value;
    while (std::cin >> cnt >> value) {
      runs.push_back({cnt, value});
    }

    const std::vector<int> mtf_codes = rle_decode(runs);
    const std::string bwt = mtf_decode(mtf_codes);
    const std::string text = bwt_decode(bwt);
    std::cout << text << '\n';
    return 0;
  }

  return 0;
}
