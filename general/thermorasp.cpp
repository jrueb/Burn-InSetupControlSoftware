#include "thermorasp.h"

#include <iostream>
#include <QTextCodec>
#include <QThread>
#include <QRegularExpression>

using namespace std;

Thermorasp::Thermorasp(const QString& address, quint16 port) {
    _address = address;
    _port = port;
}

Thermorasp::Thermorasp(const string& address, quint16 port) {
    _address = QString::fromStdString(address);
    _port = port;
}

void Thermorasp::setSensorNames(const std::vector<std::string>& names) {
    _sensorNames = names;
}

void Thermorasp::addSensorName(const std::string& name) {
    _sensorNames.push_back(name);
}

std::vector<std::string> Thermorasp::getSensorNames() const {
    return _sensorNames;
}

QMap<QString, QString> Thermorasp::_parseReplyForReadings(QByteArray buffer) const {
    QMap<QString, QString> ret;
    QString data = QTextCodec::codecForMib(106)->toUnicode(buffer);
    int nlpos = data.indexOf('\n');
    if (nlpos < 1)
        return ret;
    QString nameline = data.mid(1, nlpos - 1);
    QString readingsline = data.right(data.length() - (nlpos + 1));
    QStringList names = nameline.split(' ');
    
    QString pattern("^");
    for (const QString& name: names) {
        if (pattern != "^")
            pattern += " ";
        
        if (name == "date")
            pattern += "(\\d{4}-\\d{2}-\\d{2})";
        else if (name == "time")
            pattern += "(\\d{2}:\\d{2}:\\d{2}\\.\\d+)";
        else
            //pattern += "(\\d+\\.?\\d*?)?"; // expect float
            pattern += "([^ ]+)?"; // except anything
    }
    pattern += "$";
    QRegularExpression re(pattern);
    QRegularExpressionMatch match = re.match(readingsline);
    
    if (not match.hasMatch()) {
        qCritical("Raspberry sent line with invalid format: '%s'", readingsline.toLatin1().data());
        return ret;
    }
    
    for (int i = 0; i < names.length(); ++i)
        ret[names[i]] = match.captured(i + 1);
        
    return ret;
}

QMap<QString, QString> Thermorasp::getLastReadings() const {
    return _lastReadings;
}

QMap<QString, QString> Thermorasp::fetchReadings(int timeout) {
    QTcpSocket* sock = new QTcpSocket(nullptr);
    QMap<QString, QString> ret;
    sock->connectToHost(_address, _port);
    if (not sock->waitForDisconnected(timeout)) {
        qInfo("Thermorasp at %s didn't answer", _address.toLatin1().data());
        return ret;
    }
    sock->waitForReadyRead(timeout);
    
    QByteArray buffer = sock->readAll();
    ret = _parseReplyForReadings(buffer);
    
    sock->close();
    delete sock;
    
    _lastReadings = ret;
    emit gotNewReadings(_lastReadings);
    return ret;
}
