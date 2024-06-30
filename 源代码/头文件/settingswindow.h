#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QJsonObject>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class settingswindow;
}
QT_END_NAMESPACE

class settingsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit settingsWindow(QWidget *parent = nullptr);
    ~settingsWindow();

    bool eventFilter(QObject *watched, QEvent *event) override;
    void showContextMenu(QListWidgetItem *item, const QPoint &pos);
private slots:
    void onItemChanged(QListWidgetItem *item);
public slots:
    void updateBackgroundImg(const QString &newString);
private:
    Ui::settingswindow* ui;
    QJsonObject setObj;
};

#endif // SETTINGSWINDOW_H
