/****************************************************************************
** Meta object code from reading C++ file 'spropertydefaultui.h'
**
** Created: Tue 1. Jan 15:26:31 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/shift/UI/spropertydefaultui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'spropertydefaultui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Bool_t {
    QByteArrayData data[4];
    char stringdata[48];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Bool_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Bool_t qt_meta_stringdata_Shift__PropertyDefaultUI__Bool = {
    {
QT_MOC_LITERAL(0, 0, 30),
QT_MOC_LITERAL(1, 31, 10),
QT_MOC_LITERAL(2, 42, 0),
QT_MOC_LITERAL(3, 43, 3)
    },
    "Shift::PropertyDefaultUI::Bool\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Bool[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::Bool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Bool *_t = static_cast<Bool *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Bool::staticMetaObject = {
    { &SUIBase<QCheckBox,BoolProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Bool.data,
      qt_meta_data_Shift__PropertyDefaultUI__Bool,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Bool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Bool::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Bool.stringdata))
        return static_cast<void*>(const_cast< Bool*>(this));
    return SUIBase<QCheckBox,BoolProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Bool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QCheckBox,BoolProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Int32_t {
    QByteArrayData data[4];
    char stringdata[49];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Int32_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Int32_t qt_meta_stringdata_Shift__PropertyDefaultUI__Int32 = {
    {
QT_MOC_LITERAL(0, 0, 31),
QT_MOC_LITERAL(1, 32, 10),
QT_MOC_LITERAL(2, 43, 0),
QT_MOC_LITERAL(3, 44, 3)
    },
    "Shift::PropertyDefaultUI::Int32\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Int32[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::Int32::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Int32 *_t = static_cast<Int32 *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Int32::staticMetaObject = {
    { &SUIBase<QSpinBox,IntProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Int32.data,
      qt_meta_data_Shift__PropertyDefaultUI__Int32,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Int32::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Int32::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Int32.stringdata))
        return static_cast<void*>(const_cast< Int32*>(this));
    return SUIBase<QSpinBox,IntProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Int32::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QSpinBox,IntProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__UInt32_t {
    QByteArrayData data[4];
    char stringdata[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__UInt32_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__UInt32_t qt_meta_stringdata_Shift__PropertyDefaultUI__UInt32 = {
    {
QT_MOC_LITERAL(0, 0, 32),
QT_MOC_LITERAL(1, 33, 10),
QT_MOC_LITERAL(2, 44, 0),
QT_MOC_LITERAL(3, 45, 3)
    },
    "Shift::PropertyDefaultUI::UInt32\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__UInt32[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::UInt32::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        UInt32 *_t = static_cast<UInt32 *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::UInt32::staticMetaObject = {
    { &SUIBase<QSpinBox,UnsignedIntProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__UInt32.data,
      qt_meta_data_Shift__PropertyDefaultUI__UInt32,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::UInt32::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::UInt32::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__UInt32.stringdata))
        return static_cast<void*>(const_cast< UInt32*>(this));
    return SUIBase<QSpinBox,UnsignedIntProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::UInt32::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QSpinBox,UnsignedIntProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Int64_t {
    QByteArrayData data[4];
    char stringdata[49];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Int64_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Int64_t qt_meta_stringdata_Shift__PropertyDefaultUI__Int64 = {
    {
QT_MOC_LITERAL(0, 0, 31),
QT_MOC_LITERAL(1, 32, 10),
QT_MOC_LITERAL(2, 43, 0),
QT_MOC_LITERAL(3, 44, 3)
    },
    "Shift::PropertyDefaultUI::Int64\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Int64[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::Int64::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Int64 *_t = static_cast<Int64 *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Int64::staticMetaObject = {
    { &SUIBase<QSpinBox,LongIntProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Int64.data,
      qt_meta_data_Shift__PropertyDefaultUI__Int64,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Int64::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Int64::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Int64.stringdata))
        return static_cast<void*>(const_cast< Int64*>(this));
    return SUIBase<QSpinBox,LongIntProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Int64::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QSpinBox,LongIntProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__UInt64_t {
    QByteArrayData data[4];
    char stringdata[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__UInt64_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__UInt64_t qt_meta_stringdata_Shift__PropertyDefaultUI__UInt64 = {
    {
QT_MOC_LITERAL(0, 0, 32),
QT_MOC_LITERAL(1, 33, 10),
QT_MOC_LITERAL(2, 44, 0),
QT_MOC_LITERAL(3, 45, 3)
    },
    "Shift::PropertyDefaultUI::UInt64\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__UInt64[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::UInt64::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        UInt64 *_t = static_cast<UInt64 *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::UInt64::staticMetaObject = {
    { &SUIBase<QSpinBox,LongUnsignedIntProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__UInt64.data,
      qt_meta_data_Shift__PropertyDefaultUI__UInt64,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::UInt64::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::UInt64::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__UInt64.stringdata))
        return static_cast<void*>(const_cast< UInt64*>(this));
    return SUIBase<QSpinBox,LongUnsignedIntProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::UInt64::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QSpinBox,LongUnsignedIntProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Float_t {
    QByteArrayData data[4];
    char stringdata[49];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Float_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Float_t qt_meta_stringdata_Shift__PropertyDefaultUI__Float = {
    {
QT_MOC_LITERAL(0, 0, 31),
QT_MOC_LITERAL(1, 32, 10),
QT_MOC_LITERAL(2, 43, 0),
QT_MOC_LITERAL(3, 44, 3)
    },
    "Shift::PropertyDefaultUI::Float\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Float[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::Float::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Float *_t = static_cast<Float *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Float::staticMetaObject = {
    { &SUIBase<XFloatWidget,FloatProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Float.data,
      qt_meta_data_Shift__PropertyDefaultUI__Float,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Float::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Float::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Float.stringdata))
        return static_cast<void*>(const_cast< Float*>(this));
    return SUIBase<XFloatWidget,FloatProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Float::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<XFloatWidget,FloatProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Double_t {
    QByteArrayData data[4];
    char stringdata[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Double_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Double_t qt_meta_stringdata_Shift__PropertyDefaultUI__Double = {
    {
QT_MOC_LITERAL(0, 0, 32),
QT_MOC_LITERAL(1, 33, 10),
QT_MOC_LITERAL(2, 44, 0),
QT_MOC_LITERAL(3, 45, 3)
    },
    "Shift::PropertyDefaultUI::Double\0"
    "guiChanged\0\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Double[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,

       0        // eod
};

void Shift::PropertyDefaultUI::Double::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Double *_t = static_cast<Double *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Double::staticMetaObject = {
    { &SUIBase<XFloatWidget,DoubleProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Double.data,
      qt_meta_data_Shift__PropertyDefaultUI__Double,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Double::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Double::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Double.stringdata))
        return static_cast<void*>(const_cast< Double*>(this));
    return SUIBase<XFloatWidget,DoubleProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Double::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<XFloatWidget,DoubleProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__String_t {
    QByteArrayData data[3];
    char stringdata[46];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__String_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__String_t qt_meta_stringdata_Shift__PropertyDefaultUI__String = {
    {
QT_MOC_LITERAL(0, 0, 32),
QT_MOC_LITERAL(1, 33, 10),
QT_MOC_LITERAL(2, 44, 0)
    },
    "Shift::PropertyDefaultUI::String\0"
    "guiChanged\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__String[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void Shift::PropertyDefaultUI::String::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        String *_t = static_cast<String *>(_o);
        switch (_id) {
        case 0: _t->guiChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Shift::PropertyDefaultUI::String::staticMetaObject = {
    { &SUIBase<QLineEdit,StringProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__String.data,
      qt_meta_data_Shift__PropertyDefaultUI__String,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::String::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::String::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__String.stringdata))
        return static_cast<void*>(const_cast< String*>(this));
    return SUIBase<QLineEdit,StringProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::String::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QLineEdit,StringProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__LongString_t {
    QByteArrayData data[3];
    char stringdata[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__LongString_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__LongString_t qt_meta_stringdata_Shift__PropertyDefaultUI__LongString = {
    {
QT_MOC_LITERAL(0, 0, 36),
QT_MOC_LITERAL(1, 37, 10),
QT_MOC_LITERAL(2, 48, 0)
    },
    "Shift::PropertyDefaultUI::LongString\0"
    "guiChanged\0\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__LongString[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void Shift::PropertyDefaultUI::LongString::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LongString *_t = static_cast<LongString *>(_o);
        switch (_id) {
        case 0: _t->guiChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Shift::PropertyDefaultUI::LongString::staticMetaObject = {
    { &SUIBase<QTextEdit,StringProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__LongString.data,
      qt_meta_data_Shift__PropertyDefaultUI__LongString,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::LongString::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::LongString::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__LongString.stringdata))
        return static_cast<void*>(const_cast< LongString*>(this));
    return SUIBase<QTextEdit,StringProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::LongString::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QTextEdit,StringProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Vector2D_t {
    QByteArrayData data[5];
    char stringdata[66];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Vector2D_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Vector2D_t qt_meta_stringdata_Shift__PropertyDefaultUI__Vector2D = {
    {
QT_MOC_LITERAL(0, 0, 34),
QT_MOC_LITERAL(1, 35, 10),
QT_MOC_LITERAL(2, 46, 0),
QT_MOC_LITERAL(3, 47, 13),
QT_MOC_LITERAL(4, 61, 3)
    },
    "Shift::PropertyDefaultUI::Vector2D\0"
    "guiChanged\0\0Eks::Vector2D\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Vector2D[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x08,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Shift::PropertyDefaultUI::Vector2D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Vector2D *_t = static_cast<Vector2D *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< Eks::Vector2D(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Eks::Vector2D >(); break;
            }
            break;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Vector2D::staticMetaObject = {
    { &SUIBase<XVector2DWidget,Vector2DProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Vector2D.data,
      qt_meta_data_Shift__PropertyDefaultUI__Vector2D,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Vector2D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Vector2D::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Vector2D.stringdata))
        return static_cast<void*>(const_cast< Vector2D*>(this));
    return SUIBase<XVector2DWidget,Vector2DProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Vector2D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<XVector2DWidget,Vector2DProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Vector3D_t {
    QByteArrayData data[5];
    char stringdata[66];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Vector3D_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Vector3D_t qt_meta_stringdata_Shift__PropertyDefaultUI__Vector3D = {
    {
QT_MOC_LITERAL(0, 0, 34),
QT_MOC_LITERAL(1, 35, 10),
QT_MOC_LITERAL(2, 46, 0),
QT_MOC_LITERAL(3, 47, 13),
QT_MOC_LITERAL(4, 61, 3)
    },
    "Shift::PropertyDefaultUI::Vector3D\0"
    "guiChanged\0\0Eks::Vector3D\0val\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Vector3D[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x08,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Shift::PropertyDefaultUI::Vector3D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Vector3D *_t = static_cast<Vector3D *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< Eks::Vector3D(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Eks::Vector3D >(); break;
            }
            break;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Vector3D::staticMetaObject = {
    { &SUIBase<XVector3DWidget,Vector3DProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Vector3D.data,
      qt_meta_data_Shift__PropertyDefaultUI__Vector3D,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Vector3D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Vector3D::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Vector3D.stringdata))
        return static_cast<void*>(const_cast< Vector3D*>(this));
    return SUIBase<XVector3DWidget,Vector3DProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Vector3D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<XVector3DWidget,Vector3DProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Colour_t {
    QByteArrayData data[5];
    char stringdata[62];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Colour_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Colour_t qt_meta_stringdata_Shift__PropertyDefaultUI__Colour = {
    {
QT_MOC_LITERAL(0, 0, 32),
QT_MOC_LITERAL(1, 33, 10),
QT_MOC_LITERAL(2, 44, 0),
QT_MOC_LITERAL(3, 45, 11),
QT_MOC_LITERAL(4, 57, 3)
    },
    "Shift::PropertyDefaultUI::Colour\0"
    "guiChanged\0\0Eks::Colour\0col\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Colour[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x08,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Shift::PropertyDefaultUI::Colour::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Colour *_t = static_cast<Colour *>(_o);
        switch (_id) {
        case 0: _t->guiChanged((*reinterpret_cast< const Eks::Colour(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Eks::Colour >(); break;
            }
            break;
        }
    }
}

const QMetaObject Shift::PropertyDefaultUI::Colour::staticMetaObject = {
    { &SUIBase<XColourWidget,ColourProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Colour.data,
      qt_meta_data_Shift__PropertyDefaultUI__Colour,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Colour::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Colour::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Colour.stringdata))
        return static_cast<void*>(const_cast< Colour*>(this));
    return SUIBase<XColourWidget,ColourProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Colour::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<XColourWidget,ColourProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_Shift__PropertyDefaultUI__Filename_t {
    QByteArrayData data[4];
    char stringdata[56];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__PropertyDefaultUI__Filename_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__PropertyDefaultUI__Filename_t qt_meta_stringdata_Shift__PropertyDefaultUI__Filename = {
    {
QT_MOC_LITERAL(0, 0, 34),
QT_MOC_LITERAL(1, 35, 10),
QT_MOC_LITERAL(2, 46, 0),
QT_MOC_LITERAL(3, 47, 7)
    },
    "Shift::PropertyDefaultUI::Filename\0"
    "guiChanged\0\0syncGUI\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__PropertyDefaultUI__Filename[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x08,
       3,    0,   25,    2, 0x08,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Shift::PropertyDefaultUI::Filename::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Filename *_t = static_cast<Filename *>(_o);
        switch (_id) {
        case 0: _t->guiChanged(); break;
        case 1: _t->syncGUI(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Shift::PropertyDefaultUI::Filename::staticMetaObject = {
    { &SUIBase<QWidget,FilenameProperty>::staticMetaObject, qt_meta_stringdata_Shift__PropertyDefaultUI__Filename.data,
      qt_meta_data_Shift__PropertyDefaultUI__Filename,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::PropertyDefaultUI::Filename::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::PropertyDefaultUI::Filename::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__PropertyDefaultUI__Filename.stringdata))
        return static_cast<void*>(const_cast< Filename*>(this));
    return SUIBase<QWidget,FilenameProperty>::qt_metacast(_clname);
}

int Shift::PropertyDefaultUI::Filename::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = SUIBase<QWidget,FilenameProperty>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
