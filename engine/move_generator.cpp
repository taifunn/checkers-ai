#include "board.hpp"

#include <cstdint>
#include <vector>

using namespace std;

constexpr uint32_t EVEN_ROWS  = 0x0F0F0F0F;
constexpr uint32_t ODD_ROWS   = 0xF0F0F0F0;
constexpr uint32_t LEFT_EDGE  = 0x11111111;
constexpr uint32_t RIGHT_EDGE = 0x88888888;

void Board::gen_normal(Player player, vector<Move>& moves) const{

    auto add_moves = [&](uint32_t dest, int shift){
        while(dest){
            int to = __builtin_ctz(dest);
            int from = to - shift;

            Move move(from);
            move.to = to;

            moves.push_back(move);

            dest &= dest - 1;
        }
    };

    uint32_t occupied = w_pawns | b_pawns | w_queens | b_queens;
    uint32_t empty = ~occupied;

    uint32_t pawns =
        (player == Player::white) ? w_pawns : b_pawns;

    uint32_t queens =
        (player == Player::white) ? w_queens : b_queens;

    if(player == Player::white) {
        uint32_t src = pawns & EVEN_ROWS;
        uint32_t dest = (src << 4) & empty;
        add_moves(dest, 4);

        src = pawns & ODD_ROWS & ~LEFT_EDGE;
        dest = (src << 3) & empty;
        add_moves(dest, 3);

        src = pawns & EVEN_ROWS & ~RIGHT_EDGE;
        dest = (src << 5) & empty;
        add_moves(dest, 5);

        src = pawns & ODD_ROWS;
        dest = (src << 4) & empty;
        add_moves(dest, 4);
    }
    else{
        uint32_t src = pawns & EVEN_ROWS;
        uint32_t dest = (src >> 4) & empty;
        add_moves(dest, -4);

        src = pawns & ODD_ROWS & ~LEFT_EDGE;
        dest = (src >> 5) & empty;
        add_moves(dest, -5);

        src = pawns & EVEN_ROWS & ~RIGHT_EDGE;
        dest = (src >> 3) & empty;
        add_moves(dest, -3);

        src = pawns & ODD_ROWS;
        dest = (src >> 4) & empty;
        add_moves(dest, -4);
    }

    uint32_t src = queens & EVEN_ROWS;
    uint32_t dest = (src << 4) & empty;
    add_moves(dest, 4);

    src = queens & ODD_ROWS & ~LEFT_EDGE;
    dest = (src << 3) & empty;
    add_moves(dest, 3);

    src = queens & EVEN_ROWS & ~RIGHT_EDGE;
    dest = (src << 5) & empty;
    add_moves(dest, 5);

    src = queens & ODD_ROWS;
    dest = (src << 4) & empty;
    add_moves(dest, 4);

    src = queens & EVEN_ROWS;
    dest = (src >> 4) & empty;
    add_moves(dest, -4);

    src = queens & ODD_ROWS & ~LEFT_EDGE;
    dest = (src >> 5) & empty;
    add_moves(dest, -5);

    src = queens & EVEN_ROWS & ~RIGHT_EDGE;
    dest = (src >> 3) & empty;
    add_moves(dest, -3);

    src = queens & ODD_ROWS;
    dest = (src >> 4) & empty;
    add_moves(dest, -4);
}

void Board::gen_jumps(
    int p,
    bool queen,
    Player player,
    uint32_t occupied,
    uint32_t enemies,
    Move move,
    vector<Move>& moves
) const {
    bool found = false;

    int row = p >> 2;
    int col = p & 3;
    bool even = (row & 1) == 0;

    auto jump = [&](int victim, int land) {
        uint32_t victim_mask = 1u << victim;
        uint32_t land_mask = 1u << land;

        if(!(enemies & victim_mask))
            return;

        if(occupied & land_mask)
            return;

        found = true;

        uint32_t next_occupied = occupied;
        uint32_t next_enemies = enemies;
        Move next = move;

        next_occupied &= ~(1u << p);
        next_occupied &= ~victim_mask;
        next_occupied |=  land_mask;
        next_enemies &= ~(1u << victim);

        next.path |= uint64_t(land) << (5 * next.path_count);
        next.path_count++;

        next.to = land;
        next.captured |= victim_mask;

        int land_row = land >> 2;

        bool prom =
            !queen &&
            ((player == Player::white && land_row == 7) ||
            (player == Player::black && land_row == 0));

        if(prom) {
            moves.push_back(next);
        }
        else {
            gen_jumps(
                land,
                queen,
                player,
                next_occupied,
                next_enemies,
                next,
                moves
            );
        }
    };

    if(row <= 5 && col > 0 && (player == Player::white || queen)){
        int victim = p + (even ? 4 : 3);
        int land = p + 7;
        jump(victim, land);
    }

    if(row <= 5 && col < 3 && (player == Player::white || queen)){
        int victim = p + (even ? 5 : 4);
        int land = p + 9;
        jump(victim, land);
    }

    if(row >= 2 && col > 0 && (player == Player::black || queen)){
        int victim = p - (even ? 4 : 5);
        int land = p - 9;
        jump(victim, land);
    }

    if(row >= 2 && col < 3 && (player == Player::black || queen)){
        int victim = p - (even ? 3 : 4);
        int land = p - 7;
        jump(victim, land);
    }

    if(!found && move.captured != 0)
        moves.push_back(move);
}

void Board::gen_captures(Player player, vector<Move>& moves) const {
    moves.clear();

    uint32_t occupied =
        w_pawns | b_pawns | w_queens | b_queens;

    uint32_t enemies =
        (player == Player::black)
        ? (w_pawns | w_queens)
        : (b_pawns | b_queens);

    uint32_t pawns =
        (player == Player::black)
        ? b_pawns
        : w_pawns;

    uint32_t queens =
        (player == Player::black)
        ? b_queens
        : w_queens;

    uint32_t pieces = pawns;

    while(pieces) {
        int p = __builtin_ctz(pieces);

        Move m(p);

        gen_jumps(
            p,
            false,
            player,
            occupied,
            enemies,
            m,
            moves
        );

        pieces &= pieces - 1;
    }

    pieces = queens;

    while(pieces) {
        int p = __builtin_ctz(pieces);

        Move m(p);

        gen_jumps(
            p,
            true,
            player,
            occupied,
            enemies,
            m,
            moves
        );

        pieces &= pieces - 1;
    }
}

void Board::gen_moves(Player player, vector<Move>& moves) const {
    gen_captures(player, moves);

    if(!moves.empty())
        return;

    gen_normal(player, moves);
}