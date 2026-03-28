#pragma once
#ifndef __PROD_H__
#define __PROD_H__

#include "bitset.h"
#include "nba.h"
#include "ts.h"
#include <vector>

class Prod {
public:
  using StateID = size_t;
  using StateSet = BitSet;
  NBA &nba;
  TS &ts;
  size_t nba_states, ts_states;
  StateSet initial_states, accept_states;
  StateSet R, T;
  std::vector<StateID> U, V;
  bool cycle_found = false;
  Prod(NBA &nba, TS &ts, int init = -1) : nba(nba), ts(ts) {
    nba_states = nba.states.size();
    ts_states = ts.states.size();
    construct_accepting_states();
    construct_initial_states(init);

    if (nba_states * ts_states > BitSet::MAX_SIZE) {
      throw std::runtime_error("Too many states in NBA: " +
                               std::to_string(nba_states * ts_states));
    }
#ifdef DEBUG
    print();
#endif
  }
  void construct_initial_states(int init);
  void construct_accepting_states();
  StateSet step(const StateID &prod);
  bool persistence_check();
  bool cycle_test(StateID s);
  void reachable_cycle(StateID s);
  void print() const;
};

// col number: ts_state_id
// row number: nba_state_id

inline Prod::StateID get_id(const TS &ts, NBA::StateID nba_id,
                            TS::StateID ts_id) {
  return nba_id * ts.states.size() + ts_id;
}

inline NBA::StateID get_nba_id(const TS &ts, Prod::StateID state) {
  return state / ts.states.size();
}
inline TS::StateID get_ts_id(const TS &ts, Prod::StateID state) {
  return state % ts.states.size();
}

#endif // __PROD_H__
