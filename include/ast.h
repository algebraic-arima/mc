#pragma once
#include "lexer.h"
#include <memory>
#include <vector>

#ifndef __AST_H__
#define __AST_H__

enum class ASTNodeType {
  True,
  // False,
  Prop,
  // unary
  Not,
  Next,
  // Always,
  // Eventually,
  // binary
  And,
  // Or,
  // Implies,
  Until,
  // WeakUntil,
  // Release
};

struct ASTNode {
  int id = -1;
  ASTNodeType type;
  virtual ~ASTNode() = default;
  
  explicit ASTNode(ASTNodeType t) : type(t) {}

};

struct PropNode : ASTNode {
  std::string name;
  explicit PropNode(std::string n) : ASTNode(ASTNodeType::Prop), name(std::move(n)) {}
};

struct TrueNode : PropNode {
  TrueNode() : PropNode("true") {
    this->type = ASTNodeType::True;
  }
};

struct UnaryNode : ASTNode {
  ASTNode* child;
  
  UnaryNode(ASTNodeType t, ASTNode* c)
      : ASTNode(t), child(std::move(c)) {}
};

struct BinaryNode : ASTNode {
  ASTNode* left;
  ASTNode* right;
  
  BinaryNode(ASTNodeType t, ASTNode* l, ASTNode* r)
      : ASTNode(t), left(std::move(l)), right(std::move(r)) {}
};

void print_ast(ASTNode *node, int indent = 0);

#endif // __AST_H__