#include "logger.h"

Logger::Logger(bool enableTermOutput, bool enableColorForTerm) {
    _enableTermOutput = enableTermOutput;
    _enableColorForTerm = enableColorForTerm;
}

void Logger::handleMessage(QtMsgType type, const QMessageLogContext& /* context */, const QString &msg) {
    QString msg_ = msg;
    msg_.replace('\n', "\\n");
    msg_.replace('\r', "\\r");
    msg_.replace('\t', "\\t");
    
    if (_enableTermOutput) {
        if (_enableColorForTerm)
            _printColorTerm(type, msg_);
        else
            _printNoColorTerm(type, msg_);
    }
    
    switch (type) {
    case QtDebugMsg:
        emit newDebugMessage(msg_);
        break;
    case QtInfoMsg:
        emit newInfoMessage(msg_);
        break;
    case QtWarningMsg:
        emit newWarnMessage(msg_);
        break;
    case QtCriticalMsg:
        emit newCriticalMessage(msg_);
        break;
    case QtFatalMsg:
        emit newFatalMessage(msg_);
        break;
    }
    emit newMessage(type, msg_);
}

void Logger::_printNoColorTerm(QtMsgType type, const QString& msg) {
    QByteArray localMsg = msg.toLocal8Bit();
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

void Logger::_printColorTerm(QtMsgType type, const QString& msg) {
    QByteArray localMsg = msg.toLocal8Bit();
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
