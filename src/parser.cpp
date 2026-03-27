#include "parser.h"

inline int Parser::get_binding_power(TokenType type) {
  switch (type) {
  case TokenType::Implies:
    return 10;
  case TokenType::Or:
    return 20;
  case TokenType::And:
    return 30;
  case TokenType::Until:
  case TokenType::WeakUntil:
  case TokenType::Release:
    return 40;
  case TokenType::Not:
  case TokenType::Next:
  case TokenType::Always:
  case TokenType::Eventually:
    return 50;
  default:
    return -1;
  }
}

void Parser::parse() {
  ltlrt_ = parse_expression();
  if (lexer_.peek().type != TokenType::End) {
    throw std::runtime_error("Unexpected token '" + lexer_.peek().lexeme +
                             "' at position " +
                             std::to_string(lexer_.peek().position));
  }
  ltlrt_ = alloc_node(new UnaryNode(ASTNodeType::Not, ltlrt_));
  AP_.pop_back(); // remove the true node from AP_
#ifdef DEBUG
  print_parse_aux();
  print_ast(ltlrt_);
#endif
}

Parser::ASTNodePtr Parser::alloc_node(Parser::ASTNodePtr node) {
  if (!node)
    return nullptr;
  node->id = node_pool_.size();
  node_pool_.push_back(node);
  return node;
}

Parser::ASTNodePtr Parser::copy(Parser::ASTNodePtr node) {
  if (!node)
    return nullptr;

  switch (node->type) {
  case ASTNodeType::True:
  // case ASTNodeType::False:
  case ASTNodeType::Prop: {
    return node;
  }
  // Unary
  case ASTNodeType::Not:
  // case ASTNodeType::Always:
  // case ASTNodeType::Eventually:
  case ASTNodeType::Next: {
    UnaryNode *u = static_cast<UnaryNode *>(node);
    return alloc_node(new UnaryNode(u->type, copy(u->child)));
  }
  // Binary
  case ASTNodeType::And:
  // case ASTNodeType::Or:
  // case ASTNodeType::Implies:
  // case ASTNodeType::WeakUntil:
  // case ASTNodeType::Release:
  case ASTNodeType::Until: {
    BinaryNode *b = static_cast<BinaryNode *>(node);
    ASTNode *left_copy = copy(b->left);
    ASTNode *right_copy = copy(b->right);
    return alloc_node(new BinaryNode(b->type, left_copy, right_copy));
  }
  default:
    throw std::runtime_error("Unknown ASTNodeType in copy");
  }
}

Parser::ASTNodePtr Parser::parse_prefix(Token token) {
  switch (token.type) {
  case TokenType::Not:
    return alloc_node(new UnaryNode(
        ASTNodeType::Not, parse_expression(get_binding_power(token.type))));
  case TokenType::Next:
    return alloc_node(new UnaryNode(
        ASTNodeType::Next, parse_expression(get_binding_power(token.type))));
  case TokenType::Always: {
    auto right = parse_expression(get_binding_power(token.type));
    auto neg_right = alloc_node(new UnaryNode(ASTNodeType::Not, right));
    auto until_node = alloc_node(new BinaryNode(
        ASTNodeType::Until, alloc_node(new TrueNode()), neg_right));
    return alloc_node(new UnaryNode(ASTNodeType::Not, until_node));
  }
  case TokenType::Eventually: {
    auto right = parse_expression(get_binding_power(token.type));
    return alloc_node(
        new BinaryNode(ASTNodeType::Until, alloc_node(new TrueNode()), right));
  }
  case TokenType::Identifier: {
    if (prop_map_.find(token.lexeme) != prop_map_.end()) {
      return AP_[prop_map_[token.lexeme]];
    }
    auto not_ptr = new UnaryNode(ASTNodeType::Not, AP_[prop_map_["true"]]);
    // for atom props that does not appear in the TS, we treat them as false
    return alloc_node(not_ptr);
  }
  case TokenType::TRUE: {
    return AP_[prop_map_["true"]];
  }
  case TokenType::FALSE: {
    auto not_ptr = new UnaryNode(ASTNodeType::Not, AP_[prop_map_["true"]]);
    return alloc_node(not_ptr);
  }
  case TokenType::LParen: {
    auto node = parse_expression();
    lexer_.consume(TokenType::RParen);
    return node;
  }
  default:
    throw std::runtime_error("Unexpected token '" + token.lexeme +
                             "' at position " + std::to_string(token.position));
  }
}

// The AST may not be a tree since all unit prop nodes are shared
// they must share the same true/false
// but all other nodes are duplicated when needed.
// e.g. if we transform \phi W \psi into \neg(\neg\phi U \neg\psi)
// we will create new nodes for \neg\phi and \neg\psi,
// but reuse the same node for \phi and \psi.
Parser::ASTNodePtr Parser::parse_infix(Parser::ASTNodePtr left, Token op) {
  switch (op.type) {
  case TokenType::And:
    return alloc_node(new BinaryNode{
        ASTNodeType::And, left, parse_expression(get_binding_power(op.type))});
  case TokenType::Or: {
    auto right = parse_expression(get_binding_power(op.type));
    auto neg_left = alloc_node(new UnaryNode(ASTNodeType::Not, left));
    auto neg_right = alloc_node(new UnaryNode(ASTNodeType::Not, right));
    auto and_node =
        alloc_node(new BinaryNode(ASTNodeType::And, neg_left, neg_right));
    return alloc_node(new UnaryNode(ASTNodeType::Not, and_node));
  }
  case TokenType::Implies: {
    auto right = parse_expression(get_binding_power(op.type) - 1);
    auto neg_right = alloc_node(new UnaryNode(ASTNodeType::Not, right));
    auto and_node =
        alloc_node(new BinaryNode(ASTNodeType::And, left, neg_right));
    return alloc_node(new UnaryNode(ASTNodeType::Not, and_node));
  }
  case TokenType::Until:
    return alloc_node(
        new BinaryNode(ASTNodeType::Until, left,
                       parse_expression(get_binding_power(op.type))));
  case TokenType::WeakUntil: {
    auto right = parse_expression(get_binding_power(op.type)); // \psi
    auto neg_right =
        alloc_node(new UnaryNode(ASTNodeType::Not, right)); // \neg\psi
    auto neg_left =
        alloc_node(new UnaryNode(ASTNodeType::Not, left)); // \neg\phi
    auto ll = copy(left);
    auto lr = neg_right;
    auto rl = neg_left;
    auto rr = copy(neg_right);
    auto l = alloc_node(new BinaryNode(ASTNodeType::And, ll, lr));
    auto r = alloc_node(new BinaryNode(ASTNodeType::And, rl, rr));
    auto until_node = alloc_node(new BinaryNode(ASTNodeType::Until, l, r));
    return alloc_node(new UnaryNode(ASTNodeType::Not, until_node));
  }
  case TokenType::Release: {
    auto right = parse_expression(get_binding_power(op.type)); // \psi
    auto neg_right =
        alloc_node(new UnaryNode(ASTNodeType::Not, right)); // \neg\psi
    auto neg_left =
        alloc_node(new UnaryNode(ASTNodeType::Not, left)); // \neg\phi
    auto until_node =
        alloc_node(new BinaryNode(ASTNodeType::Until, neg_left, neg_right));
    return alloc_node(new UnaryNode(ASTNodeType::Not, until_node));
  }
  default:
    throw std::runtime_error("Unexpected operator '" + op.lexeme +
                             "' at position " + std::to_string(op.position));
  }
}

Parser::ASTNodePtr Parser::parse_expression(int min_bp) {
  Token token = lexer_.consume(lexer_.peek().type);
  auto left = parse_prefix(token);

  while (true) {
    Token op = lexer_.peek();
    int bp = get_binding_power(op.type);
    if (bp <= min_bp)
      break;

    lexer_.consume(op.type);
    left = parse_infix(left, op);
  }

  return left;
}

Parser::~Parser() {
  for (auto node : node_pool_) {
    delete node;
  }
}

void Parser::print_parse_aux() {
  for (const auto &node : AP_) {
    std::cout << "(f" << node->id << ") " << node->name << "\n";
  }
}