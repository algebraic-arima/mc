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

