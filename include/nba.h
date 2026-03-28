#pragma once
#include "ast.h"
#include "bitset.h"
#include "parser.h"
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#ifndef __NBA_H__
#define __NBA_H__

class NBA {
public:
  using StateID = size_t;
  using ASTNodePtr = ASTNode *;
  using APRef = std::vector<PropNode *>;
  using APSet = BitSet;
  using FormulaID = size_t;
  using FormulaRef = std::vector<ASTNode *>;
  using FormulaSet = BitSet;

  struct State {
    StateID id;
    FormulaSet formulas; // phi not in iff neg phi in
    size_t dup_ind = 0;
  };

  using StateRef = std::vector<State>;
  using StateSet = BitSet;

  StateRef states;
  StateSet initial_states;
  std::map<std::pair<StateID, APSet>, StateSet> delta;
  std::vector<StateSet> accepting_sets;
  StateSet accepting_states;

  Parser &parser;
  ASTNodePtr &rt;
  FormulaRef &subformulas;
  APRef &AP;

  NBA(Parser &parser)
      : parser(parser), rt(parser.ltlrt_), subformulas(parser.node_pool_),
        AP(parser.AP_) {
    // NBA such that L_\omega(NBA) = Words(\neg rt)
    construct_states();
    construct_accepting_sets();
    construct_transitions();
    duplicate_states();
    if (states.size() > BitSet::MAX_SIZE) {
      throw std::runtime_error("Too many states in NBA: " + std::to_string(states.size()));
    }
#ifdef DEBUG
    print();
#endif
  }

  void construct_states();
  void construct_accepting_sets();
  void duplicate_states();
  void construct_transitions();
  FormulaSet compute(const FormulaSet& prop);

  void print() const;

  std::vector<StateID> get_successors(StateID from, const APSet &label) const;

private:
  bool exists_state(StateID id);
  bool is_elementary(const FormulaSet &formulas) const;
  StateID add_state(const FormulaSet &formulas, bool is_initial = false);
  void add_transition(StateID from, const APSet &label, StateID to);
  void add_accepting_set(const StateSet &F_i);
};

#endif // __NBA_H__
