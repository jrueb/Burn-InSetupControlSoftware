#ifndef THERMORASP_H
#define THERMORASP_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QtNetwork/QTcpSocket>

#include <string>
#include <vector>

#include "genericinstrumentclass.h"

using namespace std;

class Thermorasp : public QObject, public GenericInstrumentClass {
    Q_OBJECT
public:
    Thermorasp(const QString& address, quint16 port);
    Thermorasp(const string& address, quint16 port);
    void initialize() {};
    void setSensorNames(const std::vector<std::string>& names);
    void addSensorName(const std::string& name);
    std::vector<std::string> getSensorNames() const;
    QMap<QString, QString> getReadings(int timeout = 5000) const;

signals:

public slots:
private:
    quint16 _port;
    QString _address;
    std::vector<std::string> _sensorNames;
    
    QMap<QString, QString> _parseReplyForReadings(QByteArray buffer) const;
};

#endif // THERMORASP_H
