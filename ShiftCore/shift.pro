# -------------------------------------------------
# Project created by QtCreator 2010-06-09T09:04:56
# -------------------------------------------------

TARGET = shift
TEMPLATE = lib

QT += widgets

include("../../Eks/EksCore/GeneralOptions.pri")

INCLUDEPATH += $$ROOT/Shift/ShiftCore/include \
    $$ROOT/Shift/ShiftCore/src \
    $$ROOT/Eks/EksCore \
    $$ROOT/Eks/EksGui \
    $$ROOT/Eks/EksScript

LIBS += -lEksCore \
    -lEksGui \
    -lEksScript

SOURCES += \
    src/sentity.cpp \
    src/sdatabase.cpp \
    src/Properties/sproperty.cpp \
    src/Properties/spropertycontainer.cpp \
    src/Properties/spropertyarray.cpp \
    src/Properties/sbaseproperties.cpp \
    src/Properties/sbasepointerproperties.cpp \
    src/Properties/sarrayproperty.cpp \
    src/Properties/sexternalpointer.cpp \
    src/Changes/sobserver.cpp \
    src/Changes/shandler.cpp \
    src/UI/sentityui.cpp \
    src/UI/sdatabasemodel.cpp \
    src/Utilities/sprocessmanager.cpp \
    src/Utilities/sresourcedescription.cpp \
    src/TypeInformation/spropertyinformation.cpp \
    src/TypeInformation/styperegistry.cpp \
    src/TypeInformation/sinterfaces.cpp \
    src/TypeInformation/spropertygroup.cpp \
    src/TypeInformation/spropertyinstanceinformation.cpp \
    src/Serialisation/JsonParser/JSON_parser.c \
    src/Serialisation/sxmlio.cpp \
    src/Serialisation/sbinaryio.cpp \
    src/Serialisation/sjsonio.cpp \
    src/Serialisation/sloader.cpp \
    src/Utilities/spropertyname.cpp \
    src/UI/moc_spropertydefaultui.cpp \
    src/UI/moc_sdatabasemodel.cpp \
    src/TypeInformation/spropertytraits.cpp

HEADERS += \
    include/shift/sglobal.h \
    include/shift/sentity.h \
    include/shift/sdatabase.h \
    include/shift/Properties/sproperty.h \
    include/shift/Properties/spropertycontainer.h \
    include/shift/Properties/spropertyarray.h \
    include/shift/Properties/sbaseproperties.h \
    include/shift/Properties/sarrayproperty.h \
    include/shift/Properties/sbasepointerproperties.h \
    include/shift/Properties/spropertycontaineriterators.h \
    include/shift/Properties/spropertymacros.h \
    include/shift/Properties/sproperty.inl \
    include/shift/Properties/sexternalpointer.h \
    include/shift/Changes/schange.h \
    include/shift/Changes/sobserver.h \
    include/shift/Changes/shandler.h \
    include/shift/Changes/shandler.inl \
    include/shift/Changes/spropertychanges.h \
    include/shift/UI/sentityui.h \
    include/shift/Serialisation/sloader.h \
    src/Serialisation/JsonParser/JSON_parser.h \
    include/shift/Serialisation/sxmlio.h \
    include/shift/Serialisation/sbinaryio.h \
    include/shift/Serialisation/sjsonio.h \
    include/shift/TypeInformation/styperegistry.h \
    include/shift/TypeInformation/sinterface.h \
    include/shift/TypeInformation/spropertyinformation.h \
    include/shift/TypeInformation/spropertyinstanceinformation.h \
    include/shift/TypeInformation/sinterfaces.h \
    include/shift/TypeInformation/spropertyinformationhelpers.h \
    include/shift/TypeInformation/spropertygroup.h \
    include/shift/Utilities/sentityweakpointer.h \
    include/shift/Utilities/sprocessmanager.h \
    include/shift/Utilities/siterator.h \
    include/shift/Utilities/sresourcedescription.h \
    include/shift/Utilities/spropertyname.h \
    include/shift/TypeInformation/spropertytraits.h

OTHER_FILES += \
    include/shift/UI/spropertydefaultui.h \
    include/shift/UI/sdatabasemodel.h \




