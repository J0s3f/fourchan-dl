#-------------------------------------------------
#
# Project created by QtCreator 2011-02-06T19:46:34
#
#-------------------------------------------------

QT       += core gui network xml widgets

win32:TARGET = ../../fourchan-dl
else:TARGET = ../fourchan-dl

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    uiimageoverview.cpp \
    uiinfo.cpp \
    uiconfig.cpp \
    uilisteditor.cpp \
    applicationupdateinterface.cpp \
    ../updater/commands.cpp \
    blacklist.cpp \
    downloadmanager.cpp \
    requesthandler.cpp \
    supervisednetworkreply.cpp \
    downloadrequest.cpp \
    uithreadadder.cpp \
    dialogfoldershortcut.cpp \
    foldershortcuts.cpp \
    thumbnailview.cpp \
    networkaccessmanager.cpp \
    pluginmanager.cpp \
    ecwin7.cpp \
    uiimageviewer.cpp \
    thumbnailcreator.cpp \
    thumbnailremover.cpp

HEADERS  += mainwindow.h \
    uiimageoverview.h \
    uiinfo.h \
    uiconfig.h \
    uilisteditor.h \
    defines.h \
    applicationupdateinterface.h \
    ../updater/commands.h \
    blacklist.h \
    downloadmanager.h \
    requesthandler.h \
    supervisednetworkreply.h \
    downloadrequest.h \
    uithreadadder.h \
    dialogfoldershortcut.h \
    foldershortcuts.h \
    thumbnailview.h \
    networkaccessmanager.h \
    pluginmanager.h \
    ParserPluginInterface.h \
    structs.h \
    ecwin7.h \
    uiimageviewer.h \
    thumbnailcreator.h \
    thumbnailremover.h

FORMS    += mainwindow.ui \
    uiimageoverview.ui \
    uiinfo.ui \
    uiconfig.ui \
    uilisteditor.ui \
    uithreadadder.ui \
    dialogfoldershortcut.ui \
    uiimageviewer.ui

RESOURCES += \
    resources.qrc

include(QsLog.pri)

win32: RC_FILE = fourchan-dl-win.rc
os2: RC_FILE = fourchan-dl-os2.rc
macx: ICON = resources/fourchan-dl.icns

BUILDDATE = __DATE__
DEFINES += BUILDDATE=$${BUILDDATE} PROGRAM_VERSION="\\\"1.5.0\\\""

win32: LIBS += ole32.lib

OTHER_FILES += \
    readme.txt \
    settings.ini \
    fourchan-dl.rc
