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
            sources/dataoperator.cpp

HEADERS  += headers/mainwindow.h \
            headers/dataoperator.h

FORMS    += ui/mainwindow.ui

#QWT
INCLUDEPATH +=  C:\\Qt\\Qwt-6.0.1\\include
LIBS        +=  -LC:\\Qt\\Qwt-6.0.1\\lib \
                -lqwtd

#D2K-DASK
INCLUDEPATH +=  C:\\ADLINK\\D2K-DASK\\Include
LIBS        +=  -LC:\\ADLINK\\D2K-DASK\\Lib \
                -lD2K-Dask
