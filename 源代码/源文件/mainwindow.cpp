#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QScreen>
#include <QDesktopServices>
#include <QUrl>
#include <QMouseEvent>
#include <QMenu>
#include <QDir>
#include <QToolTip>
#include <QGraphicsOpacityEffect>
#include <QProcess>
#include <windows.h>

void toggleDesktopIconsmain() {
    // 获取桌面窗口的句柄
    HWND hShellWnd = FindWindow(L"Progman", NULL);
    if (!hShellWnd) {
        hShellWnd = FindWindow(L"WorkerW", NULL);  // 如果 Progman 找不到，尝试 WorkerW
    }

    // 获取桌面图标窗口的句柄
    HWND hDefView = FindWindowEx(hShellWnd, NULL, L"SHELLDLL_DefView", NULL);

    // 判断当前状态并切换
    bool visible = IsWindowVisible(hDefView);
    if (visible) {
        // 隐藏桌面图标
        ShowWindow(hDefView, SW_HIDE);
    }
    else if (!visible){
        // 显示桌面图标
        ShowWindow(hDefView, SW_SHOW);
    }
}

DoubleClickFilter::DoubleClickFilter(QObject* parent):QObject(parent){};

bool DoubleClickFilter::eventFilter(QObject *obj, QEvent *event)
{
    qDebug()<<event->type();
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            // toggleDesktopIcons();
            return QObject::eventFilter(obj, event);
        }
    }
    return QObject::eventFilter(obj, event);
}


ImageView::ImageView(QWidget *parent,QJsonObject obj) : QWidget(parent) {
    this->setWindowFlags(Qt::FramelessWindowHint);

    setAttribute(Qt::WA_TranslucentBackground);

    setAttribute(Qt::WA_Hover);

    data = obj;

    QPixmap pixmap;
    if(obj["isCuted"].toBool()){
        pixmap = QPixmap(QCoreApplication::applicationDirPath()+obj["url"].toString());
    }
    else{
        pixmap = QPixmap(obj["url"].toString());
    }
    QPixmap scaledPic = pixmap.scaled(obj["scale_w"].toInt()*5/4, obj["scale_h"].toInt()*5/4, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    imageView = new QLabel(this);
    imageView->setAttribute(Qt::WA_TranslucentBackground);
    imageView->setPixmap(scaledPic);
    imageView->resize(scaledPic.size());
    imageView->setScaledContents(true);

    if(obj["isCuted"].toBool()){
        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(imageView);
        opacityEffect->setOpacity(0.1);
        imageView->setGraphicsEffect(opacityEffect);
    }

    setStyleSheet("QToolTip{border:1px solid rgb(118, 118, 118); background-color: #ffffff; color:#484848; font-size:12px;}"); //设置边框, 边框色, 背景色, 字体色, 字号

    resize(imageView->size());

    if(obj["bindtype"].toString() == "icon"){
        name = "切换";
    }
    else if (obj["bindtype"].toString() == "QTMod"){
        name = obj["ModName"].toString();
    }
    else{
        QString  path = obj["bindurl"].toString();
        name = path.section('/', -1);
    }


}
ImageView::~ImageView(){

}

bool ImageView::event(QEvent *event){

    switch (event->type()) {
    case QEvent::HoverEnter:
        // 处理鼠标进入事件
        hightlight();
        return QWidget::event(event);
        break;
    case QEvent::HoverMove:
        QToolTip::showText(QCursor::pos(), name, this);
        return QWidget::event(event);
        break;
    case QEvent::HoverLeave:
        // 处理鼠标离开事件
        hightlightback();
        return QWidget::event(event);
        break;
    default:
        return QWidget::event(event);  // 将其他事件传递给基类的事件处理函数
    }
    return true;
}

void ImageView::mouseDoubleClickEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        if(data["bindtype"].toString() == "icon"){
            toggleDesktopIconsmain();
            return;
        }
        else if (data["bindtype"].toString() == "QTMod"){
            // 获取当前鼠标坐标

            QPoint cursorPos = QCursor::pos();
            int x = cursorPos.x();
            int y = cursorPos.y();

            // 准备要启动的程序和参数
            QString program = data["bindurl"].toString();
            QStringList arguments;
            arguments << QString::number(x) << QString::number(y); // 添加鼠标坐标作为命令行参数

            // 启动外部程序并传递参数
            QProcess::startDetached(QDir::toNativeSeparators(program),arguments);
            return;
        }
        QString path = data["bindurl"].toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void ImageView::hightlight(){
    QWidget* mask = new QWidget(this);
    mask->setStyleSheet("background-color: rgba(216, 216, 216, 0.2);");
    mask->setFixedSize(this->size());  // 与 QLabel 同样大小


    // 将遮罩层放置在 QLabel 上
    mask->move(0, 0);  // 根据需要调整位置
    mask->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mask->show();

    masks.append(mask);
}
void ImageView::hightlightback(){
    while (!masks.empty()){
        delete masks.back();
        masks.pop_back();
    }
}

void ImageView::openFileLocation(){
    QString path = data["bindurl"].toString();
    path = QDir::toNativeSeparators(path);
    ShellExecute(nullptr, L"open", L"explorer.exe", ("/select," + path).toStdWString().c_str(), nullptr, SW_SHOW);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint|Qt::Tool); // 无边框且始终位于底部
    setAttribute(Qt::WA_TranslucentBackground); // 背景透明

    QScreen *deviceScreen = QGuiApplication::primaryScreen();
    resize(deviceScreen->size().width(),deviceScreen->size().height());

    QFile sfile(QCoreApplication::applicationDirPath()+"\\settings.json");
    if (!sfile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for reading:" << sfile.errorString();
    }

    // 读取文件内容并解析为QJsonDocument
    QByteArray sjsonData = sfile.readAll();
    QJsonDocument sjsonDoc = QJsonDocument::fromJson(sjsonData);

    sfile.close();

    QJsonObject setObj = sjsonDoc.object();

    QString backgrounddata = setObj["backgrounddata"].toString();


    QFile file(QCoreApplication::applicationDirPath()+backgrounddata);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for reading:" << file.errorString();
    }

    // 读取文件内容并解析为QJsonDocument
    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    file.close();

    // 将QJsonDocument转换为QJsonArray
    datas = jsonDoc.array();

    for (int i=0;i<datas.count();i++) {
        QJsonValue value=datas.at(i);
        if (value.isObject()) {
            QJsonObject obj = value.toObject();

            ImageView *imageview = new ImageView(this,obj);
            imageviews.append(imageview);
            imageview->move(obj["pos_w"].toInt()*5/4,obj["pos_h"].toInt()*5/4);
            imageview->show();

        }
    }

    // // 创建事件过滤器实例
    // doubleClickFilter = new DoubleClickFilter(this);

    // // 将事件过滤器安装到目标 QWidget 上
    // ui->label->installEventFilter(doubleClickFilter);
    // adjustSize();
    // ui->label->setStyleSheet("background-color:(0,0,0,1);");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete doubleClickFilter;
}


void MainWindow::onChangeWallpaperClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "/", tr("Image Files (*.png *.jpg *.bmp)"));
    if (!fileName.isEmpty()) {
        BOOL result = SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)fileName.toStdWString().c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        if (result) {
            QMessageBox::information(this, tr("Success"), tr("Wallpaper changed successfully!"));
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to change wallpaper."));
        }
    }
}
