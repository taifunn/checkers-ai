#pragma once

#include <cstdint>

enum class Piece : uint8_t {
    empty,
    w_pawn,
    b_pawn,
    w_queen,
    b_queen
};

enum class Player : uint8_t {
    white,
    black
};

struct Move {
    uint8_t from;
    uint8_t to;

    uint32_t captured = 0;

    uint64_t path = 0;
    uint8_t path_count = 0;

    Move(int p)
        : from(p),
        to(p),
        captured(0),
        path(0),
        path_count(0)
    {}
};