#ifndef SELECTWIDGET_H
#define SELECTWIDGET_H


#include <QWidget>
#include <QPen>
#include <QLabel>
#include <functional>


namespace Ui {
class SelectRectWidget;
}
class SelectRectWidget : public QWidget
{
    Q_OBJECT


public:
    explicit SelectRectWidget(QWidget *parent = nullptr);
    ~SelectRectWidget();


    /**
     * @brief setFixCenterRectRatio 设置选择框为固定比例
     * @param ratio 比例值width:height，比如16:9
     */
    void setFixCenterRectRatio(float ratio);


    //中间镂空区域的Rect
    QRect centerRect() const;


    /**
     * @brief setSelectRectChange 设置选择框变化时回调函数
     * @param selectRectChange
     */
    void setSelectRectChange(const std::function<void (QRect rect)> &selectRectChange);


protected:
    virtual void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *, QEvent *) override;


private:
    //根据centerRect_设置各种Rect
    void setRects();
    //检测centerRect区域值是否有效
    void checkCenterRect();


private:
    Ui::SelectRectWidget *ui;
    QRect leftRect_;//中间镂空区域的左侧蒙层Rect
    QRect topRect_;//中间镂空区域的顶部蒙层Rect
    QRect rightRect_;//中间镂空区域的右侧蒙层Rect
    QRect bottomRect_;//中间镂空区域的底部蒙层Rect
    QRect centerRect_;//中间镂空区域的Rect
    QRect preCenterRect_;//记录上一次中间镂空区域的Rect，避免中间区域相同时重复绘制
    QColor outsideColor_;//外部蒙层的颜色值
    QColor insideColor_;//内部镂空区域的颜色值
    QColor dashColor_;//中间镂空区域的虚线边框的颜色值


    QPen dashPen_;//中间镂空区域的虚线边框的Pen（可以设置虚线长度和间隔长度等）
    QVector<qreal> dashes_;//dashPen_的虚线长度和间隔长度设置


    QLabel * labelLeftBottom_ = nullptr;//可以触发缩放，镂空区域虚线边框左下部的顶点
    QLabel * labelRightBottom_ = nullptr;//可以触发缩放，镂空区域虚线边框右下部的顶点
    QLabel * labelLeftTop_ = nullptr;//可以触发缩放，镂空区域虚线边框左上部的顶点
    QLabel * labelRightTop_ = nullptr;//可以触发缩放，镂空区域虚线边框右上部的顶点
    QRect    dragMoveLocation_;//保存centerRect_移动时的初始值
    QPoint   dragZoomPos_;//四个顶点被QMouseEvent press时的初始值
    bool     dragZoomRunning_;//四个顶点被QMouseEvent press时的
    QRect    dragZoomLocation_;//四个顶点被QMouseEvent press时，保存centerRect_的初始值
    bool     press_ = false;//鼠标press centerRect_区域时为true
    QPoint   startPos_;//保存鼠标press时的QMouseEvent初始值
    float    fixCenterRectRatio_;//选择框固定比例值缩放，不设置该值时非固定比例缩放
    std::function<void (QRect rect)> selectRectChange_;//选择框变化时回调函数


    static int  sDragDotWidth_;      //选择框边线上几个可以放大缩小的点大小
    static int  sCenterRectMinWidth_;//选择框的最小宽度和高度，防止选择框边线重叠
    static int  sLineWidth_;//选择框边线宽度
    static int  sDragDotOffset_;//四个顶点的offset值


protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
};


#endif // SELECTWIDGET_H


