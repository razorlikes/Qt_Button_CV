#-------------------------------------------------
#
# Project created by QtCreator 2016-11-23T14:11:51
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia
QT       += multimediawidgets
QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CameraTest
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videosurface.cpp \
    calibration.cpp \
    worker.cpp

HEADERS  += mainwindow.h \
    videosurface.h \
    calibration.h \
    worker.h
