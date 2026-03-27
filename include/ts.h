#pragma once
#include <cstddef>
#include <istream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "bitset.h"
#ifndef __TS_H__
#define __TS_H__

class TS {
public:
  using StateID = std::size_t;
  using APRef = std::vector<std::string>;
  using APSet = BitSet;
  using ActID = std::size_t; // anonymous action
  using StateSet = BitSet;

  struct State {
    StateID id;
    APSet ap_set; // L
    StateSet transitions; // I
  };
  using StateRef = std::vector<State>;

  StateRef states;
  APRef prop; // AP
  // std::vector<ActID> actions; // Act
  StateSet initial_states;

  explicit TS(std::istream &is);
  void print() const;
  std::vector<StateID> get_successors(StateID from) const;
};

#endif // __TS_H__