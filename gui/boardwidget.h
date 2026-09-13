#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "board.hpp"

class QMouseEvent;
class QPaintEvent;

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);

    void set_board(Board *board);
    void set_ai_depth(int depth);
    void reset_game();
    void undo_move();

signals:
    void game_finished(const QString& message);
    void game_started();
    void game_returned_to_start();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    struct HistoryEntry
    {
        Move move;
        Undo undo;
    };

    Board *board = nullptr;

    QPixmap board_image;
    QPixmap w_pawn_image;
    QPixmap w_queen_image;
    QPixmap b_pawn_image;
    QPixmap b_queen_image;

    int selected_row = -1;
    int selected_col = -1;

    int ai_depth = 6;

    bool ai_thinking = false;
    bool game_over = false;
    bool game_started_flag = false;

    std::unordered_map<uint64_t, int> position_count;
    std::vector<HistoryEntry> history;

    bool register_position();
    void unregister_position();

    bool current_player_has_moves() const;

    void finish_game(const QString& message);
    void start_ai_turn();
};