/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include "CacheSimulator.h"
#include "ui_mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <string>
#include <iostream>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[3];
    char stringdata0[33];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 20), // "on_start_btn_clicked"
QT_MOC_LITERAL(2, 32, 0) // ""

    },
    "MainWindow\0on_start_btn_clicked\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

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
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_start_btn_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_start_btn_clicked()
{
    char test_case[100] = "";
    string cases = ui->box_file->currentText().toStdString();
    cases = "C:\\test_case\\"+ cases;
    strcpy(test_case, cases.c_str());

    string l1_size = ui->box_l1_size->currentText().toStdString();
    string l2_size = ui->box_l2_size->currentText().toStdString();
    string l1_line_size = ui->box_l1_line->currentText().toStdString();
    string l2_line_size = ui->box_l2_line->currentText().toStdString();
    string l1_ways = ui->box_l1_way->currentText().toStdString();
    string l2_ways = ui->box_l2_way->currentText().toStdString();
    string l1_replacement_strategy = ui->box_replace1->currentText().toStdString();
    string l2_replacement_strategy = ui->box_replace2->currentText().toStdString();

    string parity_str = ui->box_parity->currentText().toStdString();
    string secded_str = ui->box_secded->currentText().toStdString();
    string mlreps_str = ui->box_mlreps->currentText().toStdString();
    string early_write_str = ui->box_early->currentText().toStdString();
    string emergency_write_str = ui->box_emergency->currentText().toStdString();

    string error_type_str = ui->box_fault->currentText().toStdString();
    string inject_time__str = ui->box_inject->currentText().toStdString();

    // get cache config
    CacheSim cache;
    _u64 line_size[] = { 0, 0 };
    _u64 ways[] = { 0, 0 };
    _u64 cache_size[] = { 0, 0 };
    int l1_replace = 0;
    int l2_replace = 0;
    cache_size[0] = atoi(l1_size.c_str());
    cache_size[1] = atoi(l2_size.c_str());
    line_size[0] = atoi(l1_line_size.c_str());
    line_size[1] = atoi(l2_line_size.c_str());
    ways[0] = atoi(l1_ways.c_str());
    ways[1] = atoi(l2_ways.c_str());

    if (l1_replacement_strategy == "LRU") {
        l1_replace = 0;
    }
    else if (l1_replacement_strategy == "FIFO") {
        l1_replace = 1;
    }
    else if (l1_replacement_strategy == "RAND") {
        l1_replace = 2;
    }

    if (l2_replacement_strategy == "LRU") {
        l2_replace = 0;
    }
    else if (l2_replacement_strategy == "FIFO") {
        l2_replace = 1;
    }
    else if (l2_replacement_strategy == "RAND") {
        l2_replace = 2;
    }

    // get tolerance config
    int parity = 0;
    int secded = 0;
    int mlreps = 0;
    int early_write = 0;
    int emergency_write = 0;

    if (parity_str == "none") {
        parity = 0;
    }
    else if (parity_str == "ordinary") {
        parity = 1;
    }
    else if (parity_str == "cross") {
        parity = 2;
    }

    if (secded_str == "none") {
        secded = 0;
    }
    else if (secded_str == "hamming") {
        secded = 1;
    }
    else if (secded_str == "sec-ded") {
        secded = 2;
    }

    if (mlreps_str == "none") {
        mlreps = 0;
    }
    else if (mlreps_str == "mlreps") {
        mlreps = 1;
    }

    if (early_write_str == "none") {
        early_write = 0;
    }
    else if (early_write_str == "use") {
        early_write = 1;
    }

    if (emergency_write_str == "none") {
        emergency_write = 0;
    }
    else if (emergency_write_str == "use") {
        emergency_write = 1;
    }

    // get error inject config
    int error_type = 0;
    int inject_times = 0;

    if (error_type_str == "none") {
        error_type = 0;
    }
    else if (error_type_str == "SBU") {
        error_type = 1;
    }
    else if (error_type_str == "adjacent 2") {
        error_type = 2;
    }
    else if (error_type_str == "adjacent 3") {
        error_type = 3;
    }
    else if (error_type_str == "adjacent 4") {
        error_type = 4;
    }
    else if (error_type_str == "adjacent 5") {
        error_type = 5;
    }
    else if (error_type_str == "random 1-2") {
        error_type = 12;
    }
    else if (error_type_str == "random 1-3") {
        error_type = 13;
    }
    else if (error_type_str == "random 1-4") {
        error_type = 14;
    }
    else if (error_type_str == "random 1-5") {
        error_type = 15;
    }

    if (inject_time__str == "10000") {
        inject_times = 10000;
    }
    else if (inject_time__str == "3000") {
        inject_times = 3000;
    }
    else if (inject_time__str == "6000") {
        inject_times = 6000;
    }
    else if (inject_time__str == "9000") {
        inject_times = 9000;
    }
    else if (inject_time__str == "12000") {
        inject_times = 12000;
    }
    else if (inject_time__str == "15000") {
        inject_times = 15000;
    }

    // go on
    cache.init(cache_size, line_size, ways, l1_replace, l2_replace, parity, secded,
        mlreps, early_write, emergency_write, error_type, inject_times,ui);
    char testcase_core1[100] = "";
    string file_add = "C:\\test_case _1\\gcc.trace";
    strcpy(testcase_core1, file_add.c_str());
    std::thread core0(&CacheSim::fun1, &cache, test_case, 0,ui);
    std::thread core1(&CacheSim::fun1, &cache, testcase_core1, 1,ui);
    core0.detach();
    core1.join();
    //cache.fun1(test_case, 0, ui);
    

    //cache.load_trace(test_case, core_num,ui);
}

QT_WARNING_POP
QT_END_MOC_NAMESPACE


