QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

linux-g++ | linux-g++-64 | linux-g++-32 {

#sudo apt install libqt5svg5-dev libqwt-qt5-dev libqwt-headers libqwt-doc libqwt-qt5-6 libqwtmathml-qt5-6 libqwtmathml-qt5-dev libqwtplot3d-doc libqwtplot3d-qt5-0 libqwtplot3d-qt5-dev
CONFIG += qwt
QMAKE_LFLAGS += -no-pie
CONFIG += link_pkgconfig c++11

# sudo apt-get install gtk+-3.0-dev
#PKGCONFIG += gtk+-3.0
PKGCONFIG += Qt5GStreamer-1.0 Qt5GStreamerUi-1.0 Qt5GStreamerUtils-1.0 gstreamer-1.0 gstreamer-video-1.0
PKGCONFIG += libavcodec

LIBS += -L/usr/local/lib -lopencv_shape -lopencv_videoio -lopencv_imgcodecs
LIBS += -lopencv_highgui -lopencv_core -lopencv_imgproc -lopencv_features2d -lopencv_calib3d

INCLUDEPATH += /usr/local/include/opencv4
INCLUDEPATH += /usr/include/at-spi2-atk/2.0
INCLUDEPATH += /usr/include/at-spi-2.0
INCLUDEPATH += /usr/include/dbus-1.0
INCLUDEPATH += /usr/lib/aarch64-linux-gnu/dbus-1.0/include
INCLUDEPATH += /usr/include/gio-unix-2.0/
INCLUDEPATH += /usr/include/mirclient
INCLUDEPATH += /usr/include/mircore
INCLUDEPATH += /usr/include/mircookie
INCLUDEPATH += /usr/include/cairo
INCLUDEPATH += /usr/include/pango-1.0
INCLUDEPATH += /usr/include/harfbuzz
INCLUDEPATH += /usr/include/pango-1.0
INCLUDEPATH += /usr/include/atk-1.0
INCLUDEPATH += /usr/include/cairo
INCLUDEPATH += /usr/include/pixman-1
INCLUDEPATH += /usr/include/freetype2
INCLUDEPATH += /usr/include/gdk-pixbuf-2.0
INCLUDEPATH += /usr/include/libpng12
INCLUDEPATH += /usr/include/glib-2.0
INCLUDEPATH += /usr/lib/aarch64-linux-gnu/glib-2.0/include
INCLUDEPATH += /usr/include/SDL
INCLUDEPATH += /usr/include/libpng16
INCLUDEPATH += /usr/include/aarch64-linux-gnu
INCLUDEPATH += /usr/include/libusb-1.0
LIBS += -L/usr/local/lib
LIBS += -L/usr/lib
LIBS += -lpangocairo-1.0 -lpango-1.0 -latk-1.0
LIBS += -lcairo-gobject -lcairo -lgdk_pixbuf-2.0
LIBS += -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lSDL -lportaudio
LIBS += -lasound -lm -lpthread -lpng16
LIBS += -lv4l2 -ludev -lusb-1.0
}

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
