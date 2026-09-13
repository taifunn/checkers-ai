#pragma once

#include "board.hpp"

#include <vector>

namespace movegen {

void generate_moves(
    const Board& board,
    Player player,
    std::vector<Move>& moves
);

void generate_captures(
    const Board& board,
    Player player,
    std::vector<Move>& moves
);

}