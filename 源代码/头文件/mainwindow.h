#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class DoubleClickFilter : public QObject
{
    Q_OBJECT
public:
    DoubleClickFilter(QObject* parent=nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

class ImageView : public QWidget {
    Q_OBJECT
public:
    ImageView(QWidget *parent = nullptr,QJsonObject obj = QJsonObject());
    ~ImageView();

    bool event(QEvent *event) override;
    void hightlight();
    void hightlightback();

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void openFileLocation();

private:
    QLabel *imageView;
    QVector<QWidget*> masks;
    QJsonObject data;
    QString name;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void onChangeWallpaperClicked();
    QVector<ImageView*> imageviews;
    QJsonArray datas;
    DoubleClickFilter* doubleClickFilter;
};
#endif // MAINWINDOW_H
