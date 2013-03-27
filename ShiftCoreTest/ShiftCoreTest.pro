#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T09:02:34
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

include("../../Eks/EksCore/GeneralOptions.pri")

TARGET = computetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += computetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += $$ROOT/shift/GraphicsCore \
    $$ROOT/Eks/EksCore \
    $$ROOT/Eks/Eks3D/include \
    $$ROOT/Eks/EksScript \
    $$ROOT/Shift/ShiftCore/include \
    $$ROOT/shift/MeshCore

LIBS += -lShiftCore -lEksCore -lEks3D -lEksScript -lShiftGraphicsCore
