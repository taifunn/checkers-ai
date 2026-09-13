#pragma once

#include "types.hpp"

#include <cstdint>
#include <string>
#include <vector>

struct Undo {
    uint32_t w_pawns;
    uint32_t b_pawns;
    uint32_t w_queens;
    uint32_t b_queens;

    Player old_turn;
    uint64_t old_hash;
};

class Board {
private:
    Player turn;

    uint64_t hash = 0;

    void gen_normal(Player player, std::vector<Move>& moves) const;

    void gen_jumps(
        int p,
        bool queen,
        Player player,
        uint32_t occupied,
        uint32_t enemies,
        Move move,
        std::vector<Move>& moves
        ) const;

public:
    Board();

    uint32_t b_pawns;
    uint32_t w_pawns;
    uint32_t b_queens;
    uint32_t w_queens;

    void load_board(
        const std::vector<std::string>& lines,
        Player player
        );

    Piece get_piece(int x, int y) const;
    Player current_player() const;

    void gen_moves(Player player, std::vector<Move>& moves) const;
    void gen_captures(Player player, std::vector<Move>& moves) const;

    Undo make_move(const Move& move);
    void unmake_move(const Move& move, const Undo& undo);

    uint64_t get_hash() const;
    void init_hash();
};