#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class TString {
 private:
  char* data;
  size_t len;

 public:
  TString() : data(nullptr), len(0) {}
  TString(const char* s) {
    len = 0;
    while (s[len] != '\0') {
      ++len;
    }
    data = new char[len + 1];
    for (size_t i = 0; i < len; ++i) {
      data[i] = s[i];
    }
    data[len] = '\0';
  }
  TString(const TString& other) : len(other.len) {
    data = new char[len + 1];
    for (size_t i = 0; i < len; ++i) {
      data[i] = other.data[i];
    }
    data[len] = '\0';
  }
  TString& operator=(const TString& other) {
    if (this == &other) {
      return *this;
    }
    delete[] data;
    len = other.len;
    data = new char[len + 1];
    for (size_t i = 0; i < len; ++i) {
      data[i] = other.data[i];
    }
    data[len] = '\0';
    return *this;
  }
  ~TString() { delete[] data; }
  size_t Length() const { return len; }
  const char* CStr() const { return data ? data : ""; }
};

std::ostream& operator<<(std::ostream& os, const TString& s) {
  os << s.CStr();
  return os;
}

template <typename TValue>
class TVector {
 private:
  TValue* data;
  size_t cap;
  size_t sz;
  void Resize(size_t newCap) {
    TValue* newData = new TValue[newCap];
    for (size_t i = 0; i < sz; ++i) {
      newData[i] = data[i];
    }
    delete[] data;
    data = newData;
    cap = newCap;
  }

 public:
  TVector() : data(nullptr), cap(0), sz(0) {}
  ~TVector() { delete[] data; }
  size_t Size() const { return sz; }
  void PushBack(const TValue& v) {
    if (sz == cap) {
      Resize(cap ? cap * 2 : 1);
    }
    data[sz++] = v;
  }
  TValue& operator[](size_t i) { return data[i]; }
  const TValue& operator[](size_t i) const { return data[i]; }
};

template <typename TFirst, typename TSecond>
struct TPair {
  TFirst first;
  TSecond second;
  TPair() {}
  TPair(const TFirst& a, const TSecond& b) : first(a), second(b) {}
};

char MyToLower(char c) {
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 'a' - 'A') : c;
}

struct TCaseICmp {
  bool operator()(const TString& a, const TString& b) const {
    const char* pa = a.CStr();
    const char* pb = b.CStr();
    size_t i = 0;
    while (pa[i] != '\0' && pb[i] != '\0') {
      char ca = MyToLower(pa[i]);
      char cb = MyToLower(pb[i]);
      if (ca < cb) {
        return true;
      }
      if (ca > cb) {
        return false;
      }
      ++i;
    }
    return a.Length() < b.Length();
  }
  bool Eq(const TString& a, const TString& b) const {
    return !(*this)(a, b) && !(*this)(b, a);
  }
};

const int T_MIN_DEGREE = 32;
const int MAX_KEYS = 2 * T_MIN_DEGREE - 1;
const int MAX_CHILDREN = 2 * T_MIN_DEGREE;

class TBTree {
 private:
  struct TNode {
    bool leaf;
    int n;
    TString keys[MAX_KEYS];
    uint64_t vals[MAX_KEYS];
    TNode* ch[MAX_CHILDREN];
    explicit TNode(bool l = true) : leaf(l), n(0) {
      for (int i = 0; i < MAX_CHILDREN; ++i) {
        ch[i] = nullptr;
      }
    }
  };
  TNode* root;
  TCaseICmp cmp;

  void Destroy(TNode* x) {
    if (!x) {
      return;
    }
    if (!x->leaf) {
      for (int i = 0; i <= x->n; ++i) {
        Destroy(x->ch[i]);
      }
    }
    delete x;
  }
  bool Search(TNode* x, const TString& k, uint64_t& out) {
    int i = 0;
    while (i < x->n && cmp(x->keys[i], k)) {
      ++i;
    }
    if (i < x->n && cmp.Eq(x->keys[i], k)) {
      out = x->vals[i];
      return true;
    }
    if (x->leaf) {
      return false;
    }
    return Search(x->ch[i], k, out);
  }
  bool Exists(TNode* x, const TString& k) {
    uint64_t tmp;
    return Search(x, k, tmp);
  }
  void SplitChild(TNode* x, int i) {
    TNode* y = x->ch[i];
    TNode* z = new TNode(y->leaf);
    z->n = T_MIN_DEGREE - 1;
    for (int j = 0; j < T_MIN_DEGREE - 1; ++j) {
      z->keys[j] = y->keys[j + T_MIN_DEGREE];
      z->vals[j] = y->vals[j + T_MIN_DEGREE];
    }
    if (!y->leaf) {
      for (int j = 0; j < T_MIN_DEGREE; ++j) {
        z->ch[j] = y->ch[j + T_MIN_DEGREE];
      }
    }
    y->n = T_MIN_DEGREE - 1;
    for (int j = x->n; j >= i + 1; --j) {
      x->ch[j + 1] = x->ch[j];
    }
    x->ch[i + 1] = z;
    for (int j = x->n - 1; j >= i; --j) {
      x->keys[j + 1] = x->keys[j];
      x->vals[j + 1] = x->vals[j];
    }
    x->keys[i] = y->keys[T_MIN_DEGREE - 1];
    x->vals[i] = y->vals[T_MIN_DEGREE - 1];
    ++x->n;
  }
  void InsertNonFull(TNode* x, const TString& k, uint64_t v) {
    int i = x->n - 1;
    if (x->leaf) {
      while (i >= 0 && cmp(k, x->keys[i])) {
        x->keys[i + 1] = x->keys[i];
        x->vals[i + 1] = x->vals[i];
        --i;
      }
      x->keys[i + 1] = k;
      x->vals[i + 1] = v;
      ++x->n;
    } else {
      while (i >= 0 && cmp(k, x->keys[i])) {
        --i;
      }
      ++i;
      if (x->ch[i]->n == MAX_KEYS) {
        SplitChild(x, i);
        if (cmp(x->keys[i], k)) {
          ++i;
        }
      }
      InsertNonFull(x->ch[i], k, v);
    }
  }
  void Merge(TNode* x, int idx) {
    TNode* y = x->ch[idx];
    TNode* z = x->ch[idx + 1];
    y->keys[T_MIN_DEGREE - 1] = x->keys[idx];
    y->vals[T_MIN_DEGREE - 1] = x->vals[idx];
    for (int j = 0; j < z->n; ++j) {
      y->keys[j + T_MIN_DEGREE] = z->keys[j];
      y->vals[j + T_MIN_DEGREE] = z->vals[j];
    }
    if (!y->leaf) {
      for (int j = 0; j <= z->n; ++j) {
        y->ch[j + T_MIN_DEGREE] = z->ch[j];
      }
    }
    y->n += z->n + 1;
    for (int j = idx + 1; j < x->n; ++j) {
      x->keys[j - 1] = x->keys[j];
      x->vals[j - 1] = x->vals[j];
      x->ch[j] = x->ch[j + 1];
    }
    --x->n;
    delete z;
  }
  void BorrowFromPrev(TNode* x, int idx) {
    TNode* y = x->ch[idx];
    TNode* z = x->ch[idx - 1];
    for (int j = y->n - 1; j >= 0; --j) {
      y->keys[j + 1] = y->keys[j];
      y->vals[j + 1] = y->vals[j];
    }
    if (!y->leaf) {
      for (int j = y->n; j >= 0; --j) {
        y->ch[j + 1] = y->ch[j];
      }
    }
    y->keys[0] = x->keys[idx - 1];
    y->vals[0] = x->vals[idx - 1];
    if (!y->leaf) {
      y->ch[0] = z->ch[z->n];
    }
    x->keys[idx - 1] = z->keys[z->n - 1];
    x->vals[idx - 1] = z->vals[z->n - 1];
    --z->n;
    ++y->n;
  }
  void BorrowFromNext(TNode* x, int idx) {
    TNode* y = x->ch[idx];
    TNode* z = x->ch[idx + 1];
    y->keys[y->n] = x->keys[idx];
    y->vals[y->n] = x->vals[idx];
    if (!y->leaf) {
      y->ch[y->n + 1] = z->ch[0];
    }
    ++y->n;
    x->keys[idx] = z->keys[0];
    x->vals[idx] = z->vals[0];
    for (int j = 1; j < z->n; ++j) {
      z->keys[j - 1] = z->keys[j];
      z->vals[j - 1] = z->vals[j];
    }
    if (!z->leaf) {
      for (int j = 1; j <= z->n; ++j) {
        z->ch[j - 1] = z->ch[j];
      }
    }
    --z->n;
  }
  void Remove(TNode* x, const TString& k) {
    int idx = 0;
    while (idx < x->n && cmp(x->keys[idx], k)) {
      ++idx;
    }
    if (idx < x->n && cmp.Eq(x->keys[idx], k)) {
      if (x->leaf) {
        for (int i = idx + 1; i < x->n; ++i) {
          x->keys[i - 1] = x->keys[i];
          x->vals[i - 1] = x->vals[i];
        }
        --x->n;
      } else if (x->ch[idx]->n >= T_MIN_DEGREE) {
        TNode* y = x->ch[idx];
        while (!y->leaf) {
          y = y->ch[y->n];
        }
        x->keys[idx] = y->keys[y->n - 1];
        x->vals[idx] = y->vals[y->n - 1];
        Remove(x->ch[idx], x->keys[idx]);
      } else if (x->ch[idx + 1]->n >= T_MIN_DEGREE) {
        TNode* y = x->ch[idx + 1];
        while (!y->leaf) {
          y = y->ch[0];
        }
        x->keys[idx] = y->keys[0];
        x->vals[idx] = y->vals[0];
        Remove(x->ch[idx + 1], x->keys[idx]);
      } else {
        Merge(x, idx);
        Remove(x->ch[idx], k);
      }
    } else {
      if (x->leaf) {
        return;
      }
      if (x->ch[idx]->n < T_MIN_DEGREE) {
        if (idx != 0 && x->ch[idx - 1]->n >= T_MIN_DEGREE) {
          BorrowFromPrev(x, idx);
        } else if (idx != x->n && x->ch[idx + 1]->n >= T_MIN_DEGREE) {
          BorrowFromNext(x, idx);
        } else {
          if (idx != x->n) {
            Merge(x, idx);
          } else {
            Merge(x, idx - 1);
            --idx;
          }
        }
      }
      Remove(x->ch[idx], k);
    }
  }
  bool WriteStr(FILE* f, const TString& s) const {
    uint16_t l = static_cast<uint16_t>(s.Length());
    return std::fwrite(&l, sizeof l, 1, f) == 1 &&
           std::fwrite(s.CStr(), 1, l, f) == l;
  }
  bool ReadStr(FILE* f, TString& s) {
    uint16_t l = 0;
    if (std::fread(&l, sizeof l, 1, f) != 1 || l > 256) {
      return false;
    }
    char buf[257];
    if (std::fread(buf, 1, l, f) != l) {
      return false;
    }
    buf[l] = '\0';
    s = TString(buf);
    return true;
  }
  bool SaveNode(FILE* f, const TNode* x) const {
    std::fwrite(&x->leaf, 1, 1, f);
    std::fwrite(&x->n, sizeof x->n, 1, f);
    for (int i = 0; i < x->n; ++i) {
      if (!WriteStr(f, x->keys[i])) {
        return false;
      }
      std::fwrite(&x->vals[i], sizeof(uint64_t), 1, f);
    }
    if (!x->leaf) {
      for (int i = 0; i <= x->n; ++i) {
        if (!SaveNode(f, x->ch[i])) {
          return false;
        }
      }
    }
    return true;
  }
  TNode* LoadNode(FILE* f) {
    bool leaf = false;
    int n = 0;
    if (std::fread(&leaf, 1, 1, f) != 1) {
      return nullptr;
    }
    if (std::fread(&n, sizeof n, 1, f) != 1 || n < 0 || n > MAX_KEYS) {
      return nullptr;
    }
    TNode* x = new TNode(leaf);
    x->n = n;
    for (int i = 0; i < n; ++i) {
      if (!ReadStr(f, x->keys[i])) {
        delete x;
        return nullptr;
      }
      if (std::fread(&x->vals[i], sizeof(uint64_t), 1, f) != 1) {
        delete x;
        return nullptr;
      }
    }
    if (!leaf) {
      for (int i = 0; i <= n; ++i) {
        x->ch[i] = LoadNode(f);
        if (!x->ch[i]) {
          delete x;
          return nullptr;
        }
      }
    }
    return x;
  }

 public:
  TBTree() : root(new TNode(true)) {}
  ~TBTree() { Destroy(root); }
  bool Insert(const TString& k, uint64_t v) {
    if (Exists(root, k)) {
      return false;
    }
    if (root->n == MAX_KEYS) {
      TNode* s = new TNode(false);
      s->ch[0] = root;
      SplitChild(s, 0);
      root = s;
    }
    InsertNonFull(root, k, v);
    return true;
  }
  bool Remove(const TString& k) {
    if (!Exists(root, k)) {
      return false;
    }
    Remove(root, k);
    if (!root->leaf && root->n == 0) {
      TNode* old = root;
      root = root->ch[0];
      delete old;
    }
    return true;
  }
  bool Find(const TString& k, uint64_t& out) { return Search(root, k, out); }
  bool Save(const char* path) const {
    FILE* f = std::fopen(path, "wb");
    if (!f) {
      return false;
    }
    const char sig[8] = {'B', 'T', 'R', 'E', 'E', '1', 0, 0};
    std::fwrite(sig, 1, 8, f);
    bool ok = SaveNode(f, root);
    std::fclose(f);
    return ok;
  }
  bool Load(const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (!f) {
      return false;
    }
    char sig[8];
    if (std::fread(sig, 1, 8, f) != 8 || std::memcmp(sig, "BTREE1", 6) != 0) {
      std::fclose(f);
      return false;
    }
    TNode* nr = LoadNode(f);
    std::fclose(f);
    if (!nr) {
      return false;
    }
    Destroy(root);
    root = nr;
    return true;
  }
};

void TrimNl(char* s) {
  size_t l = std::strlen(s);
  if (l && (s[l - 1] == '\n' || s[l - 1] == '\r')) {
    s[l - 1] = '\0';
  }
}

int main() {
  TBTree dict;
  const int BUF_SIZE = 2048;
  char cmd[BUF_SIZE];
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  while (std::cin.getline(cmd, BUF_SIZE)) {
    TrimNl(cmd);
    if (cmd[0] == '\0') {
      continue;
    }
    if (cmd[0] == '+') {
      char wordBuf[257];
      unsigned long long num = 0;
      if (std::sscanf(cmd, "+ %256s %llu", wordBuf, &num) != 2) {
        std::cout << "ERROR: Invalid insert syntax\n";
        continue;
      }
      TString key(wordBuf);
      if (dict.Insert(key, static_cast<uint64_t>(num))) {
        std::cout << "OK\n";
      } else {
        std::cout << "Exist\n";
      }
    } else if (cmd[0] == '-') {
      char wordBuf[257];
      if (std::sscanf(cmd, "- %256s", wordBuf) != 1) {
        std::cout << "ERROR: Invalid remove syntax\n";
        continue;
      }
      TString key(wordBuf);
      if (dict.Remove(key)) {
        std::cout << "OK\n";
      } else {
        std::cout << "NoSuchWord\n";
      }
    } else if (cmd[0] == '!') {
      char sub[8];
      char pathBuf[BUF_SIZE];
      if (std::sscanf(cmd, "! %7s %1023s", sub, pathBuf) != 2) {
        std::cout << "ERROR: Invalid system command\n";
        continue;
      }
      if (std::strcmp(sub, "Save") == 0) {
        if (dict.Save(pathBuf)) {
          std::cout << "OK\n";
        } else {
          std::cout << "ERROR: Cannot save\n";
        }
      } else if (std::strcmp(sub, "Load") == 0) {
        if (dict.Load(pathBuf)) {
          std::cout << "OK\n";
        } else {
          std::cout << "ERROR: Cannot load\n";
        }
      } else {
        std::cout << "ERROR: Unknown system command\n";
      }
    } else {
      char wordBuf[257];
      if (std::sscanf(cmd, "%256s", wordBuf) != 1) {
        std::cout << "ERROR: Invalid search\n";
        continue;
      }
      TString key(wordBuf);
      uint64_t val = 0;
      if (dict.Find(key, val)) {
        std::cout << "OK: " << val << '\n';
      } else {
        std::cout << "NoSuchWord\n";
      }
    }
  }
  return 0;
}
