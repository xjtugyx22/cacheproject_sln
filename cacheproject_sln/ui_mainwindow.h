/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.9
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget* centralwidget;
    QTextBrowser* textBrowser;
    QLabel* cache_conf;
    QLabel* l1_cache;
    QLabel* l2_cache;
    QLabel* cache_conf_2;
    QLabel* cache_conf_3;
    QPushButton* start_btn;
    QWidget* layoutWidget;
    QHBoxLayout* horizontalLayout;
    QLabel* load_file;
    QComboBox* box_file;
    QWidget* layoutWidget1;
    QHBoxLayout* horizontalLayout_3;
    QVBoxLayout* verticalLayout_3;
    QLabel* parity;
    QLabel* secded;
    QLabel* mlreps;
    QLabel* early_write;
    QLabel* early_write_2;
    QVBoxLayout* verticalLayout_7;
    QComboBox* box_parity;
    QComboBox* box_secded;
    QComboBox* box_mlreps;
    QComboBox* box_early;
    QComboBox* box_emergency;
    QWidget* layoutWidget2;
    QVBoxLayout* verticalLayout_8;
    QHBoxLayout* horizontalLayout_10;
    QLabel* fault_type;
    QComboBox* box_fault;
    QHBoxLayout* horizontalLayout_4;
    QLabel* inject_times;
    QComboBox* box_inject;
    QWidget* layoutWidget3;
    QVBoxLayout* verticalLayout_9;
    QHBoxLayout* horizontalLayout_2;
    QVBoxLayout* verticalLayout_2;
    QLabel* cache_size;
    QLabel* line_size;
    QLabel* ways;
    QVBoxLayout* verticalLayout;
    QComboBox* box_l1_size;
    QComboBox* box_l1_line;
    QComboBox* box_l1_way;
    QVBoxLayout* verticalLayout_4;
    QLabel* label_kb1;
    QLabel* label_b1;
    QLabel* label_temp1;
    QVBoxLayout* verticalLayout_5;
    QComboBox* box_l2_size;
    QComboBox* box_l2_line;
    QComboBox* box_l2_way;
    QVBoxLayout* verticalLayout_6;
    QLabel* label_kb2;
    QLabel* label_b2;
    QLabel* label_temp2;
    QHBoxLayout* horizontalLayout_5;
    QLabel* replace;
    QComboBox* box_replace1;
    QComboBox* box_replace2;
    QMenuBar* menubar;
    QStatusBar* statusbar;

    void setupUi(QMainWindow* MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(682, 669);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        textBrowser = new QTextBrowser(centralwidget);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setGeometry(QRect(340, 30, 301, 531));
        cache_conf = new QLabel(centralwidget);
        cache_conf->setObjectName(QStringLiteral("cache_conf"));
        cache_conf->setGeometry(QRect(80, 80, 161, 26));
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        cache_conf->setFont(font);
        l1_cache = new QLabel(centralwidget);
        l1_cache->setObjectName(QStringLiteral("l1_cache"));
        l1_cache->setGeometry(QRect(130, 110, 21, 26));
        QFont font1;
        font1.setPointSize(12);
        l1_cache->setFont(font1);
        l2_cache = new QLabel(centralwidget);
        l2_cache->setObjectName(QStringLiteral("l2_cache"));
        l2_cache->setGeometry(QRect(220, 110, 21, 26));
        l2_cache->setFont(font1);
        cache_conf_2 = new QLabel(centralwidget);
        cache_conf_2->setObjectName(QStringLiteral("cache_conf_2"));
        cache_conf_2->setGeometry(QRect(48, 298, 231, 26));
        cache_conf_2->setFont(font);
        cache_conf_3 = new QLabel(centralwidget);
        cache_conf_3->setObjectName(QStringLiteral("cache_conf_3"));
        cache_conf_3->setGeometry(QRect(38, 528, 231, 26));
        cache_conf_3->setFont(font);
        start_btn = new QPushButton(centralwidget);
        start_btn->setObjectName(QStringLiteral("start_btn"));
        start_btn->setGeometry(QRect(460, 590, 89, 25));
        start_btn->setFont(font);
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 30, 218, 28));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        load_file = new QLabel(layoutWidget);
        load_file->setObjectName(QStringLiteral("load_file"));
        load_file->setFont(font1);

        horizontalLayout->addWidget(load_file);

        box_file = new QComboBox(layoutWidget);
        box_file->setObjectName(QStringLiteral("box_file"));
        box_file->setFont(font1);

        horizontalLayout->addWidget(box_file);

        layoutWidget1 = new QWidget(centralwidget);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(20, 330, 268, 158));
        horizontalLayout_3 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        parity = new QLabel(layoutWidget1);
        parity->setObjectName(QStringLiteral("parity"));
        parity->setFont(font1);

        verticalLayout_3->addWidget(parity);

        secded = new QLabel(layoutWidget1);
        secded->setObjectName(QStringLiteral("secded"));
        secded->setFont(font1);

        verticalLayout_3->addWidget(secded);

        mlreps = new QLabel(layoutWidget1);
        mlreps->setObjectName(QStringLiteral("mlreps"));
        mlreps->setFont(font1);

        verticalLayout_3->addWidget(mlreps);

        early_write = new QLabel(layoutWidget1);
        early_write->setObjectName(QStringLiteral("early_write"));
        early_write->setFont(font1);

        verticalLayout_3->addWidget(early_write);

        early_write_2 = new QLabel(layoutWidget1);
        early_write_2->setObjectName(QStringLiteral("early_write_2"));
        early_write_2->setFont(font1);

        verticalLayout_3->addWidget(early_write_2);


        horizontalLayout_3->addLayout(verticalLayout_3);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        box_parity = new QComboBox(layoutWidget1);
        box_parity->setObjectName(QStringLiteral("box_parity"));
        box_parity->setFont(font1);

        verticalLayout_7->addWidget(box_parity);

        box_secded = new QComboBox(layoutWidget1);
        box_secded->setObjectName(QStringLiteral("box_secded"));
        box_secded->setFont(font1);

        verticalLayout_7->addWidget(box_secded);

        box_mlreps = new QComboBox(layoutWidget1);
        box_mlreps->setObjectName(QStringLiteral("box_mlreps"));
        box_mlreps->setFont(font1);

        verticalLayout_7->addWidget(box_mlreps);

        box_early = new QComboBox(layoutWidget1);
        box_early->setObjectName(QStringLiteral("box_early"));
        box_early->setFont(font1);

        verticalLayout_7->addWidget(box_early);

        box_emergency = new QComboBox(layoutWidget1);
        box_emergency->setObjectName(QStringLiteral("box_emergency"));
        box_emergency->setFont(font1);

        verticalLayout_7->addWidget(box_emergency);


        horizontalLayout_3->addLayout(verticalLayout_7);

        layoutWidget2 = new QWidget(centralwidget);
        layoutWidget2->setObjectName(QStringLiteral("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(18, 558, 283, 64));
        verticalLayout_8 = new QVBoxLayout(layoutWidget2);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        fault_type = new QLabel(layoutWidget2);
        fault_type->setObjectName(QStringLiteral("fault_type"));
        fault_type->setFont(font1);

        horizontalLayout_10->addWidget(fault_type);

        box_fault = new QComboBox(layoutWidget2);
        box_fault->setObjectName(QStringLiteral("box_fault"));
        box_fault->setFont(font1);

        horizontalLayout_10->addWidget(box_fault);


        verticalLayout_8->addLayout(horizontalLayout_10);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        inject_times = new QLabel(layoutWidget2);
        inject_times->setObjectName(QStringLiteral("inject_times"));
        inject_times->setFont(font1);

        horizontalLayout_4->addWidget(inject_times);

        box_inject = new QComboBox(layoutWidget2);
        box_inject->setObjectName(QStringLiteral("box_inject"));
        box_inject->setFont(font1);

        horizontalLayout_4->addWidget(box_inject);


        verticalLayout_8->addLayout(horizontalLayout_4);

        layoutWidget3 = new QWidget(centralwidget);
        layoutWidget3->setObjectName(QStringLiteral("layoutWidget3"));
        layoutWidget3->setGeometry(QRect(21, 134, 261, 130));
        verticalLayout_9 = new QVBoxLayout(layoutWidget3);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        cache_size = new QLabel(layoutWidget3);
        cache_size->setObjectName(QStringLiteral("cache_size"));
        cache_size->setFont(font1);

        verticalLayout_2->addWidget(cache_size);

        line_size = new QLabel(layoutWidget3);
        line_size->setObjectName(QStringLiteral("line_size"));
        line_size->setFont(font1);

        verticalLayout_2->addWidget(line_size);

        ways = new QLabel(layoutWidget3);
        ways->setObjectName(QStringLiteral("ways"));
        ways->setFont(font1);

        verticalLayout_2->addWidget(ways);


        horizontalLayout_2->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        box_l1_size = new QComboBox(layoutWidget3);
        box_l1_size->setObjectName(QStringLiteral("box_l1_size"));
        box_l1_size->setFont(font1);

        verticalLayout->addWidget(box_l1_size);

        box_l1_line = new QComboBox(layoutWidget3);
        box_l1_line->setObjectName(QStringLiteral("box_l1_line"));
        box_l1_line->setFont(font1);

        verticalLayout->addWidget(box_l1_line);

        box_l1_way = new QComboBox(layoutWidget3);
        box_l1_way->setObjectName(QStringLiteral("box_l1_way"));
        box_l1_way->setFont(font1);

        verticalLayout->addWidget(box_l1_way);


        horizontalLayout_2->addLayout(verticalLayout);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label_kb1 = new QLabel(layoutWidget3);
        label_kb1->setObjectName(QStringLiteral("label_kb1"));
        label_kb1->setFont(font1);

        verticalLayout_4->addWidget(label_kb1);

        label_b1 = new QLabel(layoutWidget3);
        label_b1->setObjectName(QStringLiteral("label_b1"));
        label_b1->setFont(font1);

        verticalLayout_4->addWidget(label_b1);

        label_temp1 = new QLabel(layoutWidget3);
        label_temp1->setObjectName(QStringLiteral("label_temp1"));
        label_temp1->setFont(font1);

        verticalLayout_4->addWidget(label_temp1);


        horizontalLayout_2->addLayout(verticalLayout_4);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        box_l2_size = new QComboBox(layoutWidget3);
        box_l2_size->setObjectName(QStringLiteral("box_l2_size"));
        box_l2_size->setFont(font1);

        verticalLayout_5->addWidget(box_l2_size);

        box_l2_line = new QComboBox(layoutWidget3);
        box_l2_line->setObjectName(QStringLiteral("box_l2_line"));
        box_l2_line->setFont(font1);

        verticalLayout_5->addWidget(box_l2_line);

        box_l2_way = new QComboBox(layoutWidget3);
        box_l2_way->setObjectName(QStringLiteral("box_l2_way"));
        box_l2_way->setFont(font1);

        verticalLayout_5->addWidget(box_l2_way);


        horizontalLayout_2->addLayout(verticalLayout_5);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        label_kb2 = new QLabel(layoutWidget3);
        label_kb2->setObjectName(QStringLiteral("label_kb2"));
        label_kb2->setFont(font1);

        verticalLayout_6->addWidget(label_kb2);

        label_b2 = new QLabel(layoutWidget3);
        label_b2->setObjectName(QStringLiteral("label_b2"));
        label_b2->setFont(font1);

        verticalLayout_6->addWidget(label_b2);

        label_temp2 = new QLabel(layoutWidget3);
        label_temp2->setObjectName(QStringLiteral("label_temp2"));
        label_temp2->setFont(font1);

        verticalLayout_6->addWidget(label_temp2);


        horizontalLayout_2->addLayout(verticalLayout_6);


        verticalLayout_9->addLayout(horizontalLayout_2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        replace = new QLabel(layoutWidget3);
        replace->setObjectName(QStringLiteral("replace"));
        replace->setFont(font1);

        horizontalLayout_5->addWidget(replace);

        box_replace1 = new QComboBox(layoutWidget3);
        box_replace1->setObjectName(QStringLiteral("box_replace1"));
        box_replace1->setFont(font1);

        horizontalLayout_5->addWidget(box_replace1);

        box_replace2 = new QComboBox(layoutWidget3);
        box_replace2->setObjectName(QStringLiteral("box_replace2"));
        box_replace2->setFont(font1);

        horizontalLayout_5->addWidget(box_replace2);


        verticalLayout_9->addLayout(horizontalLayout_5);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 682, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainWindow->setStatusBar(statusbar);
        QWidget::setTabOrder(box_file, box_l1_size);
        QWidget::setTabOrder(box_l1_size, box_l2_size);
        QWidget::setTabOrder(box_l2_size, box_l1_line);
        QWidget::setTabOrder(box_l1_line, box_l2_line);
        QWidget::setTabOrder(box_l2_line, box_l1_way);
        QWidget::setTabOrder(box_l1_way, box_l2_way);
        QWidget::setTabOrder(box_l2_way, box_replace1);
        QWidget::setTabOrder(box_replace1, box_replace2);
        QWidget::setTabOrder(box_replace2, box_parity);
        QWidget::setTabOrder(box_parity, box_secded);
        QWidget::setTabOrder(box_secded, box_mlreps);
        QWidget::setTabOrder(box_mlreps, box_early);
        QWidget::setTabOrder(box_early, box_emergency);
        QWidget::setTabOrder(box_emergency, box_fault);
        QWidget::setTabOrder(box_fault, box_inject);
        QWidget::setTabOrder(box_inject, textBrowser);
        QWidget::setTabOrder(textBrowser, start_btn);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow* MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        cache_conf->setText(QApplication::translate("MainWindow", "Cache configuration", Q_NULLPTR));
        l1_cache->setText(QApplication::translate("MainWindow", "L1", Q_NULLPTR));
        l2_cache->setText(QApplication::translate("MainWindow", "L2", Q_NULLPTR));
        cache_conf_2->setText(QApplication::translate("MainWindow", "Fault tolerant configuration", Q_NULLPTR));
        cache_conf_3->setText(QApplication::translate("MainWindow", "Fault injection configuration", Q_NULLPTR));
        start_btn->setText(QApplication::translate("MainWindow", "START", Q_NULLPTR));
        load_file->setText(QApplication::translate("MainWindow", " Load test file ", Q_NULLPTR));
        box_file->clear();
        box_file->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "gcc.trace", Q_NULLPTR)
            << QApplication::translate("MainWindow", "gzip.trace", Q_NULLPTR)
            << QApplication::translate("MainWindow", "mcf.trace", Q_NULLPTR)
            << QApplication::translate("MainWindow", "swim.trace", Q_NULLPTR)
            << QApplication::translate("MainWindow", "twolf.trace", Q_NULLPTR)
        );
        parity->setText(QApplication::translate("MainWindow", "Parity                               ", Q_NULLPTR));
        secded->setText(QApplication::translate("MainWindow", "SEC-DED                          ", Q_NULLPTR));
        mlreps->setText(QApplication::translate("MainWindow", "MLREPS                           ", Q_NULLPTR));
        early_write->setText(QApplication::translate("MainWindow", "Early write back            ", Q_NULLPTR));
        early_write_2->setText(QApplication::translate("MainWindow", "Emergency write back", Q_NULLPTR));
        box_parity->clear();
        box_parity->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "none", Q_NULLPTR)
            << QApplication::translate("MainWindow", "ordinary", Q_NULLPTR)
            << QApplication::translate("MainWindow", "cross", Q_NULLPTR)
        );
        box_secded->clear();
        box_secded->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "none", Q_NULLPTR)
            << QApplication::translate("MainWindow", "hamming", Q_NULLPTR)
            << QApplication::translate("MainWindow", "sec-ded", Q_NULLPTR)
        );
        box_mlreps->clear();
        box_mlreps->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "none", Q_NULLPTR)
            << QApplication::translate("MainWindow", "mlreps", Q_NULLPTR)
        );
        box_early->clear();
        box_early->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "none", Q_NULLPTR)
            << QApplication::translate("MainWindow", "use", Q_NULLPTR)
        );
        box_emergency->clear();
        box_emergency->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "none", Q_NULLPTR)
            << QApplication::translate("MainWindow", "use", Q_NULLPTR)
        );
        fault_type->setText(QApplication::translate("MainWindow", "Fault type                       ", Q_NULLPTR));
        box_fault->clear();
        box_fault->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "none", Q_NULLPTR)
            << QApplication::translate("MainWindow", "SBU", Q_NULLPTR)
            << QApplication::translate("MainWindow", "adjacent 2", Q_NULLPTR)
            << QApplication::translate("MainWindow", "adjacent 3", Q_NULLPTR)
            << QApplication::translate("MainWindow", "adjacent 4", Q_NULLPTR)
            << QApplication::translate("MainWindow", "adjacent 5", Q_NULLPTR)
            << QApplication::translate("MainWindow", "random 1-2", Q_NULLPTR)
            << QApplication::translate("MainWindow", "random 1-3", Q_NULLPTR)
            << QApplication::translate("MainWindow", "random 1-4", Q_NULLPTR)
            << QApplication::translate("MainWindow", "random 1-5", Q_NULLPTR)
        );
        inject_times->setText(QApplication::translate("MainWindow", "Inject times                    ", Q_NULLPTR));
        box_inject->clear();
        box_inject->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "10000", Q_NULLPTR)
            << QApplication::translate("MainWindow", "3000", Q_NULLPTR)
            << QApplication::translate("MainWindow", "6000", Q_NULLPTR)
            << QApplication::translate("MainWindow", "9000", Q_NULLPTR)
            << QApplication::translate("MainWindow", "12000", Q_NULLPTR)
            << QApplication::translate("MainWindow", "15000", Q_NULLPTR)
        );
        cache_size->setText(QApplication::translate("MainWindow", "Cache size  ", Q_NULLPTR));
        line_size->setText(QApplication::translate("MainWindow", "Line size     ", Q_NULLPTR));
        ways->setText(QApplication::translate("MainWindow", "Ways           ", Q_NULLPTR));
        box_l1_size->clear();
        box_l1_size->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "4", Q_NULLPTR)
            << QApplication::translate("MainWindow", "8", Q_NULLPTR)
            << QApplication::translate("MainWindow", "16", Q_NULLPTR)
        );
        box_l1_line->clear();
        box_l1_line->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "32", Q_NULLPTR)
            << QApplication::translate("MainWindow", "8", Q_NULLPTR)
            << QApplication::translate("MainWindow", "16", Q_NULLPTR)
            << QApplication::translate("MainWindow", "64", Q_NULLPTR)
        );
        box_l1_way->clear();
        box_l1_way->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "4", Q_NULLPTR)
            << QApplication::translate("MainWindow", "1", Q_NULLPTR)
            << QApplication::translate("MainWindow", "2", Q_NULLPTR)
            << QApplication::translate("MainWindow", "8", Q_NULLPTR)
            << QApplication::translate("MainWindow", "16", Q_NULLPTR)
            << QApplication::translate("MainWindow", "32", Q_NULLPTR)
        );
        label_kb1->setText(QApplication::translate("MainWindow", "KB ", Q_NULLPTR));
        label_b1->setText(QApplication::translate("MainWindow", "B", Q_NULLPTR));
        label_temp1->setText(QString());
        box_l2_size->clear();
        box_l2_size->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "32", Q_NULLPTR)
            << QApplication::translate("MainWindow", "16", Q_NULLPTR)
            << QApplication::translate("MainWindow", "64", Q_NULLPTR)
        );
        box_l2_line->clear();
        box_l2_line->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "32", Q_NULLPTR)
            << QApplication::translate("MainWindow", "8", Q_NULLPTR)
            << QApplication::translate("MainWindow", "16", Q_NULLPTR)
            << QApplication::translate("MainWindow", "64", Q_NULLPTR)
        );
        box_l2_way->clear();
        box_l2_way->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "4", Q_NULLPTR)
            << QApplication::translate("MainWindow", "1", Q_NULLPTR)
            << QApplication::translate("MainWindow", "2", Q_NULLPTR)
            << QApplication::translate("MainWindow", "8", Q_NULLPTR)
            << QApplication::translate("MainWindow", "16", Q_NULLPTR)
            << QApplication::translate("MainWindow", "32", Q_NULLPTR)
        );
        label_kb2->setText(QApplication::translate("MainWindow", "KB", Q_NULLPTR));
        label_b2->setText(QApplication::translate("MainWindow", "B", Q_NULLPTR));
        label_temp2->setText(QString());
        replace->setText(QApplication::translate("MainWindow", "Re-strategy", Q_NULLPTR));
        box_replace1->clear();
        box_replace1->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "LRU", Q_NULLPTR)
            << QApplication::translate("MainWindow", "FIFO", Q_NULLPTR)
            << QApplication::translate("MainWindow", "RAND", Q_NULLPTR)
        );
        box_replace2->clear();
        box_replace2->insertItems(0, QStringList()
            << QApplication::translate("MainWindow", "LRU", Q_NULLPTR)
            << QApplication::translate("MainWindow", "FIFO", Q_NULLPTR)
            << QApplication::translate("MainWindow", "RAND", Q_NULLPTR)
        );
    } // retranslateUi

};

namespace Ui {
    class MainWindow : public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
