// naive.cpp — наивный пословный поиск образца
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<uint32_t> SplitToNumbers(const std::string& line) {
  std::vector<uint32_t> out;
  std::stringstream ss(line);
  std::string token;
  while (ss >> token) {
    out.push_back(static_cast<uint32_t>(std::stoull(token)));
  }
  return out;
}

int main() {
  /* ---------- читаем образец ---------- */
  std::string patternLine;
  if (!std::getline(std::cin, patternLine)) return 0;

  std::vector<uint32_t> pattern = SplitToNumbers(patternLine);
  const std::size_t m = pattern.size();
  if (m == 0) return 0;

  /* ---------- читаем текст полностью ---------- */
  std::vector<uint32_t> text;                              // все слова подряд
  std::vector<std::pair<std::size_t, std::size_t>> coord;  // (строка, слово)
  std::string line;
  std::size_t currentLine = 1;
  std::size_t wordInLine = 0;

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
      text.push_back(x);
      ++wordInLine;
      coord.emplace_back(currentLine, wordInLine);
    }
    ++currentLine;
    wordInLine = 0;
  }

  const std::size_t n = text.size();
  if (n < m) return 0;

  /* ---------- наивный поиск ---------- */
  for (std::size_t i = 0; i + m <= n; ++i) {
    std::size_t k = 0;
    while (k < m && text[i + k] == pattern[k]) {
      ++k;
    }
    if (k == m) {  // найдено совпадение
      std::cout << coord[i].first << ", " << coord[i].second << '\n';
    }
  }
  return 0;
}
