#include "ts.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

void TS::print() const {
  for (const auto &state : states) {
    std::cout << "s" << state.id << ": AP {";
    for (size_t i = 0; i < prop.size(); ++i) {
      if (state.ap_set.test(i)) {
        std::cout << prop[i] << " ";
      }
    }
    std::cout << "}, Transitions {";
    for (size_t i = 0; i < states.size(); ++i) {
      if (state.transitions.test(i)) {
        std::cout << i << " ";
      }
    }
    std::cout << "}\n";
  }
}

TS::TS(std::istream &is) {
  std::size_t s, t;
  is >> s >> t;
  if (s > BitSet::MAX_SIZE) {
    throw std::runtime_error("Too many states in TS: " + std::to_string(s));
  }
  states.resize(s);
  for (std::size_t i = 0; i < s; ++i) {
    states[i].id = i;
  }

  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  std::string line;
  std::getline(is, line);
  std::istringstream ss(line);

  int x;
  while (ss >> x) {
    if (x < 0) {
      initial_states.bits.reset();
    } else {
      initial_states.set(x); // 2nd line
    }
  }

  std::getline(is, line);
  std::istringstream ss2(line);
  while (ss2 >> x) {
    // actions.push_back(x); // 3rd line
  }

  std::getline(is, line);
  std::istringstream ss3(line);
  std::string ap;
  while (ss3 >> ap) {
    prop.push_back(ap); // 4th line
  }

  for (std::size_t i = 0; i < t; ++i) {
    std::size_t from, act, to;
    is >> from >> act >> to;
    (void)act;
    states[from].transitions.set(to);
  }

  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  for (std::size_t i = 0; i < s; ++i) {
    std::getline(is, line);
    std::istringstream ss(line);
    long long id;
    while (ss >> id) {
      if (id < 0) {
        states[i].ap_set.bits.reset();
        continue;
      } else {
        states[i].ap_set.set(static_cast<std::size_t>(id));
      }
    }
  }
#ifdef DEBUG
  print();
#endif
}

std::vector<TS::StateID> TS::get_successors(TS::StateID from) const {
  std::vector<StateID> result;
  if (from >= states.size()) {
    return result;
  }
  for (std::size_t to = 0; to < states.size(); ++to) {
    if (states[from].transitions.test(to)) {
      result.push_back(to);
    }
  }
  return result;
}
