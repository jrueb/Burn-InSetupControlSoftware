#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(bool enableTermOutput, bool enableColorForTerm);
    
    void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

signals:
    void newMessage(QtMsgType type, const QString& msg);
    void newDebugMessage(const QString& msg);
    void newInfoMessage(const QString& msg);
    void newWarnMessage(const QString& msg);
    void newCriticalMessage(const QString& msg);
    void newFatalMessage(const QString& msg);

public slots:

private:
    bool _enableTermOutput;
    bool _enableColorForTerm;
    
    void _printNoColorTerm(QtMsgType type, const QString& msg);
    void _printColorTerm(QtMsgType type, const QString& msg);
};

#endif // LOGGER_H
