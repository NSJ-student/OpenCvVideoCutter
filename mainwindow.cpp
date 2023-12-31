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

    QHeaderView* header;
    header = ui->treeMediaList->header();
    header->setSectionResizeMode(0,QHeaderView::Stretch);
    header->setSectionResizeMode(1,QHeaderView::ResizeToContents);

    m_videoCapture = new OpenCvVideo(this);
    connect(m_videoCapture, SIGNAL(videoFinished()),
            this, SLOT(videoEnd()));

    connect(&m_videoTimer, SIGNAL(timeout()),
            this, SLOT(videoTimer()));
    connect(&m_recordTimer, SIGNAL(timeout()),
            this, SLOT(recordTimer()));

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
    connect(m_videoProsessing, SIGNAL(recordTime(int)),
            this, SLOT(recordTime(int)));
    m_videoProsessing->startProcessing();
}

MainWindow::~MainWindow()
{
    m_videoCapture->stopVideo(1000);
    m_videoTimer.stop();
    m_recordTimer.stop();
    m_videoProsessing->stopProcessing(1000);
    delete ui;
}

void MainWindow::videoTimer()
{
    if(m_videoCapture->isPaused())
    {
        return;
    }

    QTime debug;
    debug.start();
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


    ui->lblProcessTimeMs->setText(QString::number(debug.elapsed()));
    if(total_frame_count <= frame_count)
    {
        on_btnVideoStop_clicked();
        return;
    }
    return;
}

void MainWindow::recordTimer()
{
    QTime debug;
    debug.start();
    cv::Mat output = m_videoCapture->getCapturedFrame();
    if(output.empty())
    {
        if(!m_videoCapture->isRunning())
        {
            m_recordTimer.stop();
        }
        return;
    }

    int frame_count = m_videoCapture->getCurrentVideoCount();
    int total_frame_count = m_videoCapture->getVideoFrameCount();

    int duration_msec = m_videoCapture->getCurrentVideoTime();
    int hour = (duration_msec/1000/60/60)%24;
    int minute = (duration_msec/1000/60)%60;
    int second = (duration_msec/1000)%60;

    ui->progRecord->setValue(duration_msec-record_start_msec);
    if((record_start_msec<=duration_msec) &&
            (duration_msec<=record_end_msec))
    {
#if (DEBUG_PRINT==1)
        qDebug() << "recording" << duration_msec << record_end_msec;
#endif
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

    ui->lblProcessTimeMs->setText(QString::number(debug.elapsed()));
}

void MainWindow::videoEnd()
{
    on_btnVideoStop_clicked();
}

void MainWindow::recordTime(int time)
{
    ui->lblRecordTimeMs->setText(QString::number(time));
}

void MainWindow::on_btnSelectVideoSource_clicked()
{
    QStringList path_list =
             QFileDialog::getOpenFileNames(
                this, ("Open Video Files"),
                Q_NULLPTR,
                "Video Files (*.avi *.mp4 *.mkv);; All Files (*.*)");

    if(path_list.count() > 0)
    {
        foreach(QString path, path_list)
        {
            QFileInfo info(path);

            int duration = OpenCvVideo::getVideoPlayTime(path);
            if(duration == 0)
            {
                continue;
            }
            int hour = (duration/1000/60/60)%24;
            int minute = (duration/1000/60)%60;
            int second = (duration/1000)%60;

            QString str_duration = QString("%1:%2:%3").arg(hour).arg(minute).arg(second);

            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeMediaList);
            item->setText(0, info.fileName());
            item->setText(1, str_duration);
            item->setToolTip(0, path);
            ui->treeMediaList->addTopLevelItem(item);
        }
    }
}

void MainWindow::on_btnSelectVideoRemove_clicked()
{
    QList<QTreeWidgetItem *> selected = ui->treeMediaList->selectedItems();
    foreach(QTreeWidgetItem * item, selected)
    {
        int index = ui->treeMediaList->indexOfTopLevelItem(item);
        if(index >= 0)
        {
            delete ui->treeMediaList->takeTopLevelItem(index);
        }
    }
}

void MainWindow::on_treeMediaList_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(m_videoCapture->isRunning())
    {
        on_btnVideoStop_clicked();
    }

    QString path = item->toolTip(0);
    if(path.isNull() || path.isEmpty())
    {
        return;
    }

    ui->lineVideoSourcePath->setText(path);
    ui->frameVideoWork->setEnabled(true);
    if(m_videoCapture->setVideo(path))
    {
        int duration = m_videoCapture->getVideoDuration();
        int hour = (duration/1000/60/60)%24;
        int minute = (duration/1000/60)%60;
        int second = (duration/1000)%60;

        ui->sliderVideoInput->setMaximum(duration);
        ui->sliderVideoInput->setValue(0);
        ui->lblVideoTime->setText("00:00:00");
        ui->lblVideoTotal->setText(QString("%1:%2:%3")
                           .arg(hour).arg(minute).arg(second));

        QTimeEdit * time;
        time = ui->timeVideoStart;
        time->setTimeRange(QTime(0, 0, 0), QTime(hour, minute, second));
        time->setTime(QTime(0,0,0));
        time->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
        time->setDisplayFormat(" hh:mm:ss ");
        time->setButtonSymbols(QTimeEdit::NoButtons);
        time = ui->timeVideoEnd;
        time->setTimeRange(QTime(0, 0, 1), QTime(hour, minute, second));
        time->setTime(QTime(hour,minute,second));
        time->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
        time->setDisplayFormat(" hh:mm:ss ");
        time->setButtonSymbols(QTimeEdit::NoButtons);

        on_btnVideoPlay_clicked();
    }
}

void MainWindow::on_btnSaveVideoPath_clicked()
{
    QString path =
             QFileDialog::getSaveFileName(
                this, ("Save Video File"),
                Q_NULLPTR,
#if defined(Q_OS_LINUX)
                "MP4 (*.mp4)");
#else
                "AVI (*.avi)");
#endif

    if(!path.isNull() && !path.isEmpty())
    {
        ui->lineVideoOutPath->setText(path);
    }
}

void MainWindow::on_sliderVideoInput_sliderPressed()
{
    b_slider_pressed = true;
    m_videoCapture->pauseVideo();
}

void MainWindow::on_sliderVideoInput_sliderReleased()
{
    m_videoCapture->forceCurrentVideoTime(ui->sliderVideoInput->value());
    b_slider_pressed = false;
    m_videoCapture->resumeVideo();
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
    ui->sliderVideoInput->setValue(0);
    on_btnClearScene_clicked();
}

void MainWindow::on_btnRecord_clicked(bool checked)
{
    if(ui->lineVideoOutPath->text().isEmpty())
    {
        ui->btnRecord->setChecked(false);
        return;
    }

    if(checked)
    {
        record_start_msec = ui->timeVideoStart->time().msecsSinceStartOfDay();
        record_end_msec = ui->timeVideoEnd->time().msecsSinceStartOfDay();
        if((record_end_msec-record_start_msec) <= 0)
        {
            ui->btnRecord->setChecked(false);
            QMessageBox::critical(this, "Error", "Invalid time range");
            return;
        }
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
#if (DEBUG_PRINT==1)
            qDebug() << "start" << interval << path;
#endif
            m_recordTimer.setSingleShot(false);
            m_recordTimer.setInterval(10);
            QTimer::singleShot(10, &m_recordTimer, SLOT(start()));
        }
    }
    else
    {
        ui->progRecord->setValue(ui->progRecord->maximum());
        m_recordTimer.stop();
        m_videoCapture->stopVideo(1000);
        m_videoProsessing->stopRecord();
    }

    ui->btnRecord->setChecked(checked);
    ui->timeVideoStart->setEnabled(!checked);
    ui->timeVideoEnd->setEnabled(!checked);
    ui->btnSaveVideoPath->setEnabled(!checked);
    ui->tabControl->setTabEnabled(0, !checked);
    ui->dockWidget->setEnabled(!checked);
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



void MainWindow::on_timeVideoStart_timeChanged(const QTime &time)
{
    ui->timeVideoEnd->setMinimumTime(time);
}


void MainWindow::on_timeVideoEnd_timeChanged(const QTime &time)
{
    ui->timeVideoStart->setMaximumTime(time);
}

