#include "nba.h"
#include <cassert>

void NBA::print() const {
  std::cout << "States:\n";
  for (const auto &state : states) {
    std::cout << "s" << state.id << ": ({";
    for (size_t i = 0; i < subformulas.size(); ++i) {
      if (state.formulas.test(i)) {
        std::cout << "f" << i << " ";
      }
    }
    std::cout << "}, " << state.dup_ind << ")\n";
  }

  std::cout << "\nInitial States: {";
  for (size_t i = 0; i < states.size(); ++i) {
    if (initial_states.test(i)) {
      std::cout << "s" << i << " ";
    }
  }
  std::cout << "}\n";

  std::cout << "\nTransitions:\n";
  for (const auto &[key, succ] : delta) {
    auto [from, label] = key;
    std::cout << "s" << from << " --{ ";
    for (size_t i = 0; i < AP.size(); ++i) {
      if (label.test(i)) {
        std::cout << AP[i]->name << " ";
      }
    }
    std::cout << "}--> {";
    for (size_t i = 0; i < states.size(); ++i) {
      if (succ.test(i)) {
        std::cout << "s" << i << " ";
      }
    }
    std::cout << "}\n";
  }

  std::cout << "\nAccepting States: { ";
  for (size_t j = 0; j < states.size(); ++j) {
    if (accepting_states.test(j)) {
      std::cout << "s" << j << " ";
    }
  }
  std::cout << "}\n" << std::endl;
}

bool NBA::exists_state(StateID id) {
  return id >= 0 && id < static_cast<StateID>(states.size());
}

NBA::StateID NBA::add_state(const FormulaSet &formulas, bool is_initial) {
  StateID new_id = states.size();
  states.push_back({new_id, formulas});
  if (is_initial) {
    initial_states.set(new_id);
  }
  return new_id;
}

void NBA::add_transition(StateID from, const APSet &label, StateID to) {
  if (!exists_state(from) || !exists_state(to)) {
    throw std::runtime_error("Invalid state ID in transition.");
  }
#ifdef DEBUG
  std::cout << "Adding transition on GNBA: s" << from << " --{ ";
  for (size_t i = 0; i < AP.size(); ++i) {
    if (label.test(i)) {
      std::cout << AP[i]->name << " ";
    }
  }
  std::cout << "}--> s" << to << std::endl;
#endif
  size_t k = accepting_sets.size();
  if (k == 0) {
    delta[{from, label}].set(to); // GNBA construction
    return;
  }
  size_t n = states.size();
  for (size_t j = 0; j < k; ++j) {
    if (accepting_sets[j].test(from)) {
      size_t new_ind = (j + 1) % k;
      size_t f = from + j * n;
      size_t t = to + new_ind * n;
      delta[{f, label}].set(t);
    } else {
      size_t f = from + j * n;
      size_t t = to + j * n;
      delta[{f, label}].set(t);
    }
  } // we directly add the transitions of the NBA
}

void NBA::add_accepting_set(const StateSet &F_i) {
  accepting_sets.push_back(F_i);
}

std::vector<NBA::StateID> NBA::get_successors(StateID from,
                                              const APSet &label) const {
  std::vector<StateID> successors;
  auto it = delta.find({from, label});
  if (it != delta.end()) {
    const StateSet &succ_set = it->second;
    for (size_t i = 0; i < states.size(); ++i) {
      if (succ_set.test(i)) {
        successors.push_back(i);
      }
    }
  }
  return successors;
}

bool NBA::is_elementary(const FormulaSet &formulas) const {
  for (int i = 0; i < subformulas.size(); ++i) {
    if (formulas.test(i)) {
      if (subformulas[i]->type == ASTNodeType::Not) {
        UnaryNode *not_node = static_cast<UnaryNode *>(subformulas[i]);
        int child_id = not_node->child->id;
        if (formulas.test(child_id)) {
          return false;
        }
      } else if (subformulas[i]->type == ASTNodeType::And) {
        BinaryNode *bin_node = static_cast<BinaryNode *>(subformulas[i]);
        int left_id = bin_node->left->id;
        int right_id = bin_node->right->id;
        if (!formulas.test(left_id) || !formulas.test(right_id)) {
          return false;
        }
      } else if (subformulas[i]->type == ASTNodeType::Until) {
        BinaryNode *bin_node = static_cast<BinaryNode *>(subformulas[i]);
        int left_id = bin_node->left->id;
        int right_id = bin_node->right->id;
        if (!formulas.test(right_id) && !formulas.test(left_id)) {
          return false;
        }
      }
    } else {
      if (subformulas[i]->type == ASTNodeType::Not) {
        UnaryNode *not_node = static_cast<UnaryNode *>(subformulas[i]);
        int child_id = not_node->child->id;
        if (!formulas.test(child_id)) {
          return false;
        }
      } else if (subformulas[i]->type == ASTNodeType::And) {
        BinaryNode *bin_node = static_cast<BinaryNode *>(subformulas[i]);
        int left_id = bin_node->left->id;
        int right_id = bin_node->right->id;
        if (formulas.test(right_id) && formulas.test(left_id)) {
          return false;
        }
      } else if (subformulas[i]->type == ASTNodeType::Until) {
        BinaryNode *bin_node = static_cast<BinaryNode *>(subformulas[i]);
        int left_id = bin_node->left->id;
        int right_id = bin_node->right->id;
        if (formulas.test(right_id)) {
          return false;
        }
      } else if (subformulas[i]->type == ASTNodeType::True) {
        return false;
      }
    }
  }
  return true;
}

void NBA::construct_states() {
  int n = parser.enumerated.size();
#ifdef DEBUG
  for (auto &s : parser.enumerated) {
    std::cout << s->id << std::endl;
  }
#endif
  FormulaSet formulas;
  FormulaSet e;
  for (unsigned long mask = 0; mask < (1UL << n); ++mask) {
    auto ans = compute(formulas);
    if (!ans.back()) {
      add_state(ans, ans.test(parser.ltlrt_->id));
    }
    for (size_t i = 0; i < n; ++i) {
      if (e.test(i)) {
        e.bits.reset(i);
        formulas.bits.reset(parser.enumerated[i]->id);
      } else {
        e.bits.set(i);
        formulas.bits.set(parser.enumerated[i]->id);
        break;
      }
    }
  }
}

NBA::FormulaSet NBA::compute(const NBA::FormulaSet &prop) {
  FormulaSet visited;
  FormulaSet ans = prop;
  std::vector<FormulaID> S;
  S.push_back(parser.ltlrt_->id);
  do {
    FormulaID ss = S.back();
    switch (parser.node_pool_[ss]->type) {
    case ASTNodeType::Prop: {
      S.pop_back();
      break;
    }
    case ASTNodeType::True: {
      S.pop_back();
      ans.bits.set(ss);
      break;
    }
    case ASTNodeType::Not: {
      UnaryNode *not_ptr = static_cast<UnaryNode *>(parser.node_pool_[ss]);
      FormulaID child_id = not_ptr->child->id;
      if (!visited.test(child_id)) {
        S.push_back(child_id);
        visited.bits.set(child_id);
      } else {
        S.pop_back();
        visited.bits.set(ss);
        ans.bits.set(ss, !ans.test(child_id));
      }
      break;
    }
    case ASTNodeType::And: {
      BinaryNode *and_ptr = static_cast<BinaryNode *>(parser.node_pool_[ss]);
      FormulaID left_id = and_ptr->left->id;
      FormulaID right_id = and_ptr->right->id;
      if (!visited.test(left_id)) {
        S.push_back(left_id);
        visited.bits.set(left_id);
      } else if (!visited.test(right_id)) {
        S.push_back(right_id);
        visited.bits.set(right_id);
      } else {
        S.pop_back();
        visited.bits.set(ss);
        ans.bits.set(ss, ans.test(left_id) && ans.test(right_id));
      }
      break;
    }
    case ASTNodeType::Until: {
      BinaryNode *until_ptr = static_cast<BinaryNode *>(parser.node_pool_[ss]);
      FormulaID left_id = until_ptr->left->id;
      FormulaID right_id = until_ptr->right->id;
      if (!visited.test(left_id)) {
        S.push_back(left_id);
        visited.set(left_id);
      } else if (!visited.test(right_id)) {
        S.push_back(right_id);
        visited.set(right_id);
      } else {
        S.pop_back();
        bool left_val = ans.test(left_id);
        bool right_val = ans.test(right_id);
        bool val = prop.test(ss);
        if (right_val && !val) {
          ans.bits.set();
          return ans; // fails
        } else if (val && !right_val && !left_val) {
          ans.bits.set();
          return ans; // fails
        }
      }
      break;
    }
    case ASTNodeType::Next: {
      UnaryNode *next_ptr = static_cast<UnaryNode *>(parser.node_pool_[ss]);
      FormulaID child_id = next_ptr->child->id;
      if (!visited.test(child_id)) {
        S.push_back(child_id);
        visited.bits.set(child_id);
      } else {
        S.pop_back();
      }
      break;
    }
    }
  } while (!S.empty());
  return ans;
}

void NBA::duplicate_states() {
  if (!accepting_sets.empty()) {
    accepting_states = accepting_sets[0];
  } else {
    accepting_states.bits.set();
  }
  if (accepting_sets.size() <= 1) {
    return;
  }
  size_t k = accepting_sets.size();
  size_t n = states.size();
  for (size_t j = 1; j < k; ++j) {
    for (size_t i = 0; i < n; ++i) {
      StateID new_id = states.size();
      assert(new_id == j * n + i);
      states.push_back({new_id, states[i].formulas, j});
    }
  }
}

void NBA::construct_transitions() {
  for (const auto &state : states) {
    APSet transition;
    for (size_t i = 0; i < AP.size(); ++i) {
      if (state.formulas.test(AP[i]->id)) {
        transition.set(i);
      }
    }
    FormulaSet expected_next_contains;
    for (size_t i = 0; i < subformulas.size(); ++i) {
      if (state.formulas.test(i)) {
        ASTNode *node = subformulas[i];
        if (node->type == ASTNodeType::Next) {
          UnaryNode *next_node = static_cast<UnaryNode *>(node);
          int child_id = next_node->child->id;
          expected_next_contains.set(child_id);
        } else if (node->type == ASTNodeType::Until) {
          // should satisfy 1U2\inB -> 2\in B or (1\inB and 1U2\inB')
          BinaryNode *until_node = static_cast<BinaryNode *>(node);
          int left_id = until_node->left->id;
          int right_id = until_node->right->id;
          if (!state.formulas.test(right_id)) {
            // if 2\notin B, by elementary, 1\in B (naturally)
            // then we only have to make sure 1U2\in B'
            expected_next_contains.set(i);
          }
        }
      }
    }
    FormulaSet expected_next_excludes;
    for (size_t i = 0; i < subformulas.size(); ++i) {
      if (!state.formulas.test(i)) {
        ASTNode *node = subformulas[i];
        if (node->type == ASTNodeType::Next) {
          UnaryNode *next_node = static_cast<UnaryNode *>(node);
          int child_id = next_node->child->id;
          expected_next_excludes.set(child_id);
        } else if (node->type == ASTNodeType::Until) {
          // should satisfy 1U2\notinB -> 2\notin B and (1\notinB or
          // 1U2\notinB')
          BinaryNode *until_node = static_cast<BinaryNode *>(node);
          int left_id = until_node->left->id;
          int right_id = until_node->right->id;
          if (state.formulas.test(left_id)) {
            // if 1U2\notin B, by elementary, 2\notin B (naturally)
            // then we only have to make sure if 1\inB then 1U2\notin B'
            expected_next_excludes.set(i);
          }
        }
      }
    }
    for (const auto &target : states) {
      if ((target.formulas & expected_next_contains) ==
              expected_next_contains &&
          (target.formulas & expected_next_excludes).bits.none()) {
        add_transition(state.id, transition, target.id);
      }
    }
  }
}

void NBA::construct_accepting_sets() {
  for (size_t i = 0; i < subformulas.size(); ++i) {
    if (subformulas[i]->type == ASTNodeType::Until) {
      StateSet F_i;
      BinaryNode *until_node = static_cast<BinaryNode *>(subformulas[i]);
      auto right_id = until_node->right->id;
      for (const auto &state : states) {
        if (!(state.formulas.test(i) && !state.formulas.test(right_id))) {
          F_i.set(state.id);
        }
      }
      add_accepting_set(F_i);
    }
  }
#ifdef DEBUG
  std::cout << "Accepting sets of states: \n";
  for (size_t j = 0; j < accepting_sets.size(); ++j) {
    std::cout << "F" << j << ": { ";
    for (size_t i = 0; i < states.size(); ++i) {
      if (accepting_sets[j].test(i)) {
        std::cout << "s" << i << " ";
      }
    }
    std::cout << "}\n";
  }
#endif
}
