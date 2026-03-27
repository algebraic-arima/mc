#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "ast.h"
#include "nba.h"
#include "parser.h"
#include "prod.h"
#include "ts.h"

int main() {
  std::ifstream input_file("ts.txt");
  TS ts(input_file);

  std::ifstream formula_file("ltl.txt");
  int global, local;
  formula_file >> global >> local;
  formula_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  while (global--) {
    try {
      std::string input_formula;
      std::getline(formula_file, input_formula);
      std::istringstream iss(input_formula);
      Lexer lexer(iss);
      Parser parser(lexer, ts);
      parser.parse();
      NBA nba = NBA(parser);
      Prod p = Prod(nba, ts);
      std::cout << (p.persistence_check() ? "1" : "0") << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  while (local--) {
    try {
      std::string input_formula;
      std::getline(formula_file, input_formula);
      std::istringstream iss(input_formula);
      int ind;
      iss >> ind;
      iss >> std::ws;
      Lexer lexer(iss);
      Parser parser(lexer, ts);
      parser.parse();
      NBA nba = NBA(parser);
      Prod p = Prod(nba, ts, ind);
      std::cout << (p.persistence_check() ? "1" : "0") << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
  return 0;
}
