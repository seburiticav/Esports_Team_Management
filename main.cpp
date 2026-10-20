#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/app_icon.png"));
    MainWindow w;
    w.show();
    return QApplication::exec();
}
