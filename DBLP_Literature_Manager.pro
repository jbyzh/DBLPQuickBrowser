QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Data_initial.cpp \
    XmlParser.cpp \
    functionpage.cpp \
    main.cpp \
    mainwindow.cpp \
    precise.cpp

HEADERS += \
    Data_initial.h \
    Universal_headers.h \
    User_Define_Using.h \
    XmlParser.h \
    functionpage.h \
    mainwindow.h \
    precise.h

FORMS += \
    functionpage.ui \
    mainwindow.ui \
    precise.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
