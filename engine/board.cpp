#include "board.hpp"

#include <random>

using namespace std;

Player Board::current_player() const {
    return turn;
}

void Board::load_board(
    const std::vector<std::string>& lines,
    Player player
    ){
    w_pawns = 0;
    b_pawns = 0;
    w_queens = 0;
    b_queens = 0;

    for(int i = 0; i < 8; i++){
        int start = (i % 2 == 0) ? 1 : 0;

        for(int j = start; j < 8; j += 2){
            switch(lines[i][j]){
            case 'b':
                b_pawns |= (1u << ((i << 2) + (j / 2)));
                break;

            case 'B':
                b_queens |= (1u << ((i << 2) + (j / 2)));
                break;

            case 'r':
                w_pawns |= (1u << ((i << 2) + (j / 2)));
                break;

            case 'R':
                w_queens |= (1u << ((i << 2) + (j / 2)));
                break;
            }
        }
    }

    turn = player;
}

Board::Board() {}

struct Zobrist {
    uint64_t piece[32][5];
    uint64_t turn;

    Zobrist(){
        mt19937_64 rng(123456789);

        for(int sq = 0; sq < 32; sq++){
            for(int p = 0; p < 5; p++){
                piece[sq][p] = rng();
            }
        }

        turn = rng();
    }

    uint64_t hash(const Board& board){
        uint64_t h = 0;

        for(int i = 0; i < 8; i++){
            int start = (i % 2 == 0) ? 1 : 0;

            for(int j = start; j < 8; j += 2){
                Piece p = board.get_piece(i, j);

                if(p != Piece::empty){
                    int sq = (i << 2) + (j >> 1);

                    h ^= piece[sq][static_cast<int>(p)];
                }
            }
        }

        if(board.current_player() == Player::black){
            h ^= turn;
        }

        return h;
    }
};

Zobrist zobrist{};

void Board::init_hash(){
    hash = zobrist.hash(*this);
}

uint64_t Board::get_hash() const{
    return hash;
}

Piece Board::get_piece(int x, int y) const {
    uint32_t mask = 1u << ((x << 2) + (y >> 1));

    if(w_pawns & mask)
        return Piece::w_pawn;

    if(b_pawns & mask)
        return Piece::b_pawn;

    if(w_queens & mask)
        return Piece::w_queen;

    if(b_queens & mask)
        return Piece::b_queen;

    return Piece::empty;
}

Undo Board::make_move(const Move& m) {
    Undo u;

    u.w_pawns = w_pawns;
    u.b_pawns = b_pawns;
    u.w_queens = w_queens;
    u.b_queens = b_queens;
    u.old_turn = turn;
    u.old_hash = hash;

    uint32_t from_mask = (1u << m.from);
    uint32_t to_mask = (1u << m.to);

    Piece moved;

    if(w_pawns & from_mask)
        moved = Piece::w_pawn;
    else if(b_pawns & from_mask)
        moved = Piece::b_pawn;
    else if(w_queens & from_mask)
        moved = Piece::w_queen;
    else
        moved = Piece::b_queen;

    hash ^= zobrist.piece[m.from][static_cast<int>(moved)];

    uint32_t captured_pawns;
    uint32_t captured_queens;

    Piece captured_pawn_type;
    Piece captured_queen_type;

    if(turn == Player::white){
        captured_pawns = m.captured & b_pawns;
        captured_queens = m.captured & b_queens;

        captured_pawn_type = Piece::b_pawn;
        captured_queen_type = Piece::b_queen;
    }
    else{
        captured_pawns = m.captured & w_pawns;
        captured_queens = m.captured & w_queens;

        captured_pawn_type = Piece::w_pawn;
        captured_queen_type = Piece::w_queen;
    }

    while(captured_pawns){
        int sq = __builtin_ctz(captured_pawns);

        hash ^= zobrist.piece[sq][static_cast<int>(captured_pawn_type)];

        captured_pawns &= captured_pawns - 1;
    }

    while(captured_queens){
        int sq = __builtin_ctz(captured_queens);

        hash ^= zobrist.piece[sq][static_cast<int>(captured_queen_type)];

        captured_queens &= captured_queens - 1;
    }

    Piece placed = moved;

    if(moved == Piece::w_pawn){
        w_pawns &= ~from_mask;

        if((m.to >> 2) == 7){
            w_queens |= to_mask;
            placed = Piece::w_queen;
        }
        else{
            w_pawns |= to_mask;
        }
    }
    else if(moved == Piece::b_pawn){
        b_pawns &= ~from_mask;

        if((m.to >> 2) == 0){
            b_queens |= to_mask;
            placed = Piece::b_queen;
        }
        else{
            b_pawns |= to_mask;
        }
    }
    else if(moved == Piece::w_queen){
        w_queens &= ~from_mask;
        w_queens |= to_mask;
    }
    else if(moved == Piece::b_queen){
        b_queens &= ~from_mask;
        b_queens |= to_mask;
    }

    if(turn == Player::white){
        b_pawns &= ~m.captured;
        b_queens &= ~m.captured;
    }
    else{
        w_pawns &= ~m.captured;
        w_queens &= ~m.captured;
    }

    hash ^= zobrist.piece[m.to][static_cast<int>(placed)];
    hash ^= zobrist.turn;

    turn = (turn == Player::white)
               ? Player::black
               : Player::white;

    return u;
}

void Board::unmake_move(
    const Move& m,
    const Undo& u
    ){
    w_pawns = u.w_pawns;
    b_pawns = u.b_pawns;
    w_queens = u.w_queens;
    b_queens = u.b_queens;

    turn = u.old_turn;
    hash = u.old_hash;
}