#ifndef THERMORASP_H
#define THERMORASP_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QtNetwork/QTcpSocket>
#include <map>

#include "genericinstrumentclass.h"

using namespace std;

class Thermorasp : public QObject, public GenericInstrumentClass {
    Q_OBJECT
public:
    Thermorasp(const QString& address, quint16 port);
    Thermorasp(const string& address, quint16 port);
    void initialize() {};
    QMap<QString, QString> getReadings(int timeout = 5000) const;

signals:

public slots:
private:
    quint16 _port;
    QString _address;
    
    QMap<QString, QString> _parseReplyForReadings(QByteArray buffer) const;
};

#endif // THERMORASP_H
