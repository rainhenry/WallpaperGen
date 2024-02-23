#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>

#include "CQtAI.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //  缩放参数
    typedef struct
    {
        int press_x;
        int press_y;
        int x;
        int y;
        double factor;
    }SScaleParam;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    //  QtAI对象
    CQtAI qtai;

    //  定时器
    QTimer* timer;

    //  当前忙
    bool is_cur_busy;

    //  只进行翻译
    bool only_tsl_flag;

    //  输出图像的副本
    QImage out_img_cache;

    //  鼠标左键按下的状态
    bool press_mouse_left_btn;
    int press_mouse_pos_x;
    int press_mouse_pos_y;

    //  输出图像的缩放参数
    SScaleParam out_img_scale;


private slots:
    //  当AI环境就绪
    void slot_OnAIEnvReady(void);

    //  当完成一次翻译
    void slot_OnTranslateCn2EnFinish(QString out_text, qint64 run_time_ns);

    //  当完成一次文字生成壁纸
    void slot_OnTextToWallpaplerFinish(qint64 run_time_ns);

    //  定时器
    void slot_timeout();


    //  通过英文文本生成壁纸
    void on_pushButton_byen_clicked();

    //  通过中文文本生成壁纸
    void on_pushButton_bycn_clicked();

    //  仅仅将中文翻译成英文
    void on_pushButton_tslonly_clicked();

    //  打开壁纸文件
    void on_pushButton_open_clicked();

protected:
    //  鼠标相关
    void mousePressEvent(QMouseEvent* event);          //  按下
    void mouseReleaseEvent(QMouseEvent* event);        //  抬起
    void mouseDoubleClickEvent(QMouseEvent* event);    //  双击
    void mouseMoveEvent(QMouseEvent* event);           //  移动
    void wheelEvent(QWheelEvent* event);               //  滚轮


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
