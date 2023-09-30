#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

#include <QObject>
#include <QSize>
#include <QTime>
#include <QThread>
#include <QQueue>
#include <QMutex>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define CAPTURE_FREQ_HZ       100

class OpenCvVideo : public QThread
{
    Q_OBJECT
public:
    OpenCvVideo(QObject *parent = Q_NULLPTR);
    ~OpenCvVideo();

    bool isPaused()
    {
        if(isRunning() && b_videoPause)
        {
            return true;
        }

        return false;
    }

    bool setVideo(const QString &path);
    bool startVideo();
    bool startVideo(const QString &path);
    void pauseVideo()
    {
        b_videoPause = true;
    }
    void resumeVideo()
    {
        b_videoPause = false;
    }
    bool stopVideo(quint32 wait_ms);

    cv::Mat getCapturedFrame();
    QSize getCurrentSize();
    int getVideoIntervalMs();
    int getVideoFrameCount();
    double getVideoFps();
    int getVideoDuration();
    int getCurrentVideoCount();
    int getCurrentVideoTime();

    void setCurrentVideoTime(int current_sec);

Q_SIGNALS:
    void videoFinished();
    void drawMessage(const QString & message);

private Q_SLOTS:
    void run();

public slots:

private:
    cv::VideoCapture m_videoCapture;
    QQueue<cv::Mat> m_frameQueue;
    QMutex m_frameMutex;
    bool m_cameraAvailable;
    bool b_videoWorking;
    bool b_videoPause;
    int  m_currentFrameCount;
    int  m_totalFrameCount;
    double  m_currentFps;
};

#endif // CAMERACAPTURE_H
