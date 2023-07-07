#include "widget.h"
#include <ApplicationServices/ApplicationServices.h>
#include <QTimer>
#include <QApplication>

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

    QTimer* timer = new QTimer;
    timer->setInterval(5000);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [timer](){
        delete timer;
        QApplication::quit();
    });
    timer->start();

    label = new QLabel(this);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label->setText("exit after 5 seconds");
    label->setAlignment(Qt::AlignBottom | Qt::AlignRight);
}

Widget::~Widget()
{
}

