#include "selectwindow.h"
#include "ui_selectwindow.h"
#include <selectwidget.h>

#include <QScreen>
#include <QPalette>
#include <QPixmap>
#include <QPainter>
#include <QCoreApplication>
#include <QFile>
#include <QUuid>
#include <windows.h>

selectwindow::selectwindow(QWidget *parent,QString imgurl)
    : QWidget(parent)
    , ui(new Ui::selectwindow)
{
    //自动释放内存
    setAttribute(Qt::WA_DeleteOnClose, true);

    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlags(Qt::Window);

    ui->setupUi(this);
    QScreen *deviceScreen = QGuiApplication::primaryScreen();

    originalPixmap = QPixmap(imgurl);
    scaledPixmap = originalPixmap.scaled(ui->imgwidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 调整大小保持比例
    ratiox = float(originalPixmap.width()) / float(scaledPixmap.width());
    ratioy = float(originalPixmap.height()) / float(scaledPixmap.height());

    // 创建一个空白的 QPixmap，用于居中显示
    QPixmap centeredPixmap(ui->imgwidget->size());
    centeredPixmap.fill(Qt::transparent);  // 填充透明色

    // 计算图片的居中位置
    QPainter painter(&centeredPixmap);
    int x = (ui->imgwidget->width() - scaledPixmap.width()) / 2;
    int y = (ui->imgwidget->height() - scaledPixmap.height()) / 2;
    painter.drawPixmap(x, y, scaledPixmap);

    // 将 QPixmap 转换为 QImage，然后设置为背景
    ui->imgwidget->setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(ui->imgwidget->backgroundRole(), QBrush(centeredPixmap));
    ui->imgwidget->setPalette(palette);

    selectwidget = new SelectRectWidget(this);
    selectwidget->setGeometry(x + ui->imgwidget->x(),y + ui->imgwidget->y(),scaledPixmap.width(),scaledPixmap.height());

    selectwidget->setFixCenterRectRatio(float(deviceScreen->size().width()) / float(deviceScreen->size().height()));

    connect(ui->sureButton,&QPushButton::clicked, this, &selectwindow::on_sureButton_clicked);
    connect(ui->cancleButton,&QPushButton::clicked, this, [this](){
        this->hide();
    });
}

selectwindow::~selectwindow()
{
    delete ui;
}

void selectwindow::on_sureButton_clicked(){
    QPixmap cutpic = originalPixmap.copy(float(selectwidget->centerRect().x())*ratiox,float(selectwidget->centerRect().y())*ratioy,float(selectwidget->centerRect().width())*ratiox,float(selectwidget->centerRect().height())*ratioy);

    HDC hdc = GetDC(NULL);
    int cx = GetDeviceCaps(hdc, DESKTOPHORZRES);
    int cy = GetDeviceCaps(hdc, DESKTOPVERTRES);
    ReleaseDC(NULL, hdc);
    QSize realPhysicalSize(cx,cy);
    qDebug() << "Real Physical screen size:" << realPhysicalSize;

    cutpic = cutpic.scaled(realPhysicalSize, Qt::IgnoreAspectRatio ,Qt::SmoothTransformation);
    QString filename = QCoreApplication::applicationDirPath()+"/resource/"+ QUuid::createUuid().toString().remove("{").remove("}") + ".png";
    cutpic.save(filename, "PNG");

    emit sendImgname(filename);

    this->hide();
}
