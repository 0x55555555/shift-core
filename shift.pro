# -------------------------------------------------
# Project created by QtCreator 2010-06-09T09:04:56
# -------------------------------------------------

TARGET = shift
TEMPLATE = lib

include("../Eks/EksCore/GeneralOptions.pri")

INCLUDEPATH += $$ROOT/Shift \
    $$ROOT/Eks/EksCore \
    $$ROOT/Eks/EksGui \
    $$ROOT/Eks/EksScript

LIBS += -lEksCore \
    -lEksGui \
    -lEksScript

SOURCES += \
    sentity.cpp \
    sproperty.cpp \
    sdatabase.cpp \
    schange.cpp \
    sobserver.cpp \
    UI/sentityui.cpp \
    UI/spropertydefaultui.cpp \
    spropertycontainer.cpp \
    spropertyarray.cpp \
    sbaseproperties.cpp \
    spropertyinformation.cpp \
    sarrayproperty.cpp \
    sdatabasemodel.cpp \
    sprocessmanager.cpp \
    siterator.cpp \
    styperegistry.cpp \
    sbasepointerproperties.cpp \
    sinterface.cpp \
    Serialisation/JsonParser/JSON_parser.c \
    Serialisation/sxmlio.cpp \
    Serialisation/sbinaryio.cpp \
    Serialisation/sjsonio.cpp \
    sloader.cpp \
    shandler.cpp \
    sinterfaces.cpp \
    spropertygroup.cpp

HEADERS += \
    sglobal.h \
    sentity.h \
    sproperty.h \
    sdatabase.h \
    schange.h \
    sobserver.h \
    UI/sentityui.h \
    UI/spropertydefaultui.h \
    spropertycontainer.h \
    spropertyarray.h \
    sbaseproperties.h \
    spropertyinformation.h \
    sloader.h \
    sarrayproperty.h \
    sdatabasemodel.h \
    sentityweakpointer.h \
    sprocessmanager.h \
    siterator.h \
    styperegistry.h \
    sbasepointerproperties.h \
    sinterface.h \
    Serialisation/JsonParser/JSON_parser.h \
    Serialisation/sxmlio.h \
    Serialisation/sbinaryio.h \
    Serialisation/sjsonio.h \
    shandler.h \
    sinterfaces.h \
    spropertyinformationhelpers.h \
    spropertygroup.h





