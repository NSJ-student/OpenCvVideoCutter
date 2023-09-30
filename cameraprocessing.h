#ifndef CAMERAPROCESSING_H
#define CAMERAPROCESSING_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QTime>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QRect>
#include <QGraphicsRectItem>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define ROI_COUNT   8
#define DEBUG_ROI   0

class CameraCaptureInput
{
public:
    CameraCaptureInput(int roi, bool cap, cv::Mat img)
    {
        inputTime = QDateTime::currentDateTime();
        roi_index = roi;
        capture = cap;
        cameraInput = img;
    }
    CameraCaptureInput(bool rec, cv::Mat img)
    {
        inputTime = QDateTime::currentDateTime();
        roi_index = -1;
        record = rec;
        capture = false;
        cameraInput = img;
    }
    ~CameraCaptureInput()
    {
        if(!cameraInput.empty())
        {
            cameraInput.release();
        }
    }

    int roi_index;
    bool capture;
    bool record;
    cv::Mat cameraInput;
    QDateTime inputTime;
};

class CameraRecordInfo
{
public:
    CameraRecordInfo(QString &path)
    {
        recordTime = QDateTime::currentDateTime();
        recordPath = path;
    }

    QString   recordPath;
    QDateTime recordTime;
};

class CameraProcessing : public QThread
{
    Q_OBJECT
public:
    CameraProcessing(QObject *parent = Q_NULLPTR);
    ~CameraProcessing();

    /**************************/
    //  Processing Start/Stop
    /**************************/

    void startProcessing();
    bool stopProcessing(quint32 wait_ms);

    /**************************/
    //  Capture&Record Start/Stop
    /**************************/

    bool setCaptureEn(bool enable);
    bool startRecord(QString path, cv::Size size, int fps);
    bool startRecord(cv::Size size, int fps);
    void stopRecord();

    void setCapturedFrame(int roi_index, bool do_capture, cv::Mat img);
    void setRecordFrame(bool do_record, cv::Mat img);
    void doCapture(int roi_index, cv::Mat img);
    void doCapture(CameraCaptureInput * obj);

    bool isRecording()
    {
        return (this->isRunning() && b_recordEnable);
    }

Q_SIGNALS:
    void recordTime(int time);
    void processQueueCount(int count);
    void captured(int roi_index, int time, const QString &filePath);
    void recorded(const QString &filePath);

private Q_SLOTS:
    void run();

private:
    QQueue<CameraCaptureInput *> m_frameQueue;
    QMutex m_frameMutex;
    bool b_cameraWorking;

    bool b_captureEnable;
    QTime capture_time;

    bool b_recordEnable;
    bool b_recordErrorDetected;
    cv::VideoWriter * m_videoWriter;
    QString m_recordPath;
    cv::Size m_recordSize;
    int      m_recordFps;
    QTime   m_recordingTime;
    QTime   m_recordTime;

    bool setCapturePath(QString &file_path);
    bool setRecordPath(QString &file_path);
};

#endif // CAMERAPROCESSING_H
