#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QGraphicsScene>
#include <QWidget>
#include <QGraphicsView>
#include <QTimer>
#include <random>
#include <QPoint>
#include <QGraphicsItemGroup>
#include "chessgrid.h"

namespace Ui {
class ChessBoard;
}

class ChessBoard : public QGraphicsView
{
    Q_OBJECT
    static const int WIDTH = 9;
    static const int HEIGHT = 10;
    static const int GRID_SIZE = 100; // 每个格子的边长
public:
    explicit ChessBoard(QWidget *parent = nullptr);
    ~ChessBoard() override;
private:
    void CalcPoints();
    void CreateGridLines();
private slots:
    void timeout();
private:
    Ui::ChessBoard *ui;
    QGraphicsScene scene_;
    QTimer* timer_;
    QPoint points_[HEIGHT][WIDTH];
    QGraphicsLineItem grid_meridian_[10];
    QGraphicsLineItem grid_latitude_[16];
    ChessGrid* grid_;
    vector<QGraphicsLineItem*> pathes_;
};

#endif // CHESSBOARD_H
