#include "gui/mainwindow.h"
#include "systemcontrollerclass.h"
#include <QApplication>

extern "C" {
	#include "lxi.h"
}

#include <vector>
#include <iostream>
#include <locale>

using namespace std;

int main(int argc, char *argv[])
{
    //replaces commas with dots in printf
    setlocale(LC_ALL,"");
    setlocale(LC_NUMERIC,"");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    lxi_init();

    QApplication a(argc, argv);

    MainWindow w;
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(app_quit()));

    w.show();

    return a.exec();
}
