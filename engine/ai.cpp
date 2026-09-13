#include "ai.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

using namespace std;

constexpr int MAX_PLY = 64;

array<vector<Move>, MAX_PLY> move_lists;

void init_move_lists(){
    for(auto& moves : move_lists){
        moves.reserve(32);
    }
}

enum class TTFlag {
    EXACT,
    LOWER,
    UPPER
};

struct TTEntry {
    uint64_t key = 0;
    int depth = -1;
    int score = 0;
    TTFlag flag = TTFlag::EXACT;
};

constexpr size_t TT_SIZE = 1 << 20;

array<TTEntry, TT_SIZE> tt;

int eval(const Board& board, Player player) {
    int score = 0;

    constexpr int PAWN_VALUE  = 100;
    constexpr int QUEEN_VALUE = 200;

    constexpr int ADVANCE     = 6;
    constexpr int CENTER_PAWN = 5;
    constexpr int CENTER_KING = 6;
    constexpr int PROMOTION   = 15;

    score += __builtin_popcount(board.b_pawns)  * PAWN_VALUE;
    score -= __builtin_popcount(board.w_pawns)  * PAWN_VALUE;

    score += __builtin_popcount(board.b_queens) * QUEEN_VALUE;
    score -= __builtin_popcount(board.w_queens) * QUEEN_VALUE;

    uint32_t pieces = board.b_pawns;

    while (pieces) {
        int sq = __builtin_ctz(pieces);
        pieces &= pieces - 1;

        int row = sq >> 2;
        int col = sq & 3;

        int advancement = 7 - row;

        score += advancement * ADVANCE;

        if (row >= 2 && row <= 5 && col >= 1 && col <= 2)
            score += CENTER_PAWN;

        if (row == 1)
            score += PROMOTION;
    }

    pieces = board.w_pawns;

    while (pieces) {
        int sq = __builtin_ctz(pieces);
        pieces &= pieces - 1;

        int row = sq >> 2;
        int col = sq & 3;

        int advancement = row;

        score -= advancement * ADVANCE;

        if (row >= 2 && row <= 5 && col >= 1 && col <= 2)
            score -= CENTER_PAWN;

        if (row == 6)
            score -= PROMOTION;
    }

    pieces = board.b_queens;

    while (pieces) {
        int sq = __builtin_ctz(pieces);
        pieces &= pieces - 1;

        int row = sq >> 2;
        int col = sq & 3;

        if (row >= 2 && row <= 5 && col >= 1 && col <= 2)
            score += CENTER_KING;
    }

    pieces = board.w_queens;

    while (pieces) {
        int sq = __builtin_ctz(pieces);
        pieces &= pieces - 1;

        int row = sq >> 2;
        int col = sq & 3;

        if (row >= 2 && row <= 5 && col >= 1 && col <= 2)
            score -= CENTER_KING;
    }

    constexpr int BACK_GUARD = 8;

    uint32_t black_back = board.b_pawns & 0xF0000000u;
    uint32_t white_back = board.w_pawns & 0x0000000Fu;

    score += __builtin_popcount(black_back) * BACK_GUARD;
    score -= __builtin_popcount(white_back) * BACK_GUARD;

    int total =
        __builtin_popcount(board.b_pawns) +
        __builtin_popcount(board.w_pawns) +
        __builtin_popcount(board.b_queens) +
        __builtin_popcount(board.w_queens);

    return player == Player::black ? score : -score;
}

int quiescence(Board& board, Player player, int alpha, int beta, int ply)
{
    Player curr = board.current_player();

    vector<Move>& moves = move_lists[ply];
    board.gen_captures(curr, moves);

    if(moves.empty())
        return eval(board, player);

    if(curr == player){
        int best = -200000;

        for(const Move& m : moves){
            Undo u = board.make_move(m);

            int score = quiescence(
                board,
                player,
                alpha,
                beta,
                ply + 1
                );

            board.unmake_move(m, u);

            best = max(best, score);
            alpha = max(alpha, best);

            if(alpha >= beta)
                break;
        }

        return best;
    }
    else{
        int best = 200000;

        for(const Move& m : moves){
            Undo u = board.make_move(m);

            int score = quiescence(
                board,
                player,
                alpha,
                beta,
                ply + 1
                );

            board.unmake_move(m, u);

            best = min(best, score);
            beta = min(beta, best);

            if(alpha >= beta)
                break;
        }

        return best;
    }
}

int minimax(
    Board &board,
    Player player,
    int depth,
    int alpha,
    int beta,
    int ply
    ){
    int old_alpha = alpha;
    int old_beta = beta;

    uint64_t h = board.get_hash();
    TTEntry& e = tt[h & (TT_SIZE - 1)];

    if(e.key == h && e.depth >= depth){
        if(e.flag == TTFlag::EXACT)
            return e.score;

        if(e.flag == TTFlag::LOWER)
            alpha = max(alpha, e.score);
        else if(e.flag == TTFlag::UPPER)
            beta = min(beta, e.score);

        if(alpha >= beta)
            return e.score;
    }

    Player curr = board.current_player();

    if(depth == 0)
        return quiescence(
            board,
            player,
            alpha,
            beta,
            ply
            );

    vector<Move>& moves = move_lists[ply];
    board.gen_moves(curr, moves);

    if(moves.empty()){
        if(curr == player)
            return -10000 - depth;

        return 10000 + depth;
    }

    int best;

    if(player == curr){
        best = -200000;

        for(size_t i = 0; i < moves.size(); i++){
            const Move& m = moves[i];
            Undo u = board.make_move(m);

            int score = minimax(
                board,
                player,
                depth - 1,
                alpha,
                beta,
                ply + 1
                );

            board.unmake_move(m, u);

            best = max(best, score);

            alpha = max(alpha, best);

            if(beta <= alpha)
                break;
        }
    }
    else{
        best = 200000;

        for(size_t i = 0; i < moves.size(); i++){
            const Move& m = moves[i];
            Undo u = board.make_move(m);

            int score = minimax(
                board,
                player,
                depth - 1,
                alpha,
                beta,
                ply + 1
                );

            board.unmake_move(m, u);

            best = min(best, score);

            beta = min(beta, best);

            if(beta <= alpha)
                break;
        }
    }

    TTFlag flag;

    if(best <= old_alpha)
        flag = TTFlag::UPPER;
    else if(best >= old_beta)
        flag = TTFlag::LOWER;
    else
        flag = TTFlag::EXACT;

    e.key = h;
    e.depth = depth;
    e.score = best;
    e.flag = flag;

    return best;
}

Move ai_move(
    Board &board,
    Player player,
    const vector<Move>& moves,
    int depth
    ){
    int best_score = -200000;
    Move best_move = moves[0];

    int alpha = -200000;
    int beta = 200000;

    for(const auto& m : moves){
        Undo u = board.make_move(m);

        int score = minimax(
            board,
            player,
            depth,
            alpha,
            beta,
            0
            );

        board.unmake_move(m, u);

        if(score > best_score){
            best_score = score;
            best_move = m;
        }

        alpha = max(alpha, best_score);
    }

    return best_move;
}