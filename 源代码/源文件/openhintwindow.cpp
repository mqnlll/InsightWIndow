#include "openhintwindow.h"
#include "ui_openhintwindow.h"

#include <QTimer>
#include <QGraphicsDropShadowEffect>

openhintwindow::openhintwindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::openhintwindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    // 使用样式表设置背景颜色
    setStyleSheet("background-color: #f0f8ff; border-radius: 15px;");
    ui->label->setStyleSheet("color: black;");
    ui->label_2->setStyleSheet("color: black;");


    // 创建阴影效果
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);
    shadowEffect->setOffset(5, 5);
    shadowEffect->setColor(Qt::black);

    // 应用阴影效果
    setGraphicsEffect(shadowEffect);

    // 设置计时器，指定时间后自动关闭窗口
    QTimer::singleShot(2000, this, &openhintwindow::closeAndDelete);  // 3000 毫秒后关闭并释放窗口
}

openhintwindow::~openhintwindow()
{
    delete ui;
}

void openhintwindow::closeAndDelete(){
    close();
    deleteLater();
}
