# Checkers Engine

  

A checkers engine written in **C++17**, with a simple **Qt** interface for playing against the AI.

The main focus of the project is the game engine: efficient board representation, move generation and adversarial search.


## Usage

### Requirements

* C++17 compatible compiler

* CMake 3.16+

* Qt5 or Qt6 with the `Widgets` module

### Build

```bash

git clone <https://github.com/taifunn/checkers-ai>

cd checkers_ai

  

cmake -S . -B build

cmake --build build

```

  

Run the application:

  

```bash

./build/checkers_ai

```

  

The AI search depth can be selected from the application interface.

  

## Bitboards

  

The engine represents the board using four **32-bit bitboards**:

  

```cpp

uint32_t b_pawns;

uint32_t w_pawns;

uint32_t b_queens;

uint32_t w_queens;

```

  

Since only 32 squares of a checkers board are playable, each bit directly represents one playable square.

  

This allows operations such as occupancy checks and piece manipulation to be performed efficiently using bitwise operations.

  

## Move Generation

  

Legal moves are generated directly from the current board state.

  

The move generator supports:

  

* regular pawn moves

* king moves

* mandatory captures

* multi-jump capture sequences

* promotion to kings

  

When a capture is available, regular moves are not generated.

  

## Minimax

  

The AI chooses moves using the **Minimax algorithm**.

  

The search recursively explores possible moves for both players, assuming that each side always chooses the best available continuation.

  

The final position reached by the search is assigned a score using the evaluation function.

  

## Alpha-Beta Pruning

  

**Alpha-Beta Pruning** is used to reduce the number of positions evaluated by Minimax.

  

Branches that cannot influence the final result are skipped, allowing the engine to search deeper without changing the result of the Minimax algorithm.

  

## Evaluation Function

  

When the maximum search depth is reached, the engine evaluates the current board position.

  

The evaluation considers factors such as:

  

* material advantage

* pawn advancement

* center control

* promotion potential

* back-rank protection

  

The resulting score represents how favorable the position is for the AI.

  

## Quiescence Search

  

The engine uses **Quiescence Search** when the regular Minimax depth limit is reached.

  

Instead of immediately evaluating an unstable position, the engine continues searching capture sequences until a quieter position is reached.

  

This helps reduce the **horizon effect**, where an important tactical consequence occurs just beyond the normal search depth.

  

## Zobrist Hashing

  

Board positions are identified using **Zobrist Hashing**.

  

Each combination of piece type and board square is assigned a random value. The hash of the current position is created using XOR operations.

  

This provides a fast way to identify previously visited board states during the search.

  

## Transposition Table

  

The engine stores previously evaluated positions in a **Transposition Table**.

  

The table uses the Zobrist hash as the position identifier and stores information such as:

  

* position hash

* search depth

* evaluation score

* bound type

  

When the same position appears again during the search, the engine can reuse previously calculated information instead of evaluating the entire subtree again.

  

This is especially useful because identical board positions can often be reached through different move sequences.
