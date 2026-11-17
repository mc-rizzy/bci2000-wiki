/****************************************************************************
** Meta object code from reading C++ file 'pages.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/core/Tools/P300Classifier/pages.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pages.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WeightsWorkerThread_t {
    QByteArrayData data[9];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WeightsWorkerThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WeightsWorkerThread_t qt_meta_stringdata_WeightsWorkerThread = {
    {
QT_MOC_LITERAL(0, 0, 19), // "WeightsWorkerThread"
QT_MOC_LITERAL(1, 20, 17), // "signalProgressBar"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 4), // "done"
QT_MOC_LITERAL(4, 44, 5), // "total"
QT_MOC_LITERAL(5, 50, 5), // "stage"
QT_MOC_LITERAL(6, 56, 18), // "signalProgressText"
QT_MOC_LITERAL(7, 75, 7), // "message"
QT_MOC_LITERAL(8, 83, 12) // "errorMessage"

    },
    "WeightsWorkerThread\0signalProgressBar\0"
    "\0done\0total\0stage\0signalProgressText\0"
    "message\0errorMessage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WeightsWorkerThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   34,    2, 0x06 /* Public */,
       6,    1,   41,    2, 0x06 /* Public */,
       8,    1,   44,    2, 0x06 /* Public */,
       3,    0,   47,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,

       0        // eod
};

void WeightsWorkerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WeightsWorkerThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalProgressBar((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->signalProgressText((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->errorMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->done(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WeightsWorkerThread::*)(int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WeightsWorkerThread::signalProgressBar)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (WeightsWorkerThread::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WeightsWorkerThread::signalProgressText)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (WeightsWorkerThread::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WeightsWorkerThread::errorMessage)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (WeightsWorkerThread::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WeightsWorkerThread::done)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WeightsWorkerThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_WeightsWorkerThread.data,
    qt_meta_data_WeightsWorkerThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WeightsWorkerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WeightsWorkerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WeightsWorkerThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int WeightsWorkerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void WeightsWorkerThread::signalProgressBar(int _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void WeightsWorkerThread::signalProgressText(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void WeightsWorkerThread::errorMessage(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void WeightsWorkerThread::done()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
struct qt_meta_stringdata_DataPage_t {
    QByteArrayData data[18];
    char stringdata0[241];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DataPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DataPage_t qt_meta_stringdata_DataPage = {
    {
QT_MOC_LITERAL(0, 0, 8), // "DataPage"
QT_MOC_LITERAL(1, 9, 16), // "slotProgressText"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 7), // "message"
QT_MOC_LITERAL(4, 35, 15), // "slotProgressBar"
QT_MOC_LITERAL(5, 51, 4), // "done"
QT_MOC_LITERAL(6, 56, 5), // "total"
QT_MOC_LITERAL(7, 62, 5), // "stage"
QT_MOC_LITERAL(8, 68, 22), // "slotErrorMessageDialog"
QT_MOC_LITERAL(9, 91, 12), // "errorMessage"
QT_MOC_LITERAL(10, 104, 18), // "LoadTrainingButton"
QT_MOC_LITERAL(11, 123, 17), // "LoadTestingButton"
QT_MOC_LITERAL(12, 141, 13), // "LoadIniButton"
QT_MOC_LITERAL(13, 155, 16), // "GenerateFeatures"
QT_MOC_LITERAL(14, 172, 13), // "ApplyFeatures"
QT_MOC_LITERAL(15, 186, 22), // "WriteParameterFragment"
QT_MOC_LITERAL(16, 209, 10), // "showDialog"
QT_MOC_LITERAL(17, 220, 20) // "OnFeatureWeightsDone"

    },
    "DataPage\0slotProgressText\0\0message\0"
    "slotProgressBar\0done\0total\0stage\0"
    "slotErrorMessageDialog\0errorMessage\0"
    "LoadTrainingButton\0LoadTestingButton\0"
    "LoadIniButton\0GenerateFeatures\0"
    "ApplyFeatures\0WriteParameterFragment\0"
    "showDialog\0OnFeatureWeightsDone"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DataPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x0a /* Public */,
       4,    3,   72,    2, 0x0a /* Public */,
       8,    1,   79,    2, 0x0a /* Public */,
      10,    0,   82,    2, 0x0a /* Public */,
      11,    0,   83,    2, 0x0a /* Public */,
      12,    0,   84,    2, 0x0a /* Public */,
      13,    0,   85,    2, 0x0a /* Public */,
      14,    0,   86,    2, 0x0a /* Public */,
      15,    1,   87,    2, 0x0a /* Public */,
      15,    0,   90,    2, 0x2a /* Public | MethodCloned */,
      17,    0,   91,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    5,    6,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::Bool,   16,
    QMetaType::Bool,
    QMetaType::Void,

       0        // eod
};

void DataPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DataPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotProgressText((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->slotProgressBar((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->slotErrorMessageDialog((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->LoadTrainingButton(); break;
        case 4: _t->LoadTestingButton(); break;
        case 5: _t->LoadIniButton(); break;
        case 6: _t->GenerateFeatures(); break;
        case 7: _t->ApplyFeatures(); break;
        case 8: { bool _r = _t->WriteParameterFragment((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->WriteParameterFragment();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 10: _t->OnFeatureWeightsDone(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DataPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_DataPage.data,
    qt_meta_data_DataPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DataPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DataPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DataPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
struct qt_meta_stringdata_ParametersPage_t {
    QByteArrayData data[5];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ParametersPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ParametersPage_t qt_meta_stringdata_ParametersPage = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ParametersPage"
QT_MOC_LITERAL(1, 15, 14), // "TextHasChanged"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 19), // "ChSetTextHasChanged"
QT_MOC_LITERAL(4, 51, 25) // "OnsetExpressionHasChanged"

    },
    "ParametersPage\0TextHasChanged\0\0"
    "ChSetTextHasChanged\0OnsetExpressionHasChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ParametersPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,
       3,    0,   30,    2, 0x0a /* Public */,
       4,    0,   31,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ParametersPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ParametersPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->TextHasChanged(); break;
        case 1: _t->ChSetTextHasChanged(); break;
        case 2: _t->OnsetExpressionHasChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject ParametersPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ParametersPage.data,
    qt_meta_data_ParametersPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ParametersPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ParametersPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ParametersPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ParametersPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
struct qt_meta_stringdata_DetailsPage_t {
    QByteArrayData data[1];
    char stringdata0[12];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DetailsPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DetailsPage_t qt_meta_stringdata_DetailsPage = {
    {
QT_MOC_LITERAL(0, 0, 11) // "DetailsPage"

    },
    "DetailsPage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DetailsPage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DetailsPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject DetailsPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_DetailsPage.data,
    qt_meta_data_DetailsPage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DetailsPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DetailsPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DetailsPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DetailsPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
