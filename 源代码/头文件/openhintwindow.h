#ifndef OPENHINTWINDOW_H
#define OPENHINTWINDOW_H

#include <QDialog>

namespace Ui {
class openhintwindow;
}

class openhintwindow : public QDialog
{
    Q_OBJECT

public:
    explicit openhintwindow(QWidget *parent = nullptr);
    ~openhintwindow();

    void closeAndDelete();

private:
    Ui::openhintwindow *ui;
};

#endif // OPENHINTWINDOW_H
