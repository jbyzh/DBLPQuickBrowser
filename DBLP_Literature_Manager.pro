QT       += core gui charts concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    GraphManager.cpp \
    Tools.cpp \
    algorithm.cpp \
    analytics.cpp \
    analyticswindow.cpp \
    authorindexgraphcache.cpp \
    barchart.cpp \
    clique.cpp \
    cliqueanalyse.cpp \
    CliqueTools.cpp \
    collaborationgraph.cpp \
    cooperationnet.cpp \
    Data_initial.cpp \
    DegeneracyAlgorithm.cpp \
    DegeneracyTools.cpp \
    fuzzysearchdialog.cpp \
    intervalanalysis.cpp \
    intervalwindow.cpp \
    memorymanager.cpp \
    search.cpp \
    searchdialog.cpp \
    XmlParser.cpp \
    functionpage.cpp \
    main.cpp \
    mainwindow.cpp \
    precise.cpp

HEADERS += \
    Algorithm.h \
    CliqueTools.h \
    DegeneracyAlgorithm.h \
    DegeneracyTools.h \
    GraphManager.h \
    MemoryManager.h \
    Tools.h \
    analytics.h \
    analyticswindow.h \
    authorindexgraphcache.h \
    barchart.h \
    clique.h \
    cliqueanalyse.h \
    cliquetools.h \
    collaborationgraph.h \
    cooperationnet.h \
    Data_initial.h \
    fuzzysearchdialog.h \
    intervalanalysis.h \
    intervalwindow.h \
    search.h \
    searchdialog.h \
    Universal_headers.h \
    User_Define_Using.h \
    XmlParser.h \
    functionpage.h \
    mainwindow.h \
    precise.h

FORMS += \
    barchart.ui \
    clique.ui \
    functionpage.ui \
    mainwindow.ui \
    precise.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
