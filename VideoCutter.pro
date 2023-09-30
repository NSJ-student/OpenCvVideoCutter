QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {

CONFIG += qwt
INCLUDEPATH += $$PWD/widnow_opencv_5.15.1/include

LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_core410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_highgui410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_imgcodecs410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_imgproc410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_features2d410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_calib3d410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_videoio410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\libopencv_video410.dll
LIBS += $$PWD\widnow_opencv_5.15.1\x64\mingw\bin\opencv_ffmpeg410_64.dll
}

SOURCES += \
    cameraprocessing.cpp \
    main.cpp \
    mainwindow.cpp \
    opencvvideo.cpp

HEADERS += \
    cameraprocessing.h \
    mainwindow.h \
    opencvvideo.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icon.qrc
