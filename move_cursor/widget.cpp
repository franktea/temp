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
    QRect rec = screen->availableGeometry();
    qDebug() << "Cursor pos: " << QCursor::pos();
    int h = rec.height() / 2;
    int w = rec.width() / 2;

    qDebug() << "w = "<<w<<", h= "<<h;

    QPointF p = mapToGlobal(QPointF(w, h));
    QCursor::setPos(p.x(), p.y());

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
    text = QString("Exit in ") + text + " seconds";
    ui->label->setText(text);

    if(seconds_ <= 0)
    {
        QApplication::exit(0);
    }

    -- seconds_;
}
