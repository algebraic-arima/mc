#include "lexer.h"

inline void Lexer::update_pos(char c) { pos_++; }

Token Lexer::next_token() {
  while (std::isspace(input_.peek())) {
    update_pos(input_.get());
  }

  if (input_.eof()) {
    return Token(TokenType::End, "", pos_);
  }

  char c = input_.peek();
  if (c == '(') {
    update_pos(input_.get());
    return Token(TokenType::LParen, "(", pos_ - 1);
  } else if (c == ')') {
    update_pos(input_.get());
    return Token(TokenType::RParen, ")", pos_ - 1);
  } else if (c == '!') {
    update_pos(input_.get());
    return Token(TokenType::Not, "¬", pos_ - 1);
  } else if (c == 'X') {
    update_pos(input_.get());
    return Token(TokenType::Next, "○", pos_ - 1);
  } else if (c == 'G') {
    update_pos(input_.get());
    return Token(TokenType::Always, "□", pos_ - 1);
  } else if (c == 'F') {
    update_pos(input_.get());
    return Token(TokenType::Eventually, "◇", pos_ - 1);
  } else if (c == 'U') {
    update_pos(input_.get());
    return Token(TokenType::Until, "U", pos_ - 1);
  } else if (c == 'W') {
    update_pos(input_.get());
    return Token(TokenType::WeakUntil, "W", pos_ - 1);
  } else if (c == 'R') {
    update_pos(input_.get());
    return Token(TokenType::Release, "R", pos_ - 1);
  } else if (c == '/') {
    update_pos(input_.get());
    if (input_.peek() == '\\') {
      update_pos(input_.get());
      return Token(TokenType::And, "∧", pos_ - 2);
    } else {
      throw std::runtime_error("Unrecognized character '/' at position " +
                               std::to_string(pos_ - 1));
    }
  } else if (c == '\\') {
    update_pos(input_.get());
    if (input_.peek() == '/') {
      update_pos(input_.get());
      return Token(TokenType::Or, "∨", pos_ - 2);
    } else {
      throw std::runtime_error("Unrecognized character '\\' at position " +
                               std::to_string(pos_ - 1));
    }
  } else if (c == '-') {
    update_pos(input_.get());
    if (input_.peek() == '>') {
      update_pos(input_.get());
      return Token(TokenType::Implies, "->", pos_ - 2);
    } else {
      throw std::runtime_error("Unrecognized character '-' at position " +
                               std::to_string(pos_ - 1));
    }
  } else if (std::islower(c) || std::isdigit(c)) {
    std::string lexeme;
    lexeme.reserve(8);
    while (std::islower(input_.peek()) || std::isdigit(input_.peek())) {
      lexeme += input_.peek();
      update_pos(input_.get());
    }
    if (lexeme == "true") {
      return Token(TokenType::TRUE, lexeme, pos_ - lexeme.size());
    } else if (lexeme == "false") {
      return Token(TokenType::FALSE, lexeme, pos_ - lexeme.size());
    }
    return Token(TokenType::Identifier, lexeme, pos_ - lexeme.size());
  }

  // Handle unrecognized characters
  std::string unrecognized(1, c);
  input_.get();
  throw std::runtime_error("Unrecognized character '" + unrecognized +
                           "' at position " + std::to_string(pos_));
}

Token &Lexer::peek() {
  if (token_buffer_.empty()) {
    token_buffer_.push(std::move(next_token()));
  }
  return token_buffer_.front();
}

Token Lexer::consume(TokenType expected) {
  if (token_buffer_.empty()) {
    Token token = next_token();
    if (token.type != expected) {
      throw std::runtime_error(
          "Expected token type " + std::to_string(static_cast<int>(expected)) +
          " but got " + std::to_string(static_cast<int>(token.type)) +
          " at position " + std::to_string(token.position));
    }
    return token;
  }

  Token token = std::move(token_buffer_.front());
  token_buffer_.pop();
  if (token.type != expected) {
    throw std::runtime_error(
        "Expected token type " + std::to_string(static_cast<int>(expected)) +
        " but got " + std::to_string(static_cast<int>(token.type)) +
        " at position " + std::to_string(token.position));
  }
  return token;
}