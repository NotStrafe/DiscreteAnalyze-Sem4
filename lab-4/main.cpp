#include <cstdint>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<uint32_t> ReadPattern(const std::string& line) {
  std::vector<uint32_t> pattern;
  std::stringstream ss(line);
  std::string token;
  while (ss >> token) {
    unsigned long long value = std::stoull(token);
    pattern.push_back(static_cast<uint32_t>(value));
  }
  return pattern;
}

std::vector<size_t> BuildPi(const std::vector<uint32_t>& pattern) {
  size_t m = pattern.size();
  std::vector<size_t> pi(m, 0);
  for (size_t i = 1; i < m; ++i) {
    size_t j = pi[i - 1];
    while (j > 0 && pattern[i] != pattern[j]) {
      j = pi[j - 1];
    }
    if (pattern[i] == pattern[j]) {
      ++j;
    }
    pi[i] = j;
  }
  return pi;
}

void Search(const std::vector<uint32_t>& pattern,
            const std::vector<size_t>& pi) {
  const size_t m = pattern.size();
  if (m == 0) return;

  size_t currentLine = 1, wordInLine = 0, j = 0;
  std::deque<std::pair<size_t, size_t>> window;
  std::string line;

  while (std::getline(std::cin, line)) {
    if (line.empty()) {
      ++currentLine;
      wordInLine = 0;
      continue;
    }

    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
      uint32_t x = static_cast<uint32_t>(std::stoull(token));

      ++wordInLine;
      window.emplace_back(currentLine, wordInLine);
      if (window.size() > m) window.pop_front();

      while (j > 0 && x != pattern[j]) j = pi[j - 1];
      if (x == pattern[j]) ++j;
      if (j == m) {
        std::cout << window.front().first << ", " << window.front().second
                  << '\n';
        j = pi[j - 1];
      }
    }
    ++currentLine;
    wordInLine = 0;
  }
}

int main() {
  std::string patternLine;
  if (!std::getline(std::cin, patternLine)) return 0;
  auto pattern = ReadPattern(patternLine);
  auto pi = BuildPi(pattern);
  Search(pattern, pi);
  return 0;
}
