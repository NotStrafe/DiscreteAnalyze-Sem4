#include <iostream>

class TString {
 private:
  char* data;
  int len;

 public:
  TString() : data(nullptr), len(0) {}

  TString(const char* s) {
    int l = 0;
    while (s[l] != '\0') {
      l++;
    }
    len = l;
    data = new char[len + 1];
    for (int i = 0; i <= len; i++) {
      data[i] = s[i];
    }
  }

  TString(const TString& other) {
    len = other.len;
    data = new char[len + 1];
    for (int i = 0; i <= len; i++) {
      data[i] = other.data[i];
    }
  }

  TString& operator=(const TString& other) {
    if (this != &other) {
      delete[] data;
      len = other.len;
      data = new char[len + 1];
      for (int i = 0; i <= len; i++) {
        data[i] = other.data[i];
      }
    }
    return *this;
  }

  ~TString() { delete[] data; }

  const char* c_str() const { return data; }
};

struct TPair {
  unsigned long long key;
  TString value;
};

template <typename T>
class TDynamicArray {
 private:
  T* data;
  int size;
  int capacity;

  void Resize() {
    int newCap = capacity * 2;
    T* newData = new T[newCap];
    for (int i = 0; i < size; i++) {
      newData[i] = data[i];
    }
    delete[] data;
    data = newData;
    capacity = newCap;
  }

 public:
  TDynamicArray() : size(0), capacity(1) { data = new T[capacity]; }
  ~TDynamicArray() { delete[] data; }

  void PushBack(const T& elem) {
    if (size == capacity) Resize();
    data[size++] = elem;
  }
  T& operator[](int index) { return data[index]; }
  int getSize() const { return size; }
};

void LSDRadixSort(TDynamicArray<TPair>& arr) {
  int n = arr.getSize();

  for (int shift = 0; shift < 64; shift += 8) {
    unsigned int count[256] = {0};

    for (int i = 0; i < n; i++) {
      unsigned long long x = arr[i].key;
      unsigned int c = (x >> shift) & 0xFF;
      count[c]++;
    }

    for (int i = 1; i < 256; i++) {
      count[i] += count[i - 1];
    }

    TPair* temp = new TPair[n];
    for (int i = n - 1; i >= 0; i--) {
      unsigned long long x = arr[i].key;
      unsigned int c = (x >> shift) & 0xFF;
      count[c]--;
      temp[count[c]] = arr[i];
    }

    for (int i = 0; i < n; i++) {
      arr[i] = temp[i];
    }
    delete[] temp;
  }
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  TDynamicArray<TPair> arr;
  while (true) {
    unsigned long long key;

    char buffer[10000];

    if (!(std::cin >> key)) break;
    if (!(std::cin >> buffer)) break;

    TPair p;
    p.key = key;
    p.value = TString(buffer);

    arr.PushBack(p);
  }

  if (arr.getSize() > 1) {
    LSDRadixSort(arr);
  }

  for (int i = 0; i < arr.getSize(); i++) {
    std::cout << arr[i].key << "\t" << arr[i].value.c_str() << "\n";
  }

  return 0;
}
