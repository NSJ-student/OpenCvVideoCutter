#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QTimer>
#include "opencvvideo.h"
#include "cameraprocessing.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void videoTimer();
    void videoEnd();

private slots:
    void on_btnSelectVideoSource_clicked();

    void on_btnSaveVideoPath_clicked();

    void on_sliderVideoInput_sliderMoved(int position);

    void on_btnVideoPlay_clicked();

    void on_btnVideoStop_clicked();

    void on_btnRecord_clicked(bool checked);

    void on_gbVideoPlay_clicked(bool checked);

    void on_gbVideoRecord_clicked(bool checked);

    void on_btnClearScene_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene m_scene;
    QGraphicsPixmapItem * m_pixmapItem;
    OpenCvVideo * m_videoCapture;
    QTimer m_videoTimer;
    CameraProcessing * m_videoProsessing;
    int record_start_msec;
    int record_end_msec;
};
#endif // MAINWINDOW_H
