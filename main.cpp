#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QFile qss(":/Styles/style.qss");
    qss.open(QFile::ReadOnly);
    QString styleSheet { QString(qss.readAll()) };
    a.setStyleSheet(styleSheet);

    w.show();
    return a.exec();
}
