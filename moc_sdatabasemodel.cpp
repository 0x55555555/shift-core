/****************************************************************************
** Meta object code from reading C++ file 'sdatabasemodel.h'
**
** Created: Mon Jul 2 21:31:24 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sdatabasemodel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sdatabasemodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SDatabaseDelegate[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SDatabaseDelegate[] = {
    "SDatabaseDelegate\0\0currentItemDestroyed()\0"
};

void SDatabaseDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SDatabaseDelegate *_t = static_cast<SDatabaseDelegate *>(_o);
        switch (_id) {
        case 0: _t->currentItemDestroyed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData SDatabaseDelegate::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SDatabaseDelegate::staticMetaObject = {
    { &QItemDelegate::staticMetaObject, qt_meta_stringdata_SDatabaseDelegate,
      qt_meta_data_SDatabaseDelegate, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SDatabaseDelegate::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SDatabaseDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SDatabaseDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SDatabaseDelegate))
        return static_cast<void*>(const_cast< SDatabaseDelegate*>(this));
    return QItemDelegate::qt_metacast(_clname);
}

int SDatabaseDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QItemDelegate::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_SDatabaseModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: signature, parameters, type, tag, flags
      28,   15,   16,   15, 0x02,
      44,   40,   35,   15, 0x02,
      79,   77,   35,   15, 0x02,
     104,   77,  100,   15, 0x02,
     126,   77,  100,   15, 0x02,
     158,  151,  100,   15, 0x02,
     180,   15,  100,   15, 0x22,
     209,  191,   16,   15, 0x02,
     247,  236,   16,   15, 0x22,
     268,  262,   16,   15, 0x02,
     308,  297,  288,   15, 0x02,
     351,  334,   35,   15, 0x02,

       0        // eod
};

static const char qt_meta_stringdata_SDatabaseModel[] = {
    "SDatabaseModel\0\0QModelIndex\0root()\0"
    "bool\0a,b\0isEqual(QModelIndex,QModelIndex)\0"
    "i\0isValid(QModelIndex)\0int\0"
    "rowIndex(QModelIndex)\0columnIndex(QModelIndex)\0"
    "parent\0rowCount(QModelIndex)\0rowCount()\0"
    "row,column,parent\0index(int,int,QModelIndex)\0"
    "row,column\0index(int,int)\0child\0"
    "parent(QModelIndex)\0QVariant\0index,role\0"
    "data(QModelIndex,QString)\0index,role,value\0"
    "setData(QModelIndex,QString,QVariant)\0"
};

void SDatabaseModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SDatabaseModel *_t = static_cast<SDatabaseModel *>(_o);
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

const QMetaObjectExtraData SDatabaseModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SDatabaseModel::staticMetaObject = {
    { &QAbstractItemModel::staticMetaObject, qt_meta_stringdata_SDatabaseModel,
      qt_meta_data_SDatabaseModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SDatabaseModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SDatabaseModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SDatabaseModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SDatabaseModel))
        return static_cast<void*>(const_cast< SDatabaseModel*>(this));
    if (!strcmp(_clname, "STreeObserver"))
        return static_cast< STreeObserver*>(const_cast< SDatabaseModel*>(this));
    return QAbstractItemModel::qt_metacast(_clname);
}

int SDatabaseModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractItemModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
