#include "opencvvideo.h"

OpenCvVideo::OpenCvVideo(QObject *parent) :
    QThread(parent)
  , b_videoWorking(false)
  , b_videoPause(false)
  , m_currentFrameCount(0)
{

}

OpenCvVideo::~OpenCvVideo()
{
    if(m_videoCapture.isOpened())
    {
        m_videoCapture.release();
    }
    cv::destroyAllWindows();
}

bool OpenCvVideo::setVideo(const QString &path)
{
    if(this->isRunning())
    {
        b_videoWorking = false;
        if(!wait(1000))
        {
            qDebug("prev video close fail");
            return false;
        }
    }

    if(m_videoCapture.isOpened())
    {
        m_videoCapture.release();
    }

    m_videoCapture = cv::VideoCapture(path.toStdString());
    if(!m_videoCapture.isOpened())
    {
        qDebug("video open fail");
        return false;
    }

    m_currentVideoPath = path;
    return true;
}

bool OpenCvVideo::startVideo()
{
    if(this->isRunning())
    {
        b_videoWorking = false;
        if(!wait(1000))
        {
            emit drawMessage("prev video close fail");
            return false;
        }
    }

    m_frameMutex.lock();
    if(m_frameQueue.count() > 0)
    {
        m_frameQueue.clear();
        m_currentFrameCount = 0;
    }
    m_frameMutex.unlock();

    b_videoPause = false;
    b_videoWorking = true;
    m_currentFrameCount = m_videoCapture.get(cv::CAP_PROP_POS_FRAMES);
    this->start();
    return true;
}

bool OpenCvVideo::startVideo(const QString &path)
{
    if(this->isRunning())
    {
        b_videoWorking = false;
        if(!wait(1000))
        {
            emit drawMessage("prev video close fail");
            return false;
        }
    }

    m_frameMutex.lock();
    if(m_frameQueue.count() > 0)
    {
        m_frameQueue.clear();
        m_currentFrameCount = 0;
    }
    m_frameMutex.unlock();

    if(m_videoCapture.isOpened())
    {
        m_videoCapture.release();
    }

    m_videoCapture = cv::VideoCapture(path.toStdString());
    if(!m_videoCapture.isOpened())
    {
        qDebug("video open fail");
        return false;
    }

    m_currentVideoPath = path;
    b_videoPause = false;
    b_videoWorking = true;
    m_currentFrameCount = m_videoCapture.get(cv::CAP_PROP_POS_FRAMES);
    this->start();
    return true;
}

bool OpenCvVideo::stopVideo(quint32 wait_ms)
{
    b_videoWorking = false;

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

cv::Mat OpenCvVideo::getCapturedFrame()
{
    cv::Mat output;

    m_frameMutex.lock();
    if(m_frameQueue.count() > 0)
    {
        output = m_frameQueue.dequeue();
        m_currentFrameCount++;
    }
    m_frameMutex.unlock();

    return output;
}

QSize OpenCvVideo::getCurrentSize()
{
    int curr_width, curr_height;
    curr_width = m_videoCapture.get(cv::CAP_PROP_FRAME_WIDTH);
    curr_height = m_videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);

    QSize size(curr_width, curr_height);
    return size;
}

int OpenCvVideo::getVideoIntervalMs()
{
    if(!m_videoCapture.isOpened())
    {
        return cvRound(1000 / m_currentFps);
    }
    else
    {
        m_currentFps = m_videoCapture.get(cv::CAP_PROP_FPS);
        return cvRound(1000 / m_currentFps);
    }
}

int OpenCvVideo::getVideoFrameCount()
{
    if(!m_videoCapture.isOpened())
    {
        return m_totalFrameCount;
    }
    else
    {
        m_totalFrameCount = m_videoCapture.get(cv::CAP_PROP_FRAME_COUNT);
        return m_totalFrameCount;
    }
}

double OpenCvVideo::getVideoFps()
{
    if(!m_videoCapture.isOpened())
    {
        return m_currentFps;
    }
    else
    {
        m_currentFps = m_videoCapture.get(cv::CAP_PROP_FPS);
        return m_currentFps;
    }
}

int OpenCvVideo::getVideoDuration()
{
    if(!m_videoCapture.isOpened())
    {
        return ((double)m_totalFrameCount*1000/m_currentFps);
    }
    else
    {
        m_currentFps = m_videoCapture.get(cv::CAP_PROP_FPS);
        m_totalFrameCount = m_videoCapture.get(cv::CAP_PROP_FRAME_COUNT);
        return ((double)m_totalFrameCount*1000/m_currentFps);
    }
}

int OpenCvVideo::getCurrentVideoCount()
{
    return m_currentFrameCount;
}

int OpenCvVideo::getCurrentVideoTime()
{
    return ((double)m_currentFrameCount*1000/m_currentFps);
}

void OpenCvVideo::setCurrentVideoTime(int current_msec)
{
    if(!m_videoCapture.isOpened())
    {
        return;
    }
    else
    {
        m_videoCapture.set(cv::CAP_PROP_POS_MSEC, current_msec);
        m_currentFrameCount = m_videoCapture.get(cv::CAP_PROP_POS_FRAMES);
    }
}

void OpenCvVideo::forceCurrentVideoTime(int target_msec)
{
    if(!m_videoCapture.isOpened())
    {
        return;
    }

    if(this->isRunning())
    {
        b_videoPause = true;
    }
    m_frameMutex.lock();
    if(m_frameQueue.count() > 0)
    {
        m_frameQueue.clear();
        m_currentFrameCount = 0;
    }
    m_frameMutex.unlock();

    m_videoCapture.set(cv::CAP_PROP_POS_MSEC, target_msec);
    m_currentFrameCount = m_videoCapture.get(cv::CAP_PROP_POS_FRAMES);
    b_videoPause = false;

    if(!this->isRunning())
    {
        b_videoWorking = true;
        this->start();
        qDebug() << "restart" << target_msec;
    }
    else
    {
        qDebug() << "set" << target_msec;
    }
}

void OpenCvVideo::run()
{
    if(!m_videoCapture.isOpened())
    {
        b_videoWorking = false;
        emit videoFinished();
        emit drawMessage("video open fail");
        return;
    }

    QTime debug;
    while(b_videoWorking)
    {
        if(b_videoPause)
        {
            QThread::msleep(10);
            continue;
        }
        if(m_frameQueue.count() > 100)
        {
            QThread::msleep(10);
            continue;
        }

        debug.restart();
        bool result = m_videoCapture.grab();
        if(!result)
        {
            b_videoWorking = false;
            qDebug() << "read done";
            break;
        }

        cv::Mat frame;
        result = m_videoCapture.retrieve(frame);
        if(!result)
        {
            b_videoWorking = false;
            qDebug() << "read done 2";
            break;
        }
        if(frame.empty())
        {
            qDebug() << "empty";
            continue;
        }

        m_frameMutex.lock();
        m_frameQueue.enqueue(frame);
        m_frameMutex.unlock();

        if(debug.elapsed() < 5)
        {
            QThread::msleep(5);
        }
    }

//    m_videoCapture.release();
}
