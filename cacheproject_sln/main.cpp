#include "mainwindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QString("Fault Tolerant Cache Simulator"));
    w.show();
    return a.exec();
}
