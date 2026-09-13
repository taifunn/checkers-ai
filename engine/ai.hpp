#pragma once

#include "board.hpp"

#include <vector>

void init_move_lists();

Move ai_move(
    Board &board,
    Player player,
    const std::vector<Move>& moves,
    int depth
    );