#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "selectwindow.h"

#include <windows.h>
#include <QProcess>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QLayout>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListView>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QMouseEvent>
#include <QMenu>

bool SetStartup(bool enable, const std::wstring& appName, const std::wstring& appPath) {
    HKEY hKey;
    const wchar_t* keyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    // 打开注册表项
    if (RegOpenKeyEx(HKEY_CURRENT_USER, keyPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    if (enable) {
        // 设置开机自启动
        if (RegSetValueEx(hKey, appName.c_str(), 0, REG_SZ, (const BYTE*)appPath.c_str(), (appPath.length() + 1) * sizeof(wchar_t)) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return false;
        }
    } else {
        // 取消开机自启动
        if (RegDeleteValue(hKey, appName.c_str()) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    return true;
}
bool SetBootStartUp(bool enable)
{
    QString appName = QApplication::applicationName();
    QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
    appPath = "\"" + appPath + "\"";
    return SetStartup(enable,appName.toStdWString(),appPath.toStdWString());
}



void toggleDesktopIcons() {
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


void onchangeWindowHideButtonClicked() {
    toggleDesktopIcons();
}

settingsWindow::settingsWindow(QWidget *parent)
    : QWidget{parent},ui(new Ui::settingswindow)
{
    ui->setupUi(this);

    ui->listWidget->viewport()->installEventFilter(this);

    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    //自动释放内存
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ui->changeWindowHideButton, &QPushButton::clicked, this, &onchangeWindowHideButtonClicked);
    connect(ui->addBackgroundButton,&QPushButton::clicked, this,[this]{
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "/", tr("Image Files (*.png *.jpg *.bmp)"));
        if (!fileName.isEmpty()){
            selectwindow* sw = new selectwindow(this,fileName);
            connect(sw, &selectwindow::sendImgname, this, &settingsWindow::updateBackgroundImg);
            sw->show();
        }
    });

    QFile sfile(QCoreApplication::applicationDirPath()+"\\settings.json");
    if (!sfile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for reading:" << sfile.errorString();
    }

    // 读取文件内容并解析为QJsonDocument
    QByteArray sjsonData = sfile.readAll();
    QJsonDocument sjsonDoc = QJsonDocument::fromJson(sjsonData);

    sfile.close();

    setObj = sjsonDoc.object();

    QString backgroundimg = setObj["backgroundimg"].toString();
    // QString backgrounddata = setObj["backgrounddata"].toString();
    QString name = setObj["name"].toString();

    ui->graphicsView->setStyleSheet("QToolTip{border:1px solid rgb(118, 118, 118); background-color: #ffffff; color:#484848; font-size:12px;}"); //设置边框, 边框色, 背景色, 字体色, 字号
    ui->graphicsView->setToolTip(name);

    //创建显示容器
    QGraphicsScene *scene = new QGraphicsScene;
    //向容器中添加文件路径为fileName（QString类型）的文件
    scene->addPixmap(QPixmap(QCoreApplication::applicationDirPath()+backgroundimg).scaled(200,200,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //借助graphicsView（QGraphicsView类）控件显示容器的内容
    ui->graphicsView->setScene(scene);
    //开始显示
    ui->graphicsView->show();

    QJsonArray storage = setObj["storage"].toArray();

    // ui->listWidget->setIconSize(QSize(200, 200));
    for(int i=0;i<storage.count();i++){
        QJsonValue value=storage.at(i);
        if (value.isObject()) {
            QJsonObject obj = value.toObject();

            QPixmap pixmap(QCoreApplication::applicationDirPath()+obj["backgroundimg"].toString());
            QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
            item->setIcon(QIcon(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            item->setSizeHint(QSize(200, 200));
            item->setText(obj["name"].toString());
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->listWidget->addItem(item); // 添加到 QListWidget
        }
    }

    connect(ui->listWidget, &QListWidget::itemChanged, this, &settingsWindow::onItemChanged);
    ui->checkBox->setChecked(setObj["isStartUp"].toBool());
    connect(ui->checkBox,&QCheckBox::stateChanged, this, [this](int state) {
        if (state == Qt::Checked) {
            if(SetBootStartUp(true)){
                setObj["isStartUp"] = true;
                this->ui->checkBox->setChecked(true);
            }
        } else {
            if(SetBootStartUp(false)){
                setObj["isStartUp"] = false;
                this->ui->checkBox->setChecked(false);
            }
        }
        QFile file(QCoreApplication::applicationDirPath()+"\\settings.json");
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot open file for writing:" << file.errorString();
            return;
        }
        QJsonDocument dataDoc(setObj);
        file.write(dataDoc.toJson());
        file.close();
    });
}
settingsWindow::~settingsWindow(){
    delete ui;
}

void settingsWindow::updateBackgroundImg(const QString &newString){
    QString name = newString.section('/', -1);
    name = name.left(name.lastIndexOf('.'));
    QJsonArray storage = setObj["storage"].toArray();
    QJsonObject obj = QJsonObject();

    QString jsonfilepath = QCoreApplication::applicationDirPath() + "/resource/" + name + ".json";
    obj["name"] = name;
    obj["backgroundimg"] = "/resource/" + name + ".png";
    obj["backgrounddata"] = "/resource/" + name + ".json";
    storage.append(obj);

    setObj["storage"] = storage;

    QFile file(QCoreApplication::applicationDirPath()+"\\settings.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing:" << file.errorString();
        return;
    }
    QJsonDocument dataDoc(setObj);
    file.write(dataDoc.toJson());
    file.close();

    // 创建 JSON 数组
    QJsonArray jsonArray;

    // 将 JSON 数组转换为 QJsonDocument
    QJsonDocument jsonDoc(jsonArray);


    // 打开文件
    QFile jfile(jsonfilepath);
    if (!jfile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    // 写入 JSON 文件
    jfile.write(jsonDoc.toJson());
    jfile.close();

    QPixmap pixmap(newString);
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setIcon(QIcon(pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    item->setSizeHint(QSize(200, 200));
    item->setText(name);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->listWidget->addItem(item); // 添加到 QListWidget
}

bool settingsWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->listWidget->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            QPoint pos = mouseEvent->pos();
            QListWidgetItem *item = ui->listWidget->itemAt(pos);
            if (item) {
                showContextMenu(item, ui->listWidget->mapToGlobal(pos));
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void settingsWindow::showContextMenu(QListWidgetItem *item, const QPoint &pos) {
    QMenu menu;
    menu.addAction("应用", [this, item]() {
        int i=ui->listWidget->row(item);
        QJsonArray storage = setObj["storage"].toArray();
        QJsonObject obj= storage.at(i).toObject();

        QString path = QCoreApplication::applicationDirPath() + obj["backgroundimg"].toString();

        std::wstring ss = path.toStdWString();

        const wchar_t * c_path = ss.c_str();

        // 设置壁纸
        bool result = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void *)c_path, SPIF_UPDATEINIFILE);

        if (result) {
            QProcess::startDetached(QCoreApplication::applicationFilePath(), QCoreApplication::arguments());
            QCoreApplication::exit();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("应用失败"));
            return;
        }


        setObj["name"] = obj["name"];
        setObj["backgroundimg"] = obj["backgroundimg"];
        setObj["backgrounddata"] = obj["backgrounddata"];

        QFile file(QCoreApplication::applicationDirPath()+"\\settings.json");
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot open file for writing:" << file.errorString();
            return;
        }
        QJsonDocument dataDoc(setObj);
        file.write(dataDoc.toJson());
        file.close();

        ui->graphicsView->setToolTip(obj["name"].toString());

        //创建显示容器
        QGraphicsScene *scene = new QGraphicsScene;
        //向容器中添加文件路径为fileName（QString类型）的文件
        scene->addPixmap(QPixmap(path).scaled(200,200,Qt::KeepAspectRatio, Qt::SmoothTransformation));
        delete ui->graphicsView->scene();
        //借助graphicsView（QGraphicsView类）控件显示容器的内容
        ui->graphicsView->setScene(scene);
        //开始显示
        ui->graphicsView->show();
    });
    menu.addAction("删除", [this, item]() {

        /* 删除操作 */
        int i=ui->listWidget->row(item);

        QJsonArray storage = setObj["storage"].toArray();

        if(storage.at(i).toObject()["backgroundimg"].toString() == setObj["backgroundimg"]){
            QMessageBox::warning(this, tr("Error"), tr("不能删除当前应用壁纸"));
            return;
        }

        this->ui->listWidget->takeItem(i);

        QJsonObject obj = storage.at(i).toObject();
        storage.removeAt(i);
        setObj["storage"] = storage;

        QFile file(QCoreApplication::applicationDirPath()+"\\settings.json");
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot open file for writing:" << file.errorString();
            return;
        }
        QJsonDocument dataDoc(setObj);
        file.write(dataDoc.toJson());
        file.close();

        // 检查文件是否存在，然后删除
        if (QFile::exists(QCoreApplication::applicationDirPath()+obj["backgrounddata"].toString())) {
            if (QFile::remove(QCoreApplication::applicationDirPath()+obj["backgrounddata"].toString())) {
                qDebug()  << "已成功删除";
            } else {
                qDebug() << "无法删除文件:";
            }
        } else {
            qDebug() << "找不到文件:";
        }

        if (QFile::exists(QCoreApplication::applicationDirPath()+obj["backgroundimg"].toString())) {
            if (QFile::remove(QCoreApplication::applicationDirPath()+obj["backgroundimg"].toString())) {
                qDebug()  << "已成功删除";
            } else {
                qDebug() << "无法删除文件:";
            }
        } else {
            qDebug() << "找不到文件:";
        }


        delete item;
    });
    menu.exec(pos);
}

void settingsWindow::onItemChanged(QListWidgetItem *item){

    int i=ui->listWidget->row(item);
    QJsonArray storage = setObj["storage"].toArray();
    QJsonObject obj = storage.at(i).toObject();

    if ( obj["backgroundimg"].toString() == setObj["backgroundimg"].toString()){
        setObj["name"] = item->text();
    }

    obj["name"] = item->text();

    storage.replace(i, obj);

    setObj["storage"] = storage;

    QFile file(QCoreApplication::applicationDirPath()+"\\settings.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing:" << file.errorString();
        return;
    }
    QJsonDocument dataDoc(setObj);
    file.write(dataDoc.toJson());
    file.close();
}
