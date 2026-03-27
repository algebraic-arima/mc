#pragma once
#include <bitset>
#ifndef __BITSET_H__
#define __BITSET_H__

class BitSet {
public:
  std::bitset<500> bits;

  BitSet() = default;

  explicit BitSet(const std::bitset<500> &b) : bits(b) {}

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
};

#endif // __BITSET_H__