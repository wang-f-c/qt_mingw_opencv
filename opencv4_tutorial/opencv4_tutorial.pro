QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    facedemo.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    facedemo.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH+=D:\opencv4.6\include
LIBS+=D:\opencv4.6\x64\mingw\lib\libopencv_world460.dll.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    model.qrc

DISTFILES += \
    Icons/testt.png

