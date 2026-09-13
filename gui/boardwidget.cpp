#include "boardwidget.h"
#include "ai.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QTimer>

#include <vector>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent),
    board_image(":/pics/board.png"),
    w_pawn_image(":/pics/w_pawn.png"),
    w_queen_image(":/pics/w_queen.png"),
    b_pawn_image(":/pics/b_pawn.png"),
    b_queen_image(":/pics/b_queen.png")
{
}

void BoardWidget::set_board(Board *board)
{
    this->board = board;
    update();
}

void BoardWidget::set_ai_depth(int depth)
{
    ai_depth = depth;
}

void BoardWidget::reset_game()
{
    selected_row = -1;
    selected_col = -1;

    ai_thinking = false;
    game_over = false;
    game_started_flag = false;

    history.clear();
    position_count.clear();

    if(board != nullptr){
        position_count[board->get_hash()] = 1;
    }

    update();
}

bool BoardWidget::register_position()
{
    if(board == nullptr)
        return false;

    uint64_t hash = board->get_hash();

    position_count[hash]++;

    return position_count[hash] >= 3;
}

void BoardWidget::unregister_position()
{
    if(board == nullptr)
        return;

    uint64_t hash = board->get_hash();

    auto it = position_count.find(hash);

    if(it == position_count.end())
        return;

    it->second--;

    if(it->second <= 0){
        position_count.erase(it);
    }
}

bool BoardWidget::current_player_has_moves() const
{
    if(board == nullptr)
        return false;

    std::vector<Move> moves;

    board->gen_moves(
        board->current_player(),
        moves
        );

    return !moves.empty();
}

void BoardWidget::finish_game(const QString& message)
{
    game_over = true;
    ai_thinking = false;

    selected_row = -1;
    selected_col = -1;

    update();

    emit game_finished(message);
}

void BoardWidget::undo_move()
{
    if(board == nullptr)
        return;

    if(ai_thinking)
        return;

    if(history.empty())
        return;

    game_over = false;

    selected_row = -1;
    selected_col = -1;

    do{
        unregister_position();

        HistoryEntry entry = history.back();

        history.pop_back();

        board->unmake_move(
            entry.move,
            entry.undo
            );

        if(history.empty())
            break;

    }while(board->current_player() != Player::black);

    if(history.empty()){
        game_started_flag = false;
        emit game_returned_to_start();
    }
    else{
        game_started_flag = true;
    }

    update();
}

void BoardWidget::start_ai_turn()
{
    if(board == nullptr || game_over)
        return;

    ai_thinking = true;

    update();

    QTimer::singleShot(
        150,
        this,
        [this]()
        {
            if(board == nullptr || game_over){
                ai_thinking = false;
                return;
            }

            std::vector<Move> ai_moves;

            board->gen_moves(
                board->current_player(),
                ai_moves
                );

            if(ai_moves.empty()){
                finish_game("You won!");
                return;
            }

            Move move = ai_move(
                *board,
                board->current_player(),
                ai_moves,
                ai_depth
                );

            Undo undo =
                board->make_move(move);

            history.push_back({
                move,
                undo
            });

            if(register_position()){
                finish_game("Draw!");
                return;
            }

            if(!current_player_has_moves()){
                finish_game("You lost!");
                return;
            }

            ai_thinking = false;

            update();
        }
        );
}

void BoardWidget::mousePressEvent(QMouseEvent *event)
{
    if(board == nullptr ||
        ai_thinking ||
        game_over)
    {
        return;
    }

    if(board->current_player() != Player::black)
        return;

    int board_size =
        qMin(width(), height());

    int tile_size =
        board_size / 8;

    int x =
        static_cast<int>(
            event->position().x()
            );

    int y =
        static_cast<int>(
            event->position().y()
            );

    if(x < 0 ||
        y < 0 ||
        x >= board_size ||
        y >= board_size)
    {
        return;
    }

    int col = x / tile_size;
    int row = y / tile_size;

    if((row + col) % 2 == 0)
        return;

    int clicked_square =
        (row << 2) +
        (col >> 1);

    if(selected_row != -1 &&
        selected_col != -1)
    {
        int selected_square =
            (selected_row << 2) +
            (selected_col >> 1);

        std::vector<Move> moves;

        board->gen_moves(
            board->current_player(),
            moves
            );

        for(const Move& move : moves)
        {
            if(move.from == selected_square &&
                move.to == clicked_square)
            {
                if(!game_started_flag){
                    game_started_flag = true;
                    emit game_started();
                }

                Undo undo =
                    board->make_move(move);

                history.push_back({
                    move,
                    undo
                });

                selected_row = -1;
                selected_col = -1;

                if(register_position()){
                    finish_game("Draw!");
                    return;
                }

                if(!current_player_has_moves()){
                    finish_game("You won!");
                    return;
                }

                update();

                start_ai_turn();

                return;
            }
        }
    }

    Piece piece =
        board->get_piece(row, col);

    bool own_piece =
        piece == Piece::b_pawn ||
        piece == Piece::b_queen;

    if(own_piece){
        selected_row = row;
        selected_col = col;

        update();
    }
}

void BoardWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    int board_size =
        qMin(width(), height());

    int tile_size =
        board_size / 8;

    QRect board_rect(
        0,
        0,
        board_size,
        board_size
        );

    painter.drawPixmap(
        board_rect,
        board_image
        );

    if(board == nullptr)
        return;

    for(int row = 0; row < 8; row++)
    {
        for(int col = 0; col < 8; col++)
        {
            if((row + col) % 2 == 0)
                continue;

            Piece piece =
                board->get_piece(
                    row,
                    col
                    );

            if(piece == Piece::empty)
                continue;

            int margin =
                tile_size / 10;

            QRect piece_rect(
                col * tile_size + margin,
                row * tile_size + margin,
                tile_size - 2 * margin,
                tile_size - 2 * margin
                );

            switch(piece)
            {
            case Piece::w_pawn:
                painter.drawPixmap(
                    piece_rect,
                    w_pawn_image
                    );
                break;

            case Piece::b_pawn:
                painter.drawPixmap(
                    piece_rect,
                    b_pawn_image
                    );
                break;

            case Piece::w_queen:
                painter.drawPixmap(
                    piece_rect,
                    w_queen_image
                    );
                break;

            case Piece::b_queen:
                painter.drawPixmap(
                    piece_rect,
                    b_queen_image
                    );
                break;

            case Piece::empty:
                break;
            }
        }
    }

    if(selected_row != -1 &&
        selected_col != -1)
    {
        QRect selected_rect(
            selected_col * tile_size,
            selected_row * tile_size,
            tile_size,
            tile_size
            );

        QPen pen(Qt::yellow);

        pen.setWidth(4);

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(
            selected_rect
            );
    }
}