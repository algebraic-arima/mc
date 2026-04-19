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

inline size_t hash_combine(size_t lhs, size_t rhs) {
  lhs ^= rhs + 0x9e3779b97f4a7c15ULL + (lhs << 6) + (lhs >> 2);
  return lhs;
}

inline size_t hash2(size_t a, size_t b) { return hash_combine(a, b); }

inline size_t hash3(size_t a, size_t b, size_t c) {
  return hash_combine(hash_combine(a, b), c);
}

struct ASTNode {
  int id = -1;
  ASTNodeType type;
  size_t hash = 0;
  virtual ~ASTNode() = default;

  explicit ASTNode(ASTNodeType t) : type(t) {}
};

struct PropNode : ASTNode {
  std::string name;
  explicit PropNode(std::string n)
      : ASTNode(ASTNodeType::Prop), name(std::move(n)) {
    this->hash = std::hash<std::string>{}(this->name);
  }
};

struct TrueNode : PropNode {
  TrueNode() : PropNode("true") {
    this->type = ASTNodeType::True;
    this->hash = std::hash<std::string>{}(this->name);
  }
};

struct UnaryNode : ASTNode {
  ASTNode *child;

  UnaryNode(ASTNodeType t, ASTNode *c) : ASTNode(t), child(std::move(c)) {
    this->hash = hash_combine(std::hash<int>{}(static_cast<int>(t)), c->hash);
  }
};

struct BinaryNode : ASTNode {
  ASTNode *left;
  ASTNode *right;

  BinaryNode(ASTNodeType t, ASTNode *l, ASTNode *r)
      : ASTNode(t), left(std::move(l)), right(std::move(r)) {
        if (t == ASTNodeType::And) {
          size_t min = std::min(left->hash, right->hash);
          size_t max = std::max(left->hash, right->hash);
          this->hash = hash2(min, max);
        } else {
          this->hash = hash3(std::hash<int>{}(static_cast<int>(t)), left->hash,
                             right->hash);
        }
      }
};

void print_ast(ASTNode *node, int indent = 0);

#endif // __AST_H__