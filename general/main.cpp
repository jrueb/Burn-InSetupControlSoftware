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

const bool COLOR_TERM_MESSAGES = true;

void messageHandler(QtMsgType type, const QMessageLogContext& /* context */, const QString& msg)
{
    QString msg_ = msg;
    msg_.replace('\n', "\\n");
    msg_.replace('\r', "\\r");
    msg_.replace('\t', "\\t");
    QByteArray localMsg = msg_.toLocal8Bit();
    //const char *file = context.file ? context.file : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", localMsg.constData());
        break;
    }
}

void messageHandlerColor(QtMsgType type, const QMessageLogContext& /* context */, const QString& msg)
{
    QString msg_ = msg;
    msg_.replace('\n', "\\n");
    msg_.replace('\r', "\\r");
    msg_.replace('\t', "\\t");
    QByteArray localMsg = msg_.toLocal8Bit();
    //const char *file = context.file ? context.file : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "\033[90mDebug: %s\033[0m\n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "\033[37mInfo: %s\033[0m\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "\033[91mWarning: %s\033[0m\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "\033[31mCritical: %s\033[0m\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "\033[31mFatal: %s\033[0m\n", localMsg.constData());
        break;
    }
}

int main(int argc, char *argv[])
{
    //replaces commas with dots in printf
    setlocale(LC_ALL,"");
    setlocale(LC_NUMERIC,"");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    lxi_init();

    qInstallMessageHandler(COLOR_TERM_MESSAGES ? messageHandlerColor : messageHandler);
    QApplication a(argc, argv);

    MainWindow w;
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(app_quit()));

    w.show();

    return a.exec();
}
