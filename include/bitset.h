#pragma once
#include <bitset>
#ifndef __BITSET_H__
#define __BITSET_H__

class BitSet {
public:
  static const size_t MAX_SIZE = 65536;
  std::bitset<MAX_SIZE> bits;

  BitSet() = default;

  explicit BitSet(const std::bitset<MAX_SIZE> &b) : bits(b) {}

  BitSet &operator++() {
    for (size_t i = 0; i < bits.size(); ++i) {
      if (!bits.test(i)) {
        bits.set(i);
        break;
      } else {
        bits.reset(i);
      }
    }
    return *this;
  }

  bool operator<(const BitSet &other) const {
    return bits.to_ullong() < other.bits.to_ullong();
  }

  bool operator==(const BitSet &other) const { return bits == other.bits; }

  bool operator!=(const BitSet &other) const { return bits != other.bits; }

  BitSet operator&(const BitSet &other) const {
    return BitSet(bits & other.bits);
  }

  BitSet operator|(const BitSet &other) const {
    return BitSet(bits | other.bits);
  }

  BitSet operator~() const { return BitSet(~bits); }

  bool operator[](size_t pos) const { return bits[pos]; }

  bool test(size_t pos) const { return bits.test(pos); }

  void set(size_t pos, bool value = true) { bits.set(pos, value); }

  void reset(size_t pos) { bits.reset(pos); }

  bool back() const { return bits[bits.size() - 1]; }

  friend std::ostream &operator<<(std::ostream &os, const BitSet &bs) {
    os << bs.bits;
    return os;
  }
};

#endif // __BITSET_H__