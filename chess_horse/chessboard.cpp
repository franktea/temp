#include "chessboard.h"
#include "ui_chessboard.h"

#include <QGraphicsLineItem>
#include <QDebug>
#include <QThread>

ChessBoard::ChessBoard(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::ChessBoard)
{
    ui->setupUi(this);
    timer_ = new QTimer(this);
    scene_.setParent(this);
    this->resize(9*GRID_SIZE + 20, 10 * GRID_SIZE + 20);
    this->setScene(&scene_);
    connect(timer_, SIGNAL(timeout()), this, SLOT(timeout()));
    CalcPoints();
    for(QGraphicsLineItem& l: grid_meridian_)
    {
        scene_.addItem(&l);
    }
    for(QGraphicsLineItem& l: grid_latitude_)
    {
        scene_.addItem(&l);
    }
    grid_ = new ChessGrid(3, 5);
    timer_->start(30);
}

ChessBoard::~ChessBoard()
{
    delete ui;
}

void ChessBoard::CalcPoints()
{
    for(int y = 0; y < 10; ++y)
    {
        for(int x = 0; x < 9; ++x)
        {
            int xvalue = 10 + x * GRID_SIZE;
            int yvalue = 10 + y * GRID_SIZE;
            points_[y][x] = QPoint{xvalue, yvalue};
        }
    }

    for(int y = 0; y < 10; ++y)
    {
        grid_meridian_[y].setLine(points_[y][0].x(), points_[y][0].y(), points_[y][8].x(), points_[y][8].y());
    }

    for(int x = 0; x < 7; ++x)
    {
        grid_latitude_[x].setLine(points_[0][x+1].x(), points_[0][x+1].y(),
                points_[4][x+1].x(), points_[4][x+1].y());
    }

    for(int x = 7; x < 14; ++x)
    {
        grid_latitude_[x].setLine(points_[5][x+1-7].x(), points_[5][x+1-7].y(),
                points_[9][x+1-7].x(), points_[9][x+1-7].y());
    }
    grid_latitude_[14].setLine(points_[0][0].x(), points_[0][0].y(), points_[9][0].x(), points_[9][0].y());
    grid_latitude_[15].setLine(points_[0][8].x(), points_[0][8].y(), points_[9][8].x(), points_[9][8].y());
}

void ChessBoard::CreateGridLines()
{

}


void ChessBoard::timeout()
{
    if(grid_->Finished())
    {
        timer_->stop();
        return;
    }
    grid_->Step();

    vector<std::pair<int, int>>& points = grid_->points;
    for(auto it: pathes_)
    {
        QGraphicsLineItem* l = it;
        scene_.removeItem(l);
        delete l;
    }
    pathes_.clear();

    for(int i = 1; i < int(points.size()); ++i)
    {
        int x1 = points_[points[i-1].second][points[i-1].first].x();
        int y1 = points_[points[i-1].second][points[i-1].first].y();
        int x2 = points_[points[i].second][points[i].first].x();
        int y2 = points_[points[i].second][points[i].first].y();
        QGraphicsLineItem* l = new QGraphicsLineItem(x1, y1, x2, y2);
        QPen pen;
        pen.setColor(Qt::red);
        pen.setWidth(3);
        l->setPen(pen);
        scene_.addItem(l);
        pathes_.push_back(l);
    }
    update();
    //qDebug()<<"timer griggered...";
}
