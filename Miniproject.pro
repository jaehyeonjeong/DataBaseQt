QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chetting.cpp \
    clientmanager.cpp \
    clientthread.cpp \
    logthread.cpp \
    main.cpp \
    mainwindow.cpp \
    productmanager.cpp \
    shoppingmanager.cpp \
    tcpclient.cpp \
    tcpserver.cpp

HEADERS += \
    chetting.h \
    clientmanager.h \
    clientthread.h \
    logthread.h \
    mainwindow.h \
    productmanager.h \
    shoppingmanager.h \
    tcpclient.h \
    tcpserver.h

FORMS += \
    chetting.ui \
    clientmanager.ui \
    mainwindow.ui \
    productmanager.ui \
    shoppingmanager.ui \
    tcplog.ui

TRANSLATIONS += \
    Miniproject_ko_KR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_CXXFLAGS += /utf-8
