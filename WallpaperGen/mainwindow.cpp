#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //  初始化UI
    QImage img_out(ui->label_output->width(), ui->label_output->height(), QImage::Format_RGB888);
    img_out.fill(QColor(0, 0, 255));
    ui->label_output->setPixmap(QPixmap::fromImage(img_out));

    //  设置当前忙状态
    is_cur_busy = true;

    //  提示
    ui->progressBar->setFormat("Initializing environment, please wait...");

    //  关闭控件
    ui->pushButton_bycn       ->setEnabled(false);
    ui->pushButton_byen       ->setEnabled(false);
    ui->pushButton_tslonly    ->setEnabled(false);
    ui->pushButton_open       ->setEnabled(false);
    ui->spinBox_maxres        ->setEnabled(false);
    ui->lineEdit_cachefilename->setEnabled(false);

    //  设置分辨率默认值
    ui->spinBox_maxres->setValue(1920);

    //  初始化AI环境
    connect(&qtai, &CQtAI::send_translate_cn2en_finish,   this, &MainWindow::slot_OnTranslateCn2EnFinish);
    connect(&qtai, &CQtAI::send_text_to_wallpaper_finish, this, &MainWindow::slot_OnTextToWallpaplerFinish);
    connect(&qtai, &CQtAI::send_environment_ready,        this, &MainWindow::slot_OnAIEnvReady);
    //qtai.setStackSize(1024*1024*1024);
    qtai.Init();
    qtai.start();

    //  定时器初始化
    timer = new QTimer(this);
    timer->setInterval(30);
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
    timer->start();
}

MainWindow::~MainWindow()
{
    //  释放定时器
    if(timer != nullptr)
    {
        timer->stop();
        delete timer;
        timer = nullptr;
    }

    //  释放AI资源
    qtai.Release();

    //  释放UI资源
    delete ui;
}

//  当AI环境就绪
void MainWindow::slot_OnAIEnvReady(void)
{
    //  开启UI
    ui->pushButton_bycn       ->setEnabled(true);
    ui->pushButton_byen       ->setEnabled(true);
    ui->pushButton_tslonly    ->setEnabled(true);
    ui->pushButton_open       ->setEnabled(true);
    ui->spinBox_maxres        ->setEnabled(true);
    ui->lineEdit_cachefilename->setEnabled(true);

    //  初始化缩放参数
    out_img_scale.press_x = 0;
    out_img_scale.press_y = 0;
    out_img_scale.x       = 0;
    out_img_scale.y       = 0;
    out_img_scale.factor  = 1.0;

    //  提示
    is_cur_busy = false;
    ui->progressBar->setFormat("Ready!!");
}

//  当完成一次翻译
void MainWindow::slot_OnTranslateCn2EnFinish(QString out_text, qint64 run_time_ns)
{
    //  检查文字内容的最后1个字符
    if(!out_text.isEmpty())
    {
        //  当为英文句号的时候，删除该字符
        if(*(out_text.end()-1) == '.')
        {
            out_text.remove(out_text.length() - 1, 1);
        }
    }

    //  更新输出文字内容
    ui->textEdit_en->setText(out_text);
    std::string tmp_str = out_text.toStdString();
    qDebug("out_text = [%s]", tmp_str.c_str());

    //  当为只进行翻译操作
    if(only_tsl_flag)
    {
        //  忙完
        is_cur_busy = false;

        //  开启UI
        ui->pushButton_bycn       ->setEnabled(true);
        ui->pushButton_byen       ->setEnabled(true);
        ui->pushButton_tslonly    ->setEnabled(true);
        ui->pushButton_open       ->setEnabled(true);
        ui->spinBox_maxres        ->setEnabled(true);
        ui->lineEdit_cachefilename->setEnabled(true);

        //  设置状态
        //  当为正常的文字内容
        if(!out_text.isEmpty())
        {
            long long ms = run_time_ns / 1000000LL;
            int sec = static_cast<int>(ms / 1000);
            int min = sec / 60;
            int hour = min / 60;
            QString str = QString::asprintf("Translate Done(%d:%02d:%02d.%03lld)",
                                            hour, min % 60, sec % 60, ms % 1000LL
                                           );
            ui->progressBar->setFormat(str);
        }
        //  当输出文字内容为空
        else
        {
            //  提示错误
            ui->progressBar->setFormat("Translation result is empty!!");
        }
    }
    //  否则直接执行文字生成壁纸
    else
    {
        //  当为正常的文字内容
        if(!out_text.isEmpty())
        {
            //  执行
            is_cur_busy = true;
            qtai.ExTextToWallpaper(out_text, ui->spinBox_maxres->value(), ui->lineEdit_cachefilename->text());

            //  设置状态
            ui->progressBar->setFormat("Generateing Wallpaper...");

            //  关闭控件
            ui->pushButton_bycn       ->setEnabled(false);
            ui->pushButton_byen       ->setEnabled(false);
            ui->pushButton_tslonly    ->setEnabled(false);
            ui->pushButton_open       ->setEnabled(false);
            ui->spinBox_maxres        ->setEnabled(false);
            ui->lineEdit_cachefilename->setEnabled(false);
        }
        //  当输出文字内容为空
        else
        {
            //  忙完
            is_cur_busy = false;

            //  开启UI
            ui->pushButton_bycn       ->setEnabled(true);
            ui->pushButton_byen       ->setEnabled(true);
            ui->pushButton_tslonly    ->setEnabled(true);
            ui->pushButton_open       ->setEnabled(true);
            ui->spinBox_maxres        ->setEnabled(true);
            ui->lineEdit_cachefilename->setEnabled(true);

            //  提示错误
            ui->progressBar->setFormat("Translation result is empty!!");
        }
    }
}

//  当完成一次文字生成壁纸
void MainWindow::slot_OnTextToWallpaplerFinish(qint64 run_time_ns)
{
    //  忙完
    is_cur_busy = false;

    //  开启UI
    ui->pushButton_bycn       ->setEnabled(true);
    ui->pushButton_byen       ->setEnabled(true);
    ui->pushButton_tslonly    ->setEnabled(true);
    ui->pushButton_open       ->setEnabled(true);
    ui->spinBox_maxres        ->setEnabled(true);
    ui->lineEdit_cachefilename->setEnabled(true);

    //  设置状态
    long long ms = run_time_ns / 1000000LL;
    int sec = static_cast<int>(ms / 1000);
    int min = sec / 60;
    int hour = min / 60;
    QString str = QString::asprintf("Generate Wallpaper Done(%d:%02d:%02d.%03lld)",
                                    hour, min % 60, sec % 60, ms % 1000LL
                                   );
    ui->progressBar->setFormat(str);

    //  保存输出结果到图像副本
    out_img_cache.load(ui->lineEdit_cachefilename->text());

    //  绘制输出的图像
    QImage img = out_img_cache;
    img = img.scaled(ui->label_output->width(), ui->label_output->height(), Qt::KeepAspectRatio);
    ui->label_output->setPixmap(QPixmap::fromImage(img));

}

//  定时器
void MainWindow::slot_timeout()
{
    //  进度变量
    static int cur_proc = 100;

    //  当忙
    if(is_cur_busy)
    {
        if(cur_proc >= 100) cur_proc = 0;
        else                cur_proc = cur_proc + 1;
    }
    //  当忙完
    else
    {
        cur_proc = 100;
    }

    //  更新到控件
    ui->progressBar->setValue(cur_proc);
}


//  鼠标按下处理
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    //  当处理忙
    if(is_cur_busy) return;

    if(Qt::LeftButton == event->button())
    {
        //  记录按下状态
        press_mouse_left_btn = true;

        //  在输出图像控件上的处理
        QRect rect_out_img = ui->label_output->geometry();                 //  获取该图像控件的位置信息
        bool on_out_img = rect_out_img.contains(event->x(), event->y());   //  判断鼠标是否在该控件内
        int out_img_pos_x = event->x() - rect_out_img.x();                 //  计算相对坐标x
        int out_img_pos_y = event->y() - rect_out_img.y();                 //  计算相对坐标y

        //  当在该图像中，并且图像可用
        if(on_out_img && (!out_img_cache.isNull()))
        {
            //  记录按下时刻的坐标
            out_img_scale.press_x = out_img_pos_x;
            out_img_scale.press_y = out_img_pos_y;
        }
    }
}

//  鼠标抬起处理
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if(Qt::LeftButton == event->button())
    {
        //  释放按下状态
        press_mouse_left_btn = false;
    }
}

//  鼠标双击处理
void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    //  当处理忙
    if(is_cur_busy) return;

    //  复位缩放参数
    out_img_scale.press_x = 0;
    out_img_scale.press_y = 0;
    out_img_scale.x       = 0;
    out_img_scale.y       = 0;
    out_img_scale.factor  = 1.0;

    //  在输出图像控件上的处理
    QRect rect_out_img = ui->label_output->geometry();                 //  获取该图像控件的位置信息
    bool on_out_img = rect_out_img.contains(event->x(), event->y());   //  判断鼠标是否在该控件内

    //  在输出图片上
    if(on_out_img)
    {
        //  复位显示
        QImage img = out_img_cache;
        img = img.scaled(ui->label_output->width(), ui->label_output->height(), Qt::KeepAspectRatio);
        ui->label_output->setPixmap(QPixmap::fromImage(img));
    }
}

//  鼠标移动处理
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    //  当处理忙
    if(is_cur_busy) return;

    //  在输出图像控件上的处理
    QRect rect_out_img = ui->label_output->geometry();                 //  获取该图像控件的位置信息
    bool on_out_img = rect_out_img.contains(event->x(), event->y());   //  判断鼠标是否在该控件内
    int out_img_pos_x = event->x() - rect_out_img.x();                 //  计算相对坐标x
    int out_img_pos_y = event->y() - rect_out_img.y();                 //  计算相对坐标y

    //  当在该图像中，并且图像可用, 并且为按下
    if(on_out_img && (!out_img_cache.isNull()) && press_mouse_left_btn)
    {
        //  计算新位置
        int new_x = (out_img_scale.press_x - out_img_pos_x) * (out_img_cache.width()  / ui->label_output->width())  / static_cast<int>(out_img_scale.factor) * 2;
        int new_y = (out_img_scale.press_y - out_img_pos_y) * (out_img_cache.height() / ui->label_output->height()) / static_cast<int>(out_img_scale.factor);

        //  保存
        out_img_scale.x += new_x;
        out_img_scale.y += new_y;

        //  限制
        int min_x = -(out_img_cache.width()  / 3);
        int min_y = -(out_img_cache.height() / 3);
        if(out_img_scale.x < min_x) out_img_scale.x = min_x;
        if(out_img_scale.y < min_y) out_img_scale.y = min_y;
        int max_x = out_img_cache.width()  + out_img_cache.width() / 3;
        int max_y = out_img_cache.height() + out_img_cache.height() / 3;
        if(out_img_scale.x > max_x) out_img_scale.x = max_x;
        if(out_img_scale.y > max_y) out_img_scale.y = max_y;

        //  执行
        QImage img = out_img_cache;
        img = img.copy(out_img_scale.x, out_img_scale.y, static_cast<int>(img.width() / out_img_scale.factor), static_cast<int>(img.height() / out_img_scale.factor));
        img = img.scaled(ui->label_output->width(), ui->label_output->height(), Qt::KeepAspectRatio);
        ui->label_output->setPixmap(QPixmap::fromImage(img));

        //  更新按下时刻的坐标
        out_img_scale.press_x = out_img_pos_x;
        out_img_scale.press_y = out_img_pos_y;
    }
}

//  鼠标滚轮处理
void MainWindow::wheelEvent(QWheelEvent* event)
{
    //  当处理忙
    if(is_cur_busy) return;

    //  在输出图像控件上的处理
    QRect rect_out_img = ui->label_output->geometry();                 //  获取该图像控件的位置信息
    bool on_out_img = rect_out_img.contains(event->x(), event->y());   //  判断鼠标是否在该控件内

    //  当在该图像中，并且图像可用
    if(on_out_img && (!out_img_cache.isNull()))
    {
        //  当放大
        if(event->delta() > 0)
        {
            //  在范围内
            if(out_img_scale.factor < 10.0)
            {
                out_img_scale.factor += 0.5;
            }
            //  超出放大范围
            else
            {
                out_img_scale.factor = 10.0;
            }
        }
        //  当缩小
        else
        {
            //  在范围内
            if(out_img_scale.factor > 1.0)
            {
                out_img_scale.factor -= 0.5;
            }
            //  超出缩小范围
            else
            {
                out_img_scale.factor = 1.0;
            }
        }

        //  执行
        QImage img = out_img_cache;
        img = img.copy(out_img_scale.x, out_img_scale.y, static_cast<int>(img.width() / out_img_scale.factor), static_cast<int>(img.height() / out_img_scale.factor));
        img = img.scaled(ui->label_output->width(), ui->label_output->height(), Qt::KeepAspectRatio);
        ui->label_output->setPixmap(QPixmap::fromImage(img));
    }
}

//  通过英文文本生成壁纸
void MainWindow::on_pushButton_byen_clicked()
{
    //  检查输入文字
    QString in_text = ui->textEdit_en->toPlainText();
    if(in_text.isEmpty())
    {
        //  提示
        ui->progressBar->setFormat("Input English Text is NULL!");
        return;
    }

    //  执行
    is_cur_busy = true;
    qtai.ExTextToWallpaper(in_text, ui->spinBox_maxres->value(), ui->lineEdit_cachefilename->text());

    //  设置状态
    ui->progressBar->setFormat("Generateing Video...");

    //  关闭控件
    ui->pushButton_bycn       ->setEnabled(false);
    ui->pushButton_byen       ->setEnabled(false);
    ui->pushButton_tslonly    ->setEnabled(false);
    ui->pushButton_open       ->setEnabled(false);
    ui->spinBox_maxres        ->setEnabled(false);
    ui->lineEdit_cachefilename->setEnabled(false);
}

//  通过中文文本生成壁纸
void MainWindow::on_pushButton_bycn_clicked()
{
    //  检查输入文字
    QString in_text = ui->textEdit_cn->toPlainText();
    if(in_text.isEmpty())
    {
        //  提示
        ui->progressBar->setFormat("Input Chinese Text is NULL!");
        return;
    }

    //  执行
    is_cur_busy = true;
    qtai.ExTranslateCn2En(in_text);

    //  设置状态
    ui->progressBar->setFormat("Translate Processing...");

    //  关闭控件
    ui->pushButton_bycn       ->setEnabled(false);
    ui->pushButton_byen       ->setEnabled(false);
    ui->pushButton_tslonly    ->setEnabled(false);
    ui->pushButton_open       ->setEnabled(false);
    ui->spinBox_maxres        ->setEnabled(false);
    ui->lineEdit_cachefilename->setEnabled(false);

    //  设置标志
    only_tsl_flag = false;
}

//  仅仅将中文翻译成英文
void MainWindow::on_pushButton_tslonly_clicked()
{
    //  检查输入文字
    QString in_text = ui->textEdit_cn->toPlainText();
    if(in_text.isEmpty())
    {
        //  提示
        ui->progressBar->setFormat("Input Chinese Text is NULL!");
        return;
    }

    //  执行
    is_cur_busy = true;
    qtai.ExTranslateCn2En(in_text);

    //  设置状态
    ui->progressBar->setFormat("Translate Processing...");

    //  关闭控件
    ui->pushButton_bycn       ->setEnabled(false);
    ui->pushButton_byen       ->setEnabled(false);
    ui->pushButton_tslonly    ->setEnabled(false);
    ui->pushButton_open       ->setEnabled(false);
    ui->spinBox_maxres        ->setEnabled(false);
    ui->lineEdit_cachefilename->setEnabled(false);

    //  设置标志
    only_tsl_flag = true;
}

//  打开壁纸文件
void MainWindow::on_pushButton_open_clicked()
{
    std::string cmd;
    cmd += "xdg-open ";
    cmd += ui->lineEdit_cachefilename->text().toStdString();
    system(cmd.c_str());
}
