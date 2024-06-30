#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDropEvent>
#include <QScreen>
#include <QMenu>
#include <QSizeGrip>
#include <QSettings>
#include <QString>
#include <QPalette>
#include <QBrush>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include<QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>


#include "ui_previewwindow.h"
#include "ui_BindUrlSetWindow.h"
#include "ui_information.h"
#include "previewwindow.h"


CustomSizeGrip::CustomSizeGrip(QWidget *parent):QWidget(parent){
    targetWidget = parent; // 假设父控件就是我们想要调整大小的控件
}
void CustomSizeGrip::mouseMoveEvent(QMouseEvent *event){
    if (dragging) {
        QPoint newPos = event->globalPos();
        QPoint delta = newPos - lastMousePos;
        QSize newSize = targetWidget->size() + QSize(delta.x(), delta.y());
        targetWidget->resize(newSize);
        lastMousePos = newPos;
        this->setGeometry(targetWidget->width() - 20, targetWidget->height() - 20, 20, 20);
    }
}

void CustomSizeGrip::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        lastMousePos = event->globalPos();
    }
}

void CustomSizeGrip::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        dragging = false;
    }
    ImageWidget *tar = static_cast<ImageWidget *>(targetWidget);
    tar->hightlightback();
    tar->hightlight();
}



ImageWidget::ImageWidget(QWidget *parent,QPixmap *pixmap) : QWidget(parent) {
    this->setWindowFlags(Qt::FramelessWindowHint);

    setAttribute(Qt::WA_TranslucentBackground);

    imageView = new QLabel(this);
    imageView->setAttribute(Qt::WA_TranslucentBackground);
    imageView->setPixmap(*pixmap);
    imageView->resize(pixmap->size());
    imageView->setScaledContents(true);

    resize(imageView->size());

    CustomSizeGrip *sizeGrip = new CustomSizeGrip(this);
    sizeGrip->setGeometry(this->width() - 20, this->height() - 20, 20, 20);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(imageView); // 将 QLabel 添加到布局中
    layout->setContentsMargins(0,0,0,0);

    this->setLayout(layout); // 将布局设置到 QWidget


}
ImageWidget::~ImageWidget(){

}


void ImageWidget::mousePressEvent(QMouseEvent *event) {
    m_lastPos = event->pos();
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
        QPoint newPos = pos() + (event->pos() - m_lastPos);
        newPos.setX(qMax(newPos.x(), 0));
        newPos.setY(qMax(newPos.y(), 0));
        newPos.setX(qMin(newPos.x(), parentWidget()->width() - width()));
        newPos.setY(qMin(newPos.y(), parentWidget()->height() - height()));
        move(newPos);
    }
}

void ImageWidget::hightlight(){
    mask = new QWidget(this);
    mask->setStyleSheet("background-color: rgba(192, 192, 192, 0.3);");  // 半透明灰色
    mask->setFixedSize(this->size());  // 与 QLabel 同样大小

    // 将遮罩层放置在 QLabel 上
    mask->move(0, 0);  // 根据需要调整位置
    mask->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mask->show();
}
void ImageWidget::hightlightback(){
    if (mask)
        delete mask;
}


PreviewWindow::PreviewWindow(QWidget *parent) : QWidget(parent), ui(new Ui::PreviewWindow) {

    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    //自动释放内存
    setAttribute(Qt::WA_DeleteOnClose, true);

    ui->setupUi(this);
    ui->listWidget->viewport()->installEventFilter(this);

    connect(ui->saveButton,&QPushButton::clicked, this, &PreviewWindow::on_saveButton_clicked);
    connect(ui->addButton,&QPushButton::clicked, this, &PreviewWindow::on_addButton_clicked);
    connect(ui->useButton,&QPushButton::clicked, this, &PreviewWindow::on_useButton_clicked);

    QFile sfile(QCoreApplication::applicationDirPath()+"\\settings.json");
    if (!sfile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for reading:" << sfile.errorString();
    }

    // 读取文件内容并解析为QJsonDocument
    QByteArray sjsonData = sfile.readAll();
    QJsonDocument sjsonDoc = QJsonDocument::fromJson(sjsonData);

    sfile.close();

    QJsonObject setObj = sjsonDoc.object();

    backgroundimg = setObj["backgroundimg"].toString();
    backgrounddata = setObj["backgrounddata"].toString();

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

            QPixmap pixmap;
            if(obj["isCuted"].toBool()){
                pixmap = QPixmap(QCoreApplication::applicationDirPath()+obj["url"].toString());
            }
            else{
                pixmap = QPixmap(obj["url"].toString());
            }
            QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
            item->setIcon(QIcon(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            item->setSizeHint(QSize(150, 150));
            ui->listWidget->addItem(item); // 添加到 QListWidget

            QPixmap scaledPic = pixmap.scaled(obj["scale_w"].toInt(), obj["scale_h"].toInt(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            ImageWidget *imageWidget = new ImageWidget(ui->mainPictureWidget,&scaledPic);
            imageWidget->isCuted = obj["isCuted"].toBool();
            ImageWidgets.append(imageWidget);
            imageWidget->move(obj["pos_w"].toInt(),obj["pos_h"].toInt());
            imageWidget->show();
        }
    }

    //设置右栏的图标大小
    ui->listWidget->setIconSize(QSize(150, 150));


    QScreen *deviceScreen = QGuiApplication::primaryScreen();
    ui->mainPictureWidget->resize(deviceScreen->size()*0.8);

    QPixmap originalPixmap(QCoreApplication::applicationDirPath()+backgroundimg);
    QPixmap scaledPixmap = originalPixmap.scaled(ui->mainPictureWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 调整大小保持比例

    // 将 QPixmap 转换为 QImage，然后设置为背景
    ui->mainPictureWidget->setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(ui->mainPictureWidget->backgroundRole(), QBrush(scaledPixmap));
    ui->mainPictureWidget->setPalette(palette);


    ui->listWidget->move(ui->mainPictureWidget->x()+ui->mainPictureWidget->width()+20,ui->mainPictureWidget->y());
    ui->addButton->move(ui->listWidget->x()+10,ui->listWidget->y()+ui->listWidget->height()-50);
    ui->saveButton->move(ui->listWidget->x()+50,ui->listWidget->y()+ui->listWidget->height()+10);
    ui->useButton->move(ui->listWidget->x()+50,ui->listWidget->y()+ui->listWidget->height()+50);

    QPushButton* cutButton = new QPushButton(this);
    cutButton->setGeometry(ui->listWidget->x()+102,ui->listWidget->y()+ui->listWidget->height()-50,92,28);
    cutButton->setText("截取");

    connect(cutButton,&QPushButton::clicked, this, [this](){
        if(!isEditBind && !isCut){
            isCut = 1 ;
            DrawingView* view = new DrawingView(this);
            connect(view, &DrawingView::sendImg, this, &PreviewWindow::updateCutImg);
            connect(view, &QWidget::destroyed, this, [this](){
                this->isCut = 0;
            });
            view->show();
        }
    });

}

PreviewWindow::~PreviewWindow(){
    delete ui;
}

void PreviewWindow::on_useButton_clicked(){
    if(!isEditBind && !isCut){
    on_saveButton_clicked();
    QProcess::startDetached(QCoreApplication::applicationFilePath(), QCoreApplication::arguments());
    QCoreApplication::exit();
    }
}

void PreviewWindow::on_saveButton_clicked()
{
    // 保存布局逻辑
    for(int i=0;i<ImageWidgets.count();i++){
        QJsonObject obj = datas.at(i).toObject();

        // 修改对象的 键
        obj["pos_w"] = ImageWidgets[i]->pos().x();
        obj["pos_h"] = ImageWidgets[i]->pos().y();
        obj["scale_w"] = ImageWidgets[i]->width();
        obj["scale_h"] = ImageWidgets[i]->height();

        obj["isCuted"] = ImageWidgets[i]->isCuted;

        // 替换回数组中
        datas.replace(i, obj);

        // qDebug()<<obj["bindtype"].toString();
        // qDebug()<<obj["bindurl"].toString();
    }

    QFile file(QCoreApplication::applicationDirPath()+backgrounddata);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing:" << file.errorString();
        return;
    }
    QJsonDocument dataDoc(datas);
    file.write(dataDoc.toJson());
    file.close();

}

void PreviewWindow::on_addButton_clicked(){
    if(isEditBind || isCut)return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "/", tr("Image Files (*.png *.jpg *.bmp)"));
    if (!fileName.isEmpty()){
        QJsonObject obj = QJsonObject();

        QPixmap pixmap(fileName);
        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(QIcon(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        item->setSizeHint(QSize(150, 150));
        ui->listWidget->addItem(item); // 添加到 QListWidget

        QPixmap scaledPic = pixmap.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        ImageWidget *imageWidget = new ImageWidget(ui->mainPictureWidget,&scaledPic);
        ImageWidgets.append(imageWidget);
        imageWidget->move(ui->mainPictureWidget->width()/2,ui->mainPictureWidget->height()/2);
        imageWidget->show();

        obj["url"] = fileName;
        datas.append(obj);
    }
}

void PreviewWindow::updateBindType(const QString &newString){
    bindtype = newString;

    QJsonObject obj = datas.at(bechangedobj).toObject();
    // 修改对象的 键
    obj["bindtype"] = bindtype;
    // 替换回数组中
    datas.replace(bechangedobj, obj);
};
void PreviewWindow::updateBindUrl(const QString &newString){
    bindurl = newString;

    QJsonObject obj = datas.at(bechangedobj).toObject();
    // 修改对象的 键
    obj["bindurl"] = bindurl;
    // 替换回数组中
    datas.replace(bechangedobj, obj);
};
void PreviewWindow::updateModname(const QString &newString){
    QString modname = newString;

    QJsonObject obj = datas.at(bechangedobj).toObject();
    // 修改对象的 键
    obj["ModName"] = modname;
    // 替换回数组中
    datas.replace(bechangedobj, obj);
};
void PreviewWindow::showContextMenu(QListWidgetItem *item, const QPoint &pos) {
    QMenu menu;
    menu.addAction("编辑绑定", [this, item]() {
        // 编辑操作
        this->isEditBind = true;
        bechangedobj = ui->listWidget->row(item);
        BindUrlSetWindow* bindsetwindow=new BindUrlSetWindow(this);
        bindsetwindow->show();
        connect(bindsetwindow, &BindUrlSetWindow::sendBindType, this, &PreviewWindow::updateBindType);
        connect(bindsetwindow, &BindUrlSetWindow::sendBindUrl, this, &PreviewWindow::updateBindUrl);
        connect(bindsetwindow, &BindUrlSetWindow::sendModName, this, &PreviewWindow::updateModname);
        connect(bindsetwindow, &QDialog::finished, this, [this](){
            this->isEditBind = false;  // 对话框关闭时重新启用父窗口
        });

    });
    menu.addAction("删除", [this, item]() {
        /* 删除操作 */
        int i=ui->listWidget->row(item);
        this->ui->listWidget->takeItem(i);
        ImageWidget* tmp = ImageWidgets.at(i);
        ImageWidgets.removeAt(i);
        delete tmp;
        datas.removeAt(i);
        delete item;
    });
    menu.addAction("绑定信息", [this, item]() {
        /* 更多信息 */
        int i=ui->listWidget->row(item);
        QJsonObject obj=datas[i].toObject();
        information* inforwindow = new information(this,obj);
        inforwindow->exec();
    });
    menu.exec(pos);
}

bool PreviewWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->listWidget->viewport() && event->type() == QEvent::MouseButtonPress && !isEditBind) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            QPoint pos = mouseEvent->pos();
            QListWidgetItem *item = ui->listWidget->itemAt(pos);
            if (item) {
                showContextMenu(item, ui->listWidget->mapToGlobal(pos));
                return true;
            }
        }
    if (mouseEvent->button() == Qt::LeftButton) {
            QPoint pos = mouseEvent->pos();
            QListWidgetItem *item = ui->listWidget->itemAt(pos);
            if (item) {
                if (last_highlight != -1){
                    ImageWidgets.at(last_highlight)->hightlightback();
                }
                int i=ui->listWidget->row(item);
                ImageWidgets.at(i)->hightlight();
                last_highlight = i;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

BindUrlSetWindow::BindUrlSetWindow(QWidget *parent):QDialog(parent),ui(new Ui::BindUrlSetWindow){
    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);


    ui->setupUi(this);

    connect(ui->bindtypechoose, QOverload<int>::of(&QComboBox::activated), this, &BindUrlSetWindow::performAction);
    connect(ui->pushButton, &QPushButton::clicked, this, &BindUrlSetWindow::choose_file);
    bindtype=QString("file");

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if(ui->bindtypechoose->currentIndex() == 0 || ui->bindtypechoose->currentIndex() == 2){
            bindtype = "file";
        }
        else if(ui->bindtypechoose->currentIndex() == 1){
            bindtype = "fdir";
        }
        else if(ui->bindtypechoose->currentIndex() == 3){
            bindtype = "icon";
        }
        else if(ui->bindtypechoose->currentIndex() == 4){
            bindtype = "QTMod";
        }
        if(bindtype == "icon"){
            bindurl = "";
            emit sendBindType(bindtype);
            emit sendBindUrl(bindurl);
            return;
        }
        else if (bindtype == "QTMod"){
            emit sendBindType(bindtype);
            emit sendBindUrl(bindurl);
            emit sendModName(modname);
        }
        bindurl = ui->bindurltext->toPlainText();
        bindurl.replace("\\", "/");
        QFileInfo fileInfo(bindurl);
        if(fileInfo.exists()){
            if(fileInfo.isFile() && bindtype == "file"){
                emit sendBindType(bindtype);
                emit sendBindUrl(bindurl);
            }
            else if(fileInfo.isDir() && (bindtype == "fdir")){
                emit sendBindType(bindtype);
                emit sendBindUrl(bindurl);
            }
        }
    });
}


BindUrlSetWindow::~BindUrlSetWindow(){
    delete ui;
}

void BindUrlSetWindow::performAction(int index) {
    ui->pushButton->setEnabled(true);
    ui->bindurltext->setEnabled(true);
    if(index==3){
        ui->bindurltext->clear();
        ui->bindurltext->setEnabled(false);
        ui->pushButton->setEnabled(false);
        bindtype=QString("icon");
        bindurl = "";
    }
    else if(index == 2){
        disconnect(ui->pushButton, &QPushButton::clicked, this, nullptr);
        connect(ui->pushButton, &QPushButton::clicked, this, &BindUrlSetWindow::choose_lnk);
    }
    else if(index==1){
        disconnect(ui->pushButton, &QPushButton::clicked, this, nullptr);
        connect(ui->pushButton, &QPushButton::clicked, this, &BindUrlSetWindow::choose_fdir);
        bindtype=QString("fdir");
    }
    else if (index==0){
        disconnect(ui->pushButton, &QPushButton::clicked, this, nullptr);
        connect(ui->pushButton, &QPushButton::clicked, this, &BindUrlSetWindow::choose_file);
        bindtype=QString("file");
    }
    else if(index == 4){
        ui->pushButton->setEnabled(false);
        ui->bindurltext->setEnabled(false);
        ModWidget* modselectwindow = new ModWidget(this);
        modselectwindow->show();
        connect(modselectwindow, &ModWidget::itemDoubleClicked, this, [this,modselectwindow](const QString &name, const QString &url) {
            bindurl = url;
            modname = name;
            qDebug()<<name;
            ui->bindurltext->setText(name);
            modselectwindow->close();
        });
    }
}

void BindUrlSetWindow::choose_lnk(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a lnk"), QDir::homePath(), tr("Lnk Files (*.lnk *.url)"));
    if (!fileName.isEmpty()) {
        ui->bindurltext->setText(fileName);
        bindurl = fileName;
    }
};
void BindUrlSetWindow::choose_file(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a file"), QDir::homePath());
    if (!fileName.isEmpty()) {
        ui->bindurltext->setText(fileName);
        bindurl = fileName;
    }
};
void BindUrlSetWindow::choose_fdir(){
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Select a folder"), QDir::homePath());
    if (!fileName.isEmpty()) {
        ui->bindurltext->setText(fileName);
        bindurl = fileName;
    }
};

information::information(QWidget *parent,const QJsonObject data):QDialog(parent),ui(new Ui::information){
    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->setupUi(this);

    ui->textEdit->setText(data["url"].toString());
    QString type = data["bindtype"].toString();
    if (type==QString("icon"))
        ui->textEdit_2->setText("切换桌面图标隐藏状态");
    else if (type==QString("fdir"))
        ui->textEdit_2->setText("文件夹");
    else if(type==QString("file"))
        ui->textEdit_2->setText("文件/快捷方式");
    else if(type==QString("QTMod")){
        ui->textEdit_2->setText("Mods");
    }
    ui->textEdit_3->setText(data["bindurl"].toString());

}

information::~information(){
    delete ui;
}

DrawingView::DrawingView(QWidget *parent) : QWidget(parent), pathItem(nullptr) {
    //自动释放内存
    setAttribute(Qt::WA_DeleteOnClose, true);

    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    setWindowFlags(Qt::Window);

    PreviewWindow *father = static_cast<PreviewWindow *>(parent);


    view = new QGraphicsView(this);
    view->setAttribute(Qt::WA_TransparentForMouseEvents);

    view->setFixedSize(QGuiApplication::primaryScreen()->size()*0.8); // 你可以根据需要调整大小
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameShape(QFrame::NoFrame);
    view->setRenderHint(QPainter::Antialiasing);

    scene = new QGraphicsScene(view);

    scene->addPixmap(QPixmap(QCoreApplication::applicationDirPath()+father->backgroundimg).scaled(QGuiApplication::primaryScreen()->size()*0.8,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    view->setScene(scene);

    path = new QPainterPath();

    QPushButton* saveButton = new QPushButton(this);
    saveButton->setGeometry(view->x()+view->size().width()+20,view->y()+view->height()-40,100,30);
    saveButton->setText("保存");

    QPushButton* clearButton = new QPushButton(this);
    clearButton->setGeometry(view->x()+view->size().width()+20,view->y()+view->height()-100,100,30);
    clearButton->setText("清除");

    connect(saveButton,&QPushButton::clicked,this,&DrawingView::onSaveButtonClicked);
    connect(clearButton,&QPushButton::clicked,this,[this](){
        if(isFinishOne){
            path->clear();
            if(filledPathItem)
            delete filledPathItem;

            delete pathItem;
            pathItem = nullptr;

            filledPathItem = nullptr;

            isFinishOne = 0;
        }
    });

    move(200,200);
}
DrawingView::~DrawingView(){
    if(pathItem)delete pathItem;
    if(path)delete path;
}

void DrawingView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !isFinishOne) {
        path->moveTo(view->mapToScene(event->pos()));
        pathItem = new QGraphicsPathItem();
        scene->addItem(pathItem);
    }
}

void DrawingView::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !isFinishOne) {
        path->lineTo(view->mapToScene(event->pos()));
        pathItem->setPath(*path);
    }
}

void DrawingView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && pathItem && !isFinishOne) {
        path->lineTo(view->mapToScene(event->pos()));
        path->closeSubpath();
        pathItem->setPath(*path);

        QPainterPath closedPath = pathItem->path();
        // if (closedPath.contains(closedPath.pointAtPercent(0.5))) {  // 判断是否为封闭路径
            filledPathItem = new QGraphicsPathItem(closedPath);
        filledPathItem->setBrush(QColor(255,0,0,54));
            scene->addItem(filledPathItem);

            isFinishOne = 1;
        // }

        // path->clear();
        // delete pathItem;
        // pathItem = nullptr;
    }
}

void DrawingView::onSaveButtonClicked(){
    if(isFinishOne){
        PreviewWindow *father = static_cast<PreviewWindow *>(this->parent());
        QPixmap pixmap = QPixmap(QCoreApplication::applicationDirPath() + father->backgroundimg).scaled(QGuiApplication::primaryScreen()->size()*0.8,Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 获取path的边界框
        QRectF rect = pathItem->path().boundingRect();

        // 创建一个透明的pixmap
        QPixmap croppedPixmap(rect.size().toSize());
        croppedPixmap.fill(Qt::transparent);

        // 使用QPainter在pixmap上绘制path
        QPainter painter(&croppedPixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        // 使用QBitmap设置裁剪区域
        QBitmap bitmap(rect.size().toSize());
        bitmap.clear();
        QPainter bitmapPainter(&bitmap);
        bitmapPainter.setRenderHint(QPainter::Antialiasing);
        bitmapPainter.setBrush(Qt::color1);
        bitmapPainter.drawPath(pathItem->path().translated(-rect.topLeft()));
        painter.setClipRegion(QRegion(bitmap));

        // 在裁剪后的 QPixmap 上绘制原始 pixmap 的相应部分
        painter.drawPixmap(0, 0, pixmap, rect.x(), rect.y(), rect.width(), rect.height());

        emit sendImg(croppedPixmap,rect.x(),rect.y());

        this->hide();
    }
};

void PreviewWindow::updateCutImg(const QPixmap& pixmap,const int& x,const int& y){
    QPixmap nowpixmap = pixmap;

    QString fileName = "/resource/cutimgs/" + QUuid::createUuid().toString().remove("{").remove("}") + ".png";

    QJsonObject obj = QJsonObject();

    nowpixmap.save(QCoreApplication::applicationDirPath() +fileName,"PNG");

    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon(QIcon(nowpixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    item->setSizeHint(QSize(150, 150));
    ui->listWidget->addItem(item); // 添加到 QListWidget


    ImageWidget *imageWidget = new ImageWidget(ui->mainPictureWidget,&nowpixmap);
    imageWidget->isCuted = 1;
    ImageWidgets.append(imageWidget);
    imageWidget->move(x,y);
    imageWidget->show();

    obj["url"] = fileName;
    datas.append(obj);

    isCut = 0;
}


ModWidget::ModWidget(QWidget *parent)
    : QWidget(parent), listView(new QListView(this)), model(new QStandardItemModel(this))
{
    //自动释放内存
    setAttribute(Qt::WA_DeleteOnClose, true);

    //防止关闭整个程序
    setAttribute(Qt::WA_QuitOnClose, false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(listView);
    setLayout(layout);

    listView->setModel(model);
     listView->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁用编辑功能
    connect(listView, &QListView::doubleClicked, this, &ModWidget::onItemDoubleClicked);

    QPushButton *closeButton = new QPushButton("取消", this);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    layout->addWidget(closeButton);

    loadJsonData();
}

ModWidget::~ModWidget()
{
}

void ModWidget::loadJsonData()
{
    QFile file(QCoreApplication::applicationDirPath()+"/resource/Mods.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open Mods.json");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        QMessageBox::critical(this, "Error", "Mods.json is not a valid JSON array");
        return;
    }

    modsArray = doc.array();
    for (const QJsonValue &value : modsArray) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            QString name = obj["name"].toString();
            QStandardItem *item = new QStandardItem(name);
            model->appendRow(item);
        }
    }
}

void ModWidget::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QString name = model->data(index, Qt::DisplayRole).toString();
    for (const QJsonValue &value : modsArray) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            if (obj["name"].toString() == name) {
                QString url = obj["url"].toString();
                emit itemDoubleClicked(name, url);
                return;
            }
        }
    }
}
