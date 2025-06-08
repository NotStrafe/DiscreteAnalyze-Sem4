CXX      := g++
CXXFLAGS := -std=c++17 -O2 -pipe -Wall

all: kmp naive

kmp   : main.cpp   ; $(CXX) $(CXXFLAGS) $< -o $@
naive : naive.cpp ; $(CXX) $(CXXFLAGS) $< -o $@

bench: kmp naive bench.py
	python3 bench.py

clean:
	rm -f kmp naive *.txt *.csv
