#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyle>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    b_slider_pressed = false;
    m_pixmapItem = Q_NULLPTR;
    ui->graphicsView->setScene(&m_scene);
    ui->btnClearScene->setIcon(this->style()->standardIcon(
                                   QStyle::SP_TrashIcon));

    m_videoCapture = new OpenCvVideo(this);
    connect(m_videoCapture, SIGNAL(videoFinished()),
            this, SIGNAL(videoEnd()));

    connect(&m_videoTimer, SIGNAL(timeout()),
            this, SLOT(videoTimer()));

    ui->frameVideoWork->setEnabled(false);

    QTimeEdit * time;
    time = ui->timeVideoStart;
    time->setTime(QTime(0,0,0));
    time->setTimeRange(QTime(0, 0, 0), QTime(24, 0, 0));
    time->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
    time->setDisplayFormat(" hh:mm:ss ");
    time->setButtonSymbols(QTimeEdit::NoButtons);
    time = ui->timeVideoEnd;
    time->setTime(QTime(0,0,0));
    time->setTimeRange(QTime(0, 0, 1), QTime(24, 0, 0));
    time->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
    time->setDisplayFormat(" hh:mm:ss ");
    time->setButtonSymbols(QTimeEdit::NoButtons);

    m_videoProsessing = new CameraProcessing(this);
    m_videoProsessing->startProcessing();
}

MainWindow::~MainWindow()
{
    m_videoCapture->stopVideo(1000);
    m_videoTimer.stop();
    m_videoProsessing->stopProcessing(1000);
    delete ui;
}

void MainWindow::videoTimer()
{
    if(m_videoCapture->isPaused())
    {
        return;
    }

    cv::Mat output = m_videoCapture->getCapturedFrame();
    if(output.empty())
    {
        if(!m_videoCapture->isRunning())
        {
            m_videoTimer.stop();
        }
        return;
    }

    if(ui->cbShowVideo->isChecked())
    {
        QSize size = this->size();
        QPixmap pixmap;

        QImage qimg = QImage(output.data,
                    output.cols, output.rows, output.step,
    #if defined(Q_OS_LINUX)
                    QImage::Format_RGB888).rgbSwapped();
    #else
                    QImage::Format_BGR888);
    #endif
        pixmap = QPixmap::fromImage(qimg);

        //////////////////////////////
        /// output camera frame
        //////////////////////////////
        if(m_pixmapItem == Q_NULLPTR)
        {
            m_pixmapItem = m_scene.addPixmap(pixmap);
            m_pixmapItem->setZValue(0);
            m_scene.setSceneRect(pixmap.rect());
        }
        else
        {
            m_pixmapItem->setPixmap(pixmap);
        }
    }

    int frame_count = m_videoCapture->getCurrentVideoCount();
    int total_frame_count = m_videoCapture->getVideoFrameCount();

    int duration_msec = m_videoCapture->getCurrentVideoTime();
    int hour = (duration_msec/1000/60/60)%24;
    int minute = (duration_msec/1000/60)%60;
    int second = (duration_msec/1000)%60;

    if(!b_slider_pressed)
    {
        ui->sliderVideoInput->setValue(duration_msec);
    }
    ui->lblVideoTime->setText(QString("%1:%2:%3")
                       .arg(hour).arg(minute).arg(second));


    if(!ui->btnRecord->isChecked())
    {
        if(total_frame_count <= frame_count)
        {
            on_btnVideoStop_clicked();
            return;
        }
        return;
    }

    ui->progRecord->setValue(duration_msec-record_start_msec);
    if((record_start_msec<=duration_msec) &&
            (duration_msec<=record_end_msec))
    {
        qDebug() << "recording" << duration_msec << record_end_msec;
        m_videoProsessing->setRecordFrame(true, output);
    }
    else if(duration_msec>record_end_msec)
    {
        if(m_videoProsessing->isRecording())
        {
            on_btnRecord_clicked(false);
        }
    }

    if(total_frame_count <= frame_count)
    {
        on_btnRecord_clicked(false);
    }
}

void MainWindow::videoEnd()
{
    on_btnVideoStop_clicked();
}


void MainWindow::on_btnSelectVideoSource_clicked()
{
    QString path =
             QFileDialog::getOpenFileName(
                this, ("Open Video File"),
                Q_NULLPTR,
                "All Files (*.*)");

    if(!path.isNull() && !path.isEmpty())
    {
        ui->lineVideoSourcePath->setText(path);
        ui->frameVideoWork->setEnabled(true);
        if(m_videoCapture->setVideo(path))
        {
            int duration = m_videoCapture->getVideoDuration();
            int hour = (duration/60/60)%24;
            int minute = (duration/60)%60;
            int second = (duration)%60;

            ui->sliderVideoInput->setMaximum(duration*1000);
            ui->lblVideoTime->setText("00:00:00");
            ui->lblVideoTotal->setText(QString("%1:%2:%3")
                               .arg(hour).arg(minute).arg(second));

            QTimeEdit * time;
            time = ui->timeVideoStart;
            time->setTime(QTime(0,0,0));
            time->setTimeRange(QTime(0, 0, 0), QTime(hour, minute, second));
            time->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
            time->setDisplayFormat(" hh:mm:ss ");
            time->setButtonSymbols(QTimeEdit::NoButtons);
            time = ui->timeVideoEnd;
            time->setTime(QTime(hour,minute,second));
            time->setTimeRange(QTime(0, 0, 1), QTime(hour, minute, second));
            time->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
            time->setDisplayFormat(" hh:mm:ss ");
            time->setButtonSymbols(QTimeEdit::NoButtons);
        }
    }
}


void MainWindow::on_btnSaveVideoPath_clicked()
{
    QString path =
             QFileDialog::getSaveFileName(
                this, ("Save Video File"),
                Q_NULLPTR,
                "All Files (*.*)");

    if(!path.isNull() && !path.isEmpty())
    {
        ui->lineVideoOutPath->setText(path);
    }
}


void MainWindow::on_sliderVideoInput_sliderPressed()
{
    b_slider_pressed = true;
}

void MainWindow::on_sliderVideoInput_sliderReleased()
{
    m_videoCapture->forceCurrentVideoTime(ui->sliderVideoInput->value());
    b_slider_pressed = false;
}


void MainWindow::on_btnVideoPlay_clicked()
{
    QString path = ui->lineVideoSourcePath->text();
    if(path.isNull() || path.isEmpty())
    {
        return;
    }

    m_videoCapture->setCurrentVideoTime(0);
    if(m_videoCapture->startVideo())
    {
        ui->btnVideoPlay->setEnabled(false);
        ui->tabControl->setTabEnabled(1, false);
        int interval = m_videoCapture->getVideoIntervalMs();
        if(interval > 0)
        {
#if (DEBUG_PRINT==1)
            qDebug() << "start" << interval << path;
#endif
            m_videoTimer.setSingleShot(false);
            m_videoTimer.setInterval(interval);
            QTimer::singleShot(interval, &m_videoTimer, SLOT(start()));
        }
    }
}


void MainWindow::on_btnVideoStop_clicked()
{
    ui->btnVideoPlay->setEnabled(true);
    ui->tabControl->setTabEnabled(1, true);
    m_videoTimer.stop();
    m_videoCapture->stopVideo(1000);
    m_videoProsessing->stopRecord();
}


void MainWindow::on_btnRecord_clicked(bool checked)
{
    if(ui->lineVideoOutPath->text().isEmpty())
    {
        ui->btnRecord->setChecked(false);
        return;
    }

    ui->btnRecord->setChecked(checked);
    ui->timeVideoStart->setEnabled(!checked);
    ui->timeVideoEnd->setEnabled(!checked);
    ui->btnSaveVideoPath->setEnabled(!checked);
    ui->tabControl->setTabEnabled(0, !checked);

    if(checked)
    {
        record_start_msec = ui->timeVideoStart->time().msecsSinceStartOfDay();
        record_end_msec = ui->timeVideoEnd->time().msecsSinceStartOfDay();
        ui->progRecord->setMaximum(record_end_msec-record_start_msec);
        ui->progRecord->setValue(0);

        QSize size = m_videoCapture->getCurrentSize();
        bool result = m_videoProsessing->startRecord(
                    ui->lineVideoOutPath->text(),
                    cv::Size(size.width(), size.height()),
                    m_videoCapture->getVideoFps());

        if(!result)
        {
            ui->btnRecord->setChecked(false);
            qDebug() << "fail to start record";
            return;
        }

        m_videoCapture->setCurrentVideoTime(record_start_msec);
        if(m_videoCapture->startVideo())
        {
            ui->btnVideoPlay->setEnabled(false);
            int interval = m_videoCapture->getVideoIntervalMs();
            if(interval > 0)
            {
#if (DEBUG_PRINT==1)
                qDebug() << "start" << interval << path;
#endif
                m_videoTimer.setSingleShot(false);
                m_videoTimer.setInterval(interval);
                QTimer::singleShot(interval, &m_videoTimer, SLOT(start()));
            }
        }
    }
    else
    {
        m_videoTimer.stop();
        m_videoCapture->stopVideo(1000);
        m_videoProsessing->stopRecord();
    }
}


void MainWindow::on_btnClearScene_clicked()
{
    if(m_pixmapItem)
    {
        delete m_pixmapItem;
        m_pixmapItem = Q_NULLPTR;
    }
    m_scene.setSceneRect(QRectF());
}


