#ifndef SELECTWINDOW_H
#define SELECTWINDOW_H

#include <QWidget>
#include <selectwidget.h>

namespace Ui {
class selectwindow;
}

class selectwindow : public QWidget
{
    Q_OBJECT

public:
    explicit selectwindow(QWidget *parent = nullptr,QString imgurl = "");
    ~selectwindow();

    void on_sureButton_clicked();

signals:
    void sendImgname(const QString &imgname);

private:
    Ui::selectwindow *ui;
    SelectRectWidget* selectwidget;
    QPixmap scaledPixmap;
    QPixmap originalPixmap;
    float ratiox;
    float ratioy;
};

#endif // SELECTWINDOW_H
