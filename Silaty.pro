QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia xml

CONFIG += c++17
CONFIG += resources_big

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

ICON = silaty.icns
RC_ICONS = silaty.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    calc_prayer.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    calc_prayer.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
