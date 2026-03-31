# ltlmc

The program reads:
- A transition system (TS) from `ts.txt`
- A list of LTL formulas from `ltl.txt`

For each formula, it builds a automaton (`NBA`) for the negated-formula and checks whether an accepting cycle exists in the product graph (`TS x NBA`).

It prints one line per formula of `1` if $TS\vDash \varphi$ and `0` otherwise.

## What Is Implemented

The pipeline in `src/main.cpp` is:
1. Parse `ts.txt` into `TS`
2. Parse each LTL formula into an AST (`Parser`)
3. Convert AST to an NBA (`NBA`)
4. Build on-the-fly product exploration (`Prod`)
5. Run nested DFS-style cycle detection (`persistence_check`)

Core files:
- `src/main.cpp`: Program entry and batch processing for formulas
- `src/ts.cpp`, `include/ts.h`: Transition system parsing and successor queries
- `src/lexer.cpp`, `include/lexer.h`: Formula lexer
- `src/parser.cpp`, `include/parser.h`: Pratt parser and operator desugaring
- `src/nba.cpp`, `include/nba.h`: NBA construction. The order is adjusted so that I can generate the NBA in a single pass
- `src/prod.cpp`, `include/prod.h`: Product graph on-the-fly and persistence check
- `src/ast.cpp`, `include/ast.h`: AST node types

## Build

Build commands:

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/ltlmc
```

The executable expects `ts.txt` and `ltl.txt` in the current working directory.

## Input Format

### `ts.txt`

1. First line: `S T`
2. Second line: initial state indices (space-separated)
3. Third line: action IDs (read but currently not used in transition relation)
4. Fourth line: AP names (space-separated), saved as `TS::prop`
5. Next `T` lines: `from action to`
6. Next `S` lines: AP index set for each state label `L(s)`; `-1` means empty set

### `ltl.txt`

1. First line: `global_count local_count`
2. Next `global_count` lines: one formula per line
3. Next `local_count` lines: `init_index formula`

## Supported LTL Syntax

Lexical tokens are defined in `lexer.cpp` and parsed in `parser.cpp`. The pratt parser is used.

Atomic propositions:
- Lowercase letters/digits, e.g. `a`, `p0`, `x12`

Constants:
- `true`, `false`

Operators:
- Unary: `!` (not), `X`, `G`, `F`
- Binary: `/\\` (and), `\\/` (or), `->`, `U`, `W`, `R`

Parentheses:
- `(` and `)`

Binding power (high to low):
- Unary (`!`, `X`, `G`, `F`)
- Temporal binary (`U`, `W`, `R`)
- `/\`
- `\/`
- `->`

The parser reduces operators to a smaller AST basis (`True`, `Prop`, `Not`, `Next`, `And`, `Until`):
- `false` becomes `!true`
- `F φ` becomes `true U φ`
- `G φ` becomes `!(true U !φ)`
- `φ \/ ψ` becomes `!(!φ /\\ !ψ)`
- `φ -> ψ` becomes `!(φ /\\ !ψ)`
- `φ W ψ` and `φ R ψ` are rewritten using `U` and `!`

After parsing, the root formula is negated (`!φ`) before NBA construction.

# Ideas and Implementations

## Lexer and Parser

The lexer tokenizes the input formula string into a stream of tokens. The parser uses a Pratt parsing approach, where each token has a binding power that determines how it interacts with other tokens. 

The parser also performs desugaring of operators into a smaller set of core AST nodes, i.e. `And`, `Not`, `Next`, `Until`, `True`, and `Prop`. This simplifies the subsequent NBA construction.

## From LTL formula to NBA

The implementation is modified from the textbook algorithms. The main idea is to construct the NBA directly from the AST, without first constructing a GNBA and then converting it to an NBA.

We can find this code fragment from `include/nba.h`:

```cpp
construct_states();
construct_accepting_sets();
construct_transitions();
duplicate_states();
```

Here we simplify that the NBA construction is done in a single pass over the AST. 

First we construct the legal states that correspond to the subset of subformulas that can be satisfied at the same time. While the notes says that we should consider the closure of the formula, we can actually use just the subformulas that appear in the original AST. This is correct because of the maximality of the elementary sets. Thus we denote the states by bitsets (The nodes of AST is arranged in a linear order, and we assign each node an index. The i-th bit indicates whether the i-th subformula is included in the set $B$).

For efficiency, we are not enumerating all the subsets of subformulas and checking their consistency. Instead, we enumerate all evaluations of the `Prop`, `Next` and `Until` nodes. From the evaluation of these nodes, we can compute the evaluation of all other nodes by a single pass over the AST. If the evaluation is consistent, we construct a state for it.

Then we construct acepting states, which is the same to the notes.

Third, we construct the transitions according to the semantics of the operators. Here we directly construct the transitions of the NBA, without first constructing a GNBA and then converting it to an NBA. The function `void NBA::add_transition(StateID from, const APSet &label, StateID to)` is called whenever we find that there is a legal transition in the GNBA from state `from` to state `to` under the label `label`. The function adds a transition not in the GNBA but in the NBA. Here is the code fragment from this function, which shows the logic to translate GNBA transitions to NBA transitions, with $k$ being the number of accepting sets, and $n$ being the number of states in the original GNBA:

```cpp
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
```

Finally, we duplicate states to make sure that the NBA is complete. We simply copy them because the transitions are all set.

## Product of TS and NBA

The product transition system is implicitly constructed. When moving to new successors, we call the function `Prod::StateSet Prod::step(const StateID &prod)`.

## Persistence Check

Pure textbook algorithms.
