#include "widget.h"
#include <ApplicationServices/ApplicationServices.h>
#include <QTimer>
#include <QApplication>
#include <QHBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    CGEventRef event = CGEventCreate(NULL);
    CGPoint center = CGEventGetLocation(event);
    CFRelease(event);
    center.x = 0;
    center.y = 0;

    CGDisplayCount displayCount;
    CGGetActiveDisplayList(0, NULL, &displayCount);

    //std::cout<<"displaycount is: "<<displayCount<<"\n";

    CGDirectDisplayID *displays = new CGDirectDisplayID[displayCount];
    CGGetActiveDisplayList(displayCount, displays, NULL);

    for (int i = 0; i < displayCount; i++) {
        CGRect bounds = CGDisplayBounds(displays[i]);
        center.x += bounds.size.width / 2;
        center.y += bounds.size.height / 2;
    }

    center.x /= displayCount;
    center.y /= displayCount;

    CGWarpMouseCursorPosition(center);
    CGAssociateMouseAndMouseCursorPosition(true);

    delete[] displays;

    label = new QLabel(this);
    QFont font = label->font();
    font.setPointSize(50);
    label->setFont(font);
    label->setContentsMargins(0, 0, 0, 0);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label->setText("exit after 5 seconds");
    label->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    layout->addWidget(label);

    QTimer* timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, &QTimer::timeout, [label=this->label, timer](){
        static int i = 0;
        if(i >= 5) {
            delete timer;
            QApplication::quit();
        } else {
            QString str = QString("Exit after %1 seconds").arg(5-i);
            label->setText(str);
            ++ i;
        }
    });
    timer->start();
}

Widget::~Widget()
{
}

