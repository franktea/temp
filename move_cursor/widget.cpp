#include <QCursor>
#include <QTime>
#include <QTimer>
#include <QScreen>
#include <QApplication>
#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    seconds_ = 10;

    QScreen* screen = QApplication::primaryScreen();
    QRect rec = screen->geometry();
    int h = rec.height() / 2;
    int w = rec.width() / 2;

    QCursor c = cursor();
    c.setPos(QPoint(w, h));
    //c.setShape(Qt::BlankCursor);
    setCursor(c);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    timer->start(1000);

    showTime();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::showTime()
{
    QString text = QString::number(seconds_);
/*
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm");
    if ((time.second() % 2) == 0)
        text[2] = ' ';
*/
    ui->lcdNumber->display(text);

    if(seconds_ <= 0)
    {
        QApplication::exit(0);
    }

    -- seconds_;
}
