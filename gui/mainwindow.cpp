#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

#include <string>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLayout *layout =
        ui->centralwidget->layout();

    if(layout == nullptr){
        QHBoxLayout *horizontalLayout =
            new QHBoxLayout(
                ui->centralwidget
                );

        horizontalLayout->addWidget(
            ui->boardWidget
            );

        horizontalLayout->addWidget(
            ui->sidePanel
            );

        layout = horizontalLayout;
    }

    layout->setSpacing(25);

    layout->setContentsMargins(
        12,
        12,
        12,
        12
        );

    ui->boardWidget->setFixedSize(
        640,
        640
        );

    ui->sidePanel->setFixedWidth(160);

    setFixedSize(850, 690);

    connect(
        ui->start_button,
        &QPushButton::clicked,
        this,
        &MainWindow::new_game
        );

    connect(
        ui->undo_button,
        &QPushButton::clicked,
        ui->boardWidget,
        &BoardWidget::undo_move
        );

    connect(
        ui->d_box,
        qOverload<int>(
            &QSpinBox::valueChanged
            ),
        ui->boardWidget,
        &BoardWidget::set_ai_depth
        );

    connect(
        ui->boardWidget,
        &BoardWidget::game_started,
        this,
        [this]()
        {
            ui->d_box->setEnabled(false);
        }
        );

    connect(
        ui->boardWidget,
        &BoardWidget::game_returned_to_start,
        this,
        [this]()
        {
            ui->d_box->setEnabled(true);
        }
        );

    connect(
        ui->boardWidget,
        &BoardWidget::game_finished,
        this,
        [this](const QString& message)
        {
            QMessageBox::information(
                this,
                "Game over",
                message
                );
        }
        );

    ui->boardWidget->set_ai_depth(
        ui->d_box->value()
        );

    new_game();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::new_game()
{
    std::vector<std::string> lines = {
        ".r.r.r.r",
        "r.r.r.r.",
        ".r.r.r.r",
        "........",
        "........",
        "b.b.b.b.",
        ".b.b.b.b",
        "b.b.b.b."
    };

    board.load_board(
        lines,
        Player::black
        );

    board.init_hash();

    ui->boardWidget->set_board(
        &board
        );

    ui->boardWidget->reset_game();

    ui->d_box->setEnabled(true);
}