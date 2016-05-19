#-------------------------------------------------
#
# Project created by QtCreator 2016-04-28T16:18:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = signal visualizer
TEMPLATE = app


SOURCES +=  sources/main.cpp\
            sources/mainwindow.cpp \
            sources/dataoperator.cpp \
            sources/parametersdialog.cpp \
            sources/graphicplot.cpp \
            sources/timerslider.cpp \
            sources/plotgrid.cpp \
            sources/plotlegend.cpp \
            sources/plotmagnifier.cpp \
            sources/scaledraw.cpp \
            sources/singleshotdialog.cpp \
            sources/blockdialog.cpp

HEADERS  += headers/mainwindow.h \
            headers/dataoperator.h \
            headers/parametersdialog.h \
            headers/defines.h \
            headers/graphicplot.h \
            headers/timerslider.h \
            headers/plotgrid.h \
            headers/plotgridplotlegend.h \
            headers/plotgridplotmagnifier.h \
            headers/scaledraw.h \
            headers/singleshotdialog.h \
            headers/blockdialog.h

FORMS    += ui/mainwindow.ui \
            ui/parametersdialog.ui \
            ui/singleshotdialog.ui \
            ui/blockdialog.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS += daq_signal_visualizer_ru.ts

#QWT
INCLUDEPATH +=  C:\\Qt\\Qwt-6.0.1\\include
LIBS        +=  -LC:\\Qt\\Qwt-6.0.1\\lib \
                -lqwtd

#D2K-DASK
INCLUDEPATH +=  C:\\ADLINK\\D2K-DASK\\Include
LIBS        +=  -LC:\\ADLINK\\D2K-DASK\\Lib \
                -lD2K-Dask

