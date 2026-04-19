#pragma once

#ifndef __PARSER_H__
#define __PARSER_H__

#include <map>
#include <memory>
#include <string>

#include "ast.h"
#include "lexer.h"
#include "ts.h"

class Parser {
  Lexer &lexer_;
  using ASTNodePtr = ASTNode *;

  ASTNodePtr parse_expression(int min_bp = 0);
  ASTNodePtr parse_prefix(Token);
  ASTNodePtr parse_infix(ASTNodePtr left, Token op);
  ASTNodePtr alloc_node(ASTNodePtr node);

  ASTNodePtr copy(ASTNodePtr node);

  std::map<size_t, int> node_map_; // from hash to node index

  inline int get_binding_power(TokenType type);

public:
  ASTNodePtr ltlrt_;
  std::vector<ASTNodePtr> node_pool_; // all ASTNodes
  std::vector<PropNode *> AP_;        // all atomic propositions
  std::map<std::string, size_t> prop_map_; // from proposition name to index in AP_
  std::vector<ASTNodePtr> enumerated; // nodes that need to be enumerated for state construction
  explicit Parser(Lexer &lexer, TS &ts) : lexer_(lexer) {
    for (size_t i = 0; i < ts.prop.size(); ++i) {
      prop_map_[ts.prop[i]] = i; 
      // we know all the propositions in the TS; those not appearing are considered false
      auto ptr = new PropNode(ts.prop[i]);
      AP_.push_back(ptr);
      alloc_node(ptr);
    }
    prop_map_["true"] = ts.prop.size(); // for true node
    auto ptr = new TrueNode();
    AP_.push_back(ptr); // after the construction, we will pop the true node
    alloc_node(ptr);
  };
  void parse();
  void print_parse_aux();
  ~Parser();
};

#endif // __PARSER_H__
