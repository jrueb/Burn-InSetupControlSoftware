#include "general/logger.h"
#include "gui/mainwindow.h"
#include <QApplication>
extern "C" {
	#include "lxi.h"
}
#include <iostream>

Logger logger(true, true);

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    logger.handleMessage(type, context, msg);
}

int main(int argc, char *argv[]) {
    //replaces commas with dots in printf
    setlocale(LC_ALL,"");
    setlocale(LC_NUMERIC,"");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    qRegisterMetaType<QtMsgType>("QtMsgType");
    lxi_init();

    qInstallMessageHandler(messageHandler);
    QApplication a(argc, argv);

    MainWindow w(&logger);
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(app_quit()));

    w.show();

    return a.exec();
}
