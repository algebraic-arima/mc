#include "prod.h"

void Prod::construct_initial_states(int init) {
  StateSet st_initial;
  if (init == -1) {
    st_initial = ts.initial_states;
  } else {
    if (init >= ts.states.size()) {
      throw std::runtime_error("Invalid initial state index: " +
                               std::to_string(init));
    }
    st_initial.set(init);
  }
  for (size_t i = 0; i < ts.states.size(); ++i) {
    if (!st_initial.test(i)) {
      continue;
    }
    auto transition = ts.states[i].ap_set; // L(s_i)
    StateSet succ;
    for (size_t j = 0; j < nba.states.size(); ++j) {
      if (!nba.initial_states.test(j)) {
        continue;
      }
      auto it = nba.delta.find({j, transition});
      if (it != nba.delta.end()) {
        succ = succ | it->second;
      }
    }
    for (size_t k = 0; k < nba.states.size(); ++k) {
      if (succ.test(k)) {
        initial_states.set(k * ts.states.size() + i);
      }
    }
  }
}

void Prod::construct_accepting_states() {
  for (size_t i = 0; i < nba.states.size(); ++i) {
    if (nba.accepting_states.test(i)) {
      for (size_t j = 0; j < ts.states.size(); ++j) {
        accept_states.set(i * ts.states.size() + j);
      }
    }
  }
}

void Prod::print() const {
  std::cout << "Initial states: { ";
  for (size_t i = 0; i < nba.states.size(); ++i) {
    for (size_t j = 0; j < ts.states.size(); ++j) {
      if (initial_states.test(i * ts.states.size() + j)) {
        std::cout << "(" << i << ", " << j << ") ";
      }
    }
  }
  std::cout << "}\nAccepting states: { ";
  for (size_t i = 0; i < nba.states.size(); ++i) {
    for (size_t j = 0; j < ts.states.size(); ++j) {
      if (accept_states.test(i * ts.states.size() + j)) {
        std::cout << "(" << i << ", " << j << ") ";
      }
    }
  }
  std::cout << "}" << std::endl;
}

// the transition function of the product TS
Prod::StateSet Prod::step(const StateID &prod) {
  StateSet successors;
  for (const auto &ts_s : ts.get_successors(get_ts_id(ts, prod))) {
#ifdef DEBUG
    std::cout << "From: (" << get_nba_id(ts, prod) << ", "
              << get_ts_id(ts, prod) << ") Via: {";
    for (size_t i = 0; i < ts.prop.size(); ++i) {
      if (ts.states[ts_s].ap_set.test(i)) {
        std::cout << ts.prop[i] << " ";
      }
    }
    std::cout << "} To: {";
#endif
    for (size_t nba_s :
         nba.get_successors(get_nba_id(ts, prod), ts.states[ts_s].ap_set)) {

#ifdef DEBUG
      std::cout << "(" << nba_s << ", " << ts_s << ") ";
#endif
      successors.set(nba_s * ts.states.size() + ts_s);
    }
#ifdef DEBUG
    std::cout << "}" << std::endl;
#endif
  }
  return successors;
}


// below are textbook algorithms of nestedDFS
bool Prod::persistence_check() {
  auto I = initial_states;
  while (!cycle_found && ((I & R) != I)) {
    // I & R == I iff I \subseteq R
    for (size_t i = 0; i < nba.states.size() * ts.states.size(); ++i) {
      if (I.test(i) && !R.test(i)) {
        reachable_cycle(i);
        break;
      }
    }
  }
#ifdef CYCLE
  if (cycle_found) {
    std::cout << "Cycle: \nU\n";
    for (auto i : U) {
      std::cout << "(" << get_nba_id(ts, i) << ", " << get_ts_id(ts, i)
                << ")\n";
    }
    std::cout << "V\n";
    for (auto i : V) {
      std::cout << "(" << get_nba_id(ts, i) << ", " << get_ts_id(ts, i)
                << ")\n";
    }
  }
#endif
  return !cycle_found;
}

void Prod::reachable_cycle(StateID s) {
  U.push_back(s);
  R.set(s);
  do {
    StateID ss = U.back();
    auto post = step(ss);
    if ((post & R) != post) {
      for (size_t i = 0; i < nba.states.size() * ts.states.size(); ++i) {
        if (post.test(i) && !R.test(i)) {
          U.push_back(i);
          R.set(i);
          break;
        }
      }
    } else {
      U.pop_back();
      if (nba.accepting_states.test(get_nba_id(ts, ss))) {
        cycle_found = cycle_test(ss);
      }
    }
  } while (!U.empty() && !cycle_found);
}

bool Prod::cycle_test(StateID s) {
  bool cycle_found = false;
  V.push_back(s);
  T.set(s);
  do {
    auto ss = V.back();
    auto post = step(ss);
    if (post.test(s)) {
      cycle_found = true;
      V.push_back(s);
    } else {
      auto post = step(ss);
      if ((post & T) != post) {
        for (size_t i = 0; i < nba.states.size() * ts.states.size(); ++i) {
          if (post.test(i) && !T.test(i)) {
            V.push_back(i);
            T.set(i);
            break;
          }
        }
      } else {
        V.pop_back();
      }
    }
  } while (!V.empty() && !cycle_found);
  return cycle_found;
}
