/****************************************************************************
** Meta object code from reading C++ file 'sdatabasemodel.h'
**
** Created: Tue 1. Jan 15:26:31 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/shift/UI/sdatabasemodel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sdatabasemodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Shift__DatabaseDelegate_t {
    QByteArrayData data[3];
    char stringdata[47];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__DatabaseDelegate_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__DatabaseDelegate_t qt_meta_stringdata_Shift__DatabaseDelegate = {
    {
QT_MOC_LITERAL(0, 0, 23),
QT_MOC_LITERAL(1, 24, 20),
QT_MOC_LITERAL(2, 45, 0)
    },
    "Shift::DatabaseDelegate\0currentItemDestroyed\0"
    "\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__DatabaseDelegate[] = {

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
       1,    0,   19,    2, 0x08,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void Shift::DatabaseDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DatabaseDelegate *_t = static_cast<DatabaseDelegate *>(_o);
        switch (_id) {
        case 0: _t->currentItemDestroyed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Shift::DatabaseDelegate::staticMetaObject = {
    { &QStyledItemDelegate::staticMetaObject, qt_meta_stringdata_Shift__DatabaseDelegate.data,
      qt_meta_data_Shift__DatabaseDelegate,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::DatabaseDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::DatabaseDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__DatabaseDelegate.stringdata))
        return static_cast<void*>(const_cast< DatabaseDelegate*>(this));
    return QStyledItemDelegate::qt_metacast(_clname);
}

int Shift::DatabaseDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_Shift__DatabaseModel_t {
    QByteArrayData data[20];
    char stringdata[134];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Shift__DatabaseModel_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Shift__DatabaseModel_t qt_meta_stringdata_Shift__DatabaseModel = {
    {
QT_MOC_LITERAL(0, 0, 20),
QT_MOC_LITERAL(1, 21, 4),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 7),
QT_MOC_LITERAL(4, 35, 1),
QT_MOC_LITERAL(5, 37, 1),
QT_MOC_LITERAL(6, 39, 7),
QT_MOC_LITERAL(7, 47, 1),
QT_MOC_LITERAL(8, 49, 8),
QT_MOC_LITERAL(9, 58, 11),
QT_MOC_LITERAL(10, 70, 8),
QT_MOC_LITERAL(11, 79, 6),
QT_MOC_LITERAL(12, 86, 5),
QT_MOC_LITERAL(13, 92, 3),
QT_MOC_LITERAL(14, 96, 6),
QT_MOC_LITERAL(15, 103, 5),
QT_MOC_LITERAL(16, 109, 4),
QT_MOC_LITERAL(17, 114, 4),
QT_MOC_LITERAL(18, 119, 7),
QT_MOC_LITERAL(19, 127, 5)
    },
    "Shift::DatabaseModel\0root\0\0isEqual\0a\0"
    "b\0isValid\0i\0rowIndex\0columnIndex\0"
    "rowCount\0parent\0index\0row\0column\0child\0"
    "data\0role\0setData\0value\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Shift__DatabaseModel[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x02,
       3,    2,   75,    2, 0x02,
       6,    1,   80,    2, 0x02,
       8,    1,   83,    2, 0x02,
       9,    1,   86,    2, 0x02,
      10,    1,   89,    2, 0x02,
      10,    0,   92,    2, 0x22,
      12,    3,   93,    2, 0x02,
      12,    2,  100,    2, 0x22,
      11,    1,  105,    2, 0x02,
      16,    2,  108,    2, 0x02,
      18,    3,  113,    2, 0x02,

 // methods: parameters
    QMetaType::QModelIndex,
    QMetaType::Bool, QMetaType::QModelIndex, QMetaType::QModelIndex,    4,    5,
    QMetaType::Bool, QMetaType::QModelIndex,    7,
    QMetaType::Int, QMetaType::QModelIndex,    7,
    QMetaType::Int, QMetaType::QModelIndex,    7,
    QMetaType::Int, QMetaType::QModelIndex,   11,
    QMetaType::Int,
    QMetaType::QModelIndex, QMetaType::Int, QMetaType::Int, QMetaType::QModelIndex,   13,   14,   11,
    QMetaType::QModelIndex, QMetaType::Int, QMetaType::Int,   13,   14,
    QMetaType::QModelIndex, QMetaType::QModelIndex,   15,
    QMetaType::QVariant, QMetaType::QModelIndex, QMetaType::QString,   12,   17,
    QMetaType::Bool, QMetaType::QModelIndex, QMetaType::QString, QMetaType::QVariant,   12,   17,   19,

       0        // eod
};

void Shift::DatabaseModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DatabaseModel *_t = static_cast<DatabaseModel *>(_o);
        switch (_id) {
        case 0: { QModelIndex _r = _t->root();
            if (_a[0]) *reinterpret_cast< QModelIndex*>(_a[0]) = _r; }  break;
        case 1: { bool _r = _t->isEqual((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: { bool _r = _t->isValid((*reinterpret_cast< const QModelIndex(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 3: { int _r = _t->rowIndex((*reinterpret_cast< const QModelIndex(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = _t->columnIndex((*reinterpret_cast< const QModelIndex(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: { int _r = _t->rowCount((*reinterpret_cast< const QModelIndex(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { int _r = _t->rowCount();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 7: { QModelIndex _r = _t->index((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QModelIndex(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< QModelIndex*>(_a[0]) = _r; }  break;
        case 8: { QModelIndex _r = _t->index((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QModelIndex*>(_a[0]) = _r; }  break;
        case 9: { QModelIndex _r = _t->parent((*reinterpret_cast< const QModelIndex(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QModelIndex*>(_a[0]) = _r; }  break;
        case 10: { QVariant _r = _t->data((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QVariant*>(_a[0]) = _r; }  break;
        case 11: { bool _r = _t->setData((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QVariant(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObject Shift::DatabaseModel::staticMetaObject = {
    { &QAbstractItemModel::staticMetaObject, qt_meta_stringdata_Shift__DatabaseModel.data,
      qt_meta_data_Shift__DatabaseModel,  qt_static_metacall, 0, 0}
};


const QMetaObject *Shift::DatabaseModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Shift::DatabaseModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Shift__DatabaseModel.stringdata))
        return static_cast<void*>(const_cast< DatabaseModel*>(this));
    if (!strcmp(_clname, "TreeObserver"))
        return static_cast< TreeObserver*>(const_cast< DatabaseModel*>(this));
    return QAbstractItemModel::qt_metacast(_clname);
}

int Shift::DatabaseModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractItemModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
