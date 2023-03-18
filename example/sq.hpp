#pragma once
#include <cstddef>

template <typename T, size_t N>
class SQ {
 public:
	SQ() {}
	bool put(T v) {
		if (cnt == N) {
			return false;
		}
		size_t pos = ri + cnt;
		if (pos >= N)
			pos -= N;
		buffer[pos] = v;
		cnt++;
		return true;
	}
	bool get(T& v) {
		if (cnt == 0) {
			return false;
		}
		v = buffer[ri];
		cnt--;
		ri++;
		if (ri >= N)
			ri -= N;
		return true;
	}
	size_t count() const { return cnt; }
	size_t capacity() const { return N; }

 private:
	T buffer[N];
	size_t ri = 0;
	size_t cnt = 0;
};
