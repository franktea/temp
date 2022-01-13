 #include <QClipboard>
#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QObject::connect(ui->paste, &QPushButton::clicked, [this](){
        QClipboard *clipboard = QGuiApplication::clipboard();
        ui->plainTextEdit->setPlainText(clipboard->text());
    });
    QObject::connect(ui->trim, &QPushButton::clicked, [this](){
        qDebug() << ui->plainTextEdit->toPlainText() << "\n";
        ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText().replace("//", "").replace("\n", " "));
    });
    QObject::connect(ui->copy, &QPushButton::clicked, [this](){
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(ui->plainTextEdit->toPlainText());
    });
}

Widget::~Widget()
{
    delete ui;
}

