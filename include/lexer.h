#pragma once
#include <iostream>
#include <queue>
#ifndef __LEXER_H__
#define __LEXER_H__

enum class TokenType {
  End,
  LParen,
  RParen,

  // unary
  Not,
  Next,
  Always,
  Eventually,

  // binary
  And,
  Or,
  Implies,
  Until,
  WeakUntil,
  Release,

  // atom
  Identifier, // numerals and lowercase letters
  TRUE,
  FALSE
};

class Token {
public:
  TokenType type;
  std::string lexeme;
  std::size_t position;
  Token(TokenType t, std::string l, std::size_t p)
      : type(t), lexeme(std::move(l)), position(p) {}
};

class Lexer {
  std::istream &input_;
  std::size_t pos_;
  std::queue<Token> token_buffer_;
  inline void update_pos(char c);
  Token next_token();

public:
  explicit Lexer(std::istream &is) : input_(is), pos_(0) {}
  Token &peek();
  Token consume(TokenType expected);
};

#endif // __LEXER_H__
