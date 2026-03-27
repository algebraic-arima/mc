#include "ast.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

void print_ast(ASTNode *node, int indent) {
  if (!node)
    return;

  int width = 3;
  std::ostringstream oss;
  oss << "(f" << std::setw(width) << std::setfill(' ') << node->id << ")"
      << std::string(indent * 2, ' ');
  std::string ind = oss.str();

  switch (node->type) {
  case ASTNodeType::True:
    std::cout << ind << "True\n";
    break;
  // case ASTNodeType::False:
  //   std::cout << ind << "False\n";
  //   break;
  case ASTNodeType::Prop: {
    PropNode *p = static_cast<PropNode *>(node);
    std::cout << ind << "Prop(" << p->name << ")\n";
    break;
  }
  case ASTNodeType::Not:
  // case ASTNodeType::Always:
  // case ASTNodeType::Eventually:
  case ASTNodeType::Next: {
    UnaryNode *u = static_cast<UnaryNode *>(node);
    std::cout << ind << "Unary(";
    switch (node->type) {
    case ASTNodeType::Not:
      std::cout << "¬";
      break;
    case ASTNodeType::Next:
      std::cout << "○";
      break;
    // case ASTNodeType::Always:
    //   std::cout << "□";
    //   break;
    // case ASTNodeType::Eventually:
    //   std::cout << "◇";
    //   break;
    default:
      break;
    }
    std::cout << ")\n";
    print_ast(u->child, indent + 1);
    break;
  }
  case ASTNodeType::And:
  // case ASTNodeType::Or:
  // case ASTNodeType::Implies:
  // case ASTNodeType::WeakUntil:
  // case ASTNodeType::Release:
  case ASTNodeType::Until: {
    BinaryNode *b = static_cast<BinaryNode *>(node);
    std::cout << ind << "Binary(";
    switch (node->type) {
    case ASTNodeType::And:
      std::cout << "∧";
      break;
    // case ASTNodeType::Or:
    //   std::cout << "∨";
    //   break;
    // case ASTNodeType::Implies:
    //   std::cout << "->";
    //   break;
    case ASTNodeType::Until:
      std::cout << "U";
      break;
    // case ASTNodeType::WeakUntil:
    //   std::cout << "W";
    //   break;
    // case ASTNodeType::Release:
    //   std::cout << "R";
    //   break;
    default:
      break;
    }
    std::cout << ")\n";
    print_ast(b->left, indent + 1);
    print_ast(b->right, indent + 1);
    break;
  }
  default:
    std::cout << ind << "UnknownNode\n";
    break;
  }
}