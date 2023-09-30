#include "cameraprocessing.h"

// https://deep-learning-study.tistory.com/108
// DIVX: mp4
// H264: mp4
// MJPG: avi
#if defined(Q_OS_LINUX)
#define CAPTURE_PATH                QDir::homePath() + "/FIRMWARE_TEST/"
#define RECORD_PATH                 QDir::homePath() + "/FIRMWARE_TEST/"
#define VIDEO_FORMAT                cv::VideoWriter::fourcc('H','2','6','4')
#define VIDEO_EXT                   ".mp4"
#else
#define CAPTURE_PATH                "D:/FIRMWARE_TEST/"
#define RECORD_PATH                 "D:/FIRMWARE_TEST/"
#define VIDEO_FORMAT                cv::VideoWriter::fourcc('M','J','P','G')
#define VIDEO_EXT                   ".avi"
#endif
#define VIDEO_CLEAN_INTERVAL_MS     60000
#define RECORD_AMOUNT_MS            30000

CameraProcessing::CameraProcessing(QObject *parent) :
    QThread(parent)
  , b_cameraWorking(false)
  , b_captureEnable(false)
{
    m_videoWriter = Q_NULLPTR;
}

CameraProcessing::~CameraProcessing()
{
    if(isRunning())
    {
        wait(1000);
    }

    if(m_videoWriter)
    {
        m_videoWriter->release();
        delete m_videoWriter;
        m_videoWriter = Q_NULLPTR;
    }

    while(m_frameQueue.count() > 0)
    {
        CameraCaptureInput * input = m_frameQueue.dequeue();
        delete input;
    }
}

/**************************/
//  Processing Start/Stop
/**************************/

void CameraProcessing::startProcessing()
{
    if(isRunning())
    {
        return;
    }

    while(m_frameQueue.count() > 0)
    {
        CameraCaptureInput * input = m_frameQueue.dequeue();
        delete input;
    }

    b_cameraWorking = true;
    this->start();
}

bool CameraProcessing::stopProcessing(quint32 wait_ms)
{
    b_cameraWorking = false;

    if(wait_ms > 0)
    {
        if(wait(wait_ms))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}

/**************************/
//  Capture&Record Start/Stop
/**************************/

bool CameraProcessing::startRecord(QString path, cv::Size size, int fps)
{
    if(b_recordEnable && m_videoWriter)
    {
        return true;
    }

    if(m_videoWriter)
    {
        m_videoWriter->release();
        delete m_videoWriter;
        m_videoWriter = Q_NULLPTR;
    }
    m_recordSize = size;
    m_recordFps = fps;
    b_recordErrorDetected = false;
#if defined(Q_OS_LINUX)
    QString gst_out = QString("appsrc ! videoconvert ! "
                        "x264enc ! "
                        "appsink location=\"%1\"").arg(path);

    m_videoWriter = new cv::VideoWriter(gst_out.toStdString(),
                                    cv::CAP_GSTREAMER,
                                    0,
                                    m_recordFps,
                                    m_recordSize);
#else
    m_videoWriter = new cv::VideoWriter(path.toStdString(),
                                    VIDEO_FORMAT,
                                    m_recordFps,
                                    m_recordSize);
#endif
    if(m_videoWriter->isOpened())
    {
        m_recordingTime = QTime::currentTime();
        b_recordEnable = true;
    }
    else
    {
        m_videoWriter->release();
        delete m_videoWriter;
        m_videoWriter = Q_NULLPTR;
    }
    return true;
}

bool CameraProcessing::startRecord(cv::Size size, int fps)
{
    if(!setRecordPath(m_recordPath))
    {
        return false;
    }
    startRecord(m_recordPath, size, fps);
}

void CameraProcessing::stopRecord()
{
    b_recordEnable = false;
}

/**************************/

void CameraProcessing::setCapturedFrame(int roi_index, bool do_capture, cv::Mat img)
{
    m_frameMutex.lock();
    m_frameQueue.enqueue(new CameraCaptureInput(roi_index, do_capture, img));
    m_frameMutex.unlock();
}

void CameraProcessing::setRecordFrame(bool do_record, cv::Mat img)
{
    m_frameMutex.lock();
    m_frameQueue.enqueue(new CameraCaptureInput(do_record, img));
    m_frameMutex.unlock();
}

void CameraProcessing::doCapture(int roi_index, cv::Mat img)
{
    QString capturePath;
    if(setCapturePath(capturePath))
    {
        capture_time.restart();
#if defined(Q_OS_WIN)
        std::string path = qPrintable(capturePath);
        cv::imwrite(path.data(), img);
#else
        cv::imwrite(capturePath.toStdString(), img);
#endif
        emit captured(roi_index, capture_time.elapsed(), capturePath);
    }
}

void CameraProcessing::doCapture(CameraCaptureInput * obj)
{
    if(obj==Q_NULLPTR)
    {
        return;
    }

#if defined(Q_OS_LINUX)
    QString capturePath = CAPTURE_PATH + QString("capture_%1.bmp")
            .arg(obj->inputTime.toString("hh:mm:ss.zzz"));
#else
    QString capturePath = CAPTURE_PATH + QString("capture_%1.bmp")
            .arg(obj->inputTime.toString("hhmmsszzz"));
#endif

    capture_time.restart();
#if defined(Q_OS_WIN)
    std::string path = qPrintable(capturePath);
    cv::imwrite(path.data(), obj->cameraInput);
#else
    cv::imwrite(capturePath.toStdString(), obj->cameraInput);
#endif
    emit captured(obj->roi_index, capture_time.elapsed(), capturePath);
}

/**************************/
//  Thread Work
/**************************/

void CameraProcessing::run()
{
    int queue_count = -1;
    while(b_cameraWorking)
    {
        CameraCaptureInput * input;
        bool isEmpty = true;

        m_frameMutex.lock();
        if(m_frameQueue.count() > 0)
        {
            input = m_frameQueue.dequeue();
            isEmpty = false;
        }
        m_frameMutex.unlock();

        if(isEmpty)
        {
//            qDebug() << "no frame data";
            if(!b_recordEnable)
            {
                if(m_videoWriter)
                {
                    m_videoWriter->release();
                    delete m_videoWriter;
                    m_videoWriter = Q_NULLPTR;
                    qDebug() << "release record";
                }
            }
            QThread::msleep(5);
            continue;
        }

        // capture detect
        if(b_captureEnable)
        {
            if(input->capture)
            {
                doCapture(input);
            }
        }
        else if(b_recordEnable)
        {
            if(m_videoWriter)
            {
                if(input->record || input->capture)
                {
                    b_recordErrorDetected = true;
                }
                m_recordTime.restart();
                m_videoWriter->write(input->cameraInput);
                emit recordTime(m_recordTime.elapsed());
            }
        }

        if(queue_count != m_frameQueue.count())
        {
            queue_count = m_frameQueue.count();
            emit processQueueCount(queue_count);
        }

        delete input;
    }

    if(m_videoWriter)
    {
        m_videoWriter->release();
        delete m_videoWriter;
        m_videoWriter = Q_NULLPTR;
    }
}

/**************************/
//  Private Function
/**************************/

bool CameraProcessing::setCapturePath(QString &file_path)
{
#if defined(Q_OS_LINUX)
    file_path = CAPTURE_PATH + QString("capture_%1.bmp")
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
#else
    file_path = CAPTURE_PATH + QString("capture_%1.bmp")
            .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
#endif
    return true;
}

bool CameraProcessing::setRecordPath(QString &file_path)
{
#if defined(Q_OS_LINUX)
    file_path = RECORD_PATH + QString("record_%1")
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz")) + QString(VIDEO_EXT);
#else
    file_path = RECORD_PATH + QString("record_%1")
            .arg(QTime::currentTime().toString("hh_mm_ss_zzz")) + QString(VIDEO_EXT);
#endif
    return true;
}
