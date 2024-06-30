#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QListWidget>
#include <QLabel>
#include <QSizeGrip>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsPixmapItem>
#include <QStandardItemModel>

#include <QMouseEvent>


QT_BEGIN_NAMESPACE
namespace Ui {
class PreviewWindow;
class BindUrlSetWindow;
class information;
}
QT_END_NAMESPACE


class CustomSizeGrip:public QWidget {
    Q_OBJECT
public:
    explicit CustomSizeGrip(QWidget *parent);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QWidget *targetWidget;
    QWidget *tarmask;
    bool dragging = false;
    QPoint lastMousePos;
};



class ImageWidget : public QWidget {
    Q_OBJECT
public:
    ImageWidget(QWidget *parent = nullptr,QPixmap *pixmap = nullptr);
    ~ImageWidget();

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void hightlight();
    void hightlightback();

    bool isCuted = 0;

private:
    QLabel *imageView;
    QPoint m_lastPos;
    QWidget *mask;
};

class BindUrlSetWindow:public QDialog {
    Q_OBJECT
public:
    explicit BindUrlSetWindow(QWidget *parent = nullptr);
    ~BindUrlSetWindow();

    void performAction(int index);
    void choose_lnk();
    void choose_file();
    void choose_fdir();
signals:
    void sendBindType(const QString &bindtype);
    void sendBindUrl(const QString &bindurl);
    void sendModName(const QString &ModName);


private:
    Ui::BindUrlSetWindow *ui;
    QString bindtype;
    QString bindurl;
    QString modname;
};

class information:public QDialog {
    Q_OBJECT
public:
    explicit information(QWidget *parent = nullptr,const QJsonObject data=QJsonObject());
    ~information();

private:
    Ui::information *ui;

};

class PreviewWindow : public QWidget {
        Q_OBJECT
public:
    explicit PreviewWindow(QWidget *parent = nullptr);
    ~PreviewWindow();
    QString backgroundimg;

public slots:
    void updateBindType(const QString &newString);
    void updateBindUrl(const QString &newString);
    void updateModname(const QString &newString);
    void updateCutImg(const QPixmap& pixmap,const int& x,const int& y);

private:
    Ui::PreviewWindow *ui;
    QVector<QPixmap> smallPics;
    QVector<ImageWidget*> ImageWidgets;
    QJsonArray datas;

    bool eventFilter(QObject *watched, QEvent *event);
    void showContextMenu(QListWidgetItem *item, const QPoint &pos);

    void on_saveButton_clicked();
    void on_addButton_clicked();
    void on_useButton_clicked();

    int last_highlight = -1;
    QString bindtype;
    QString bindurl;
    QString backgrounddata;
    int bechangedobj=-1;
    bool isEditBind = false;
    bool isCut = false;
};

class DrawingView : public QWidget
{
    Q_OBJECT

public:
    DrawingView(QWidget *parent = nullptr);
    ~DrawingView();

signals:
    void sendImg(const QPixmap& pixmap,const int x,const int y);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void onSaveButtonClicked();

private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    QPainterPath *path;
    QGraphicsPathItem *pathItem;
    QGraphicsPathItem *filledPathItem;

    bool isFinishOne = 0;
};

class ModWidget : public QWidget
{
    Q_OBJECT

public:
    ModWidget(QWidget *parent = nullptr);
    ~ModWidget();

signals:
    void itemDoubleClicked(const QString &name, const QString &url);

private slots:
    void onItemDoubleClicked(const QModelIndex &index);

private:
    void loadJsonData();

    QListView *listView;
    QStandardItemModel *model;
    QJsonArray modsArray;
};

#endif // PREVIEWWINDOW_H
