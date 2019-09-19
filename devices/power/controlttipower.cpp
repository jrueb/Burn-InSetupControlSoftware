#include "controlttipower.h"

#include <QThread>
#include <QDebug>

#include <string>
#include <iostream>

#include "general/BurnInException.h"

using namespace std;

const int TIMEOUT = 1000;
const int BUFLEN = 256;
const char RMT[] = "\r\n";

ControlTTiPower::ControlTTiPower(Communicator* comm) {
    _comm = comm;
    _comm->setSuffix("\n");
    
    for (int i = 0; i < 2; ++i) {
        _power[i] = false;
        _volt[i] = 0;
        _voltApp[i] = 0;
        _curr[i] = 0;
        _currApp[i] = 0;
    }
}

ControlTTiPower::~ControlTTiPower() {
    delete _comm;
}

void ControlTTiPower::initialize() {
    _comm->open();
    
    for (int i = 0; i < 2; ++i) {
        _refreshPowerStatus(i + 1);
        setVolt(_volt[i], i + 1);
        setCurr(_curr[i], i + 1);
    }
}

void ControlTTiPower::setVolt(double pVoltage , int pId) {
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    if (pId == 0) {
        setVolt(pVoltage, 1);
        setVolt(pVoltage, 2);
        return;
    }
    if (_comm->isOpen())
        _comm->send("V" + std::to_string(3 - pId) + " " + std::to_string(pVoltage));
    _volt[pId - 1] = pVoltage;
    
    emit voltSetChanged(pVoltage, pId);
}

void ControlTTiPower::setCurr(double pCurrent , int pId) {
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    if (pId == 0) {
        setCurr(pCurrent, 1);
        setCurr(pCurrent, 2);
        return;
    }
    if (_comm->isOpen())
        _comm->send("I" + std::to_string(3 - pId) + " " + std::to_string(pCurrent));
    _curr[pId - 1] = pCurrent;
    
    emit currSetChanged(pCurrent, pId);
}

void ControlTTiPower::onPower(int pId) {
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    if (pId)
        _comm->send("OP" + std::to_string(3 - pId) + " 1");
    else
        _comm->send("OPALL 1");
    
    if (pId == 0) {
        _power[0] = true;
        _power[1] = true;
        emit powerStateChanged(true, 1);
        emit powerStateChanged(true, 2);
    } else {
        _power[pId - 1] = true;
        emit powerStateChanged(true, pId);
    }
}

void ControlTTiPower::offPower(int pId) {
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    if (pId)
        _comm->send("OP" + std::to_string(3 - pId) + " 0");
    else
        _comm->send("OPALL 0");
    
    if (pId == 0) {
        _power[0] = false;
        _power[1] = false;
        emit powerStateChanged(false, 1);
        emit powerStateChanged(false, 2);
    } else {
        emit powerStateChanged(false, pId);
        _power[pId - 1] = false;
    }
}

bool ControlTTiPower::getPower(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    return _power[pId - 1];
}

void ControlTTiPower::_refreshPowerStatus(int pId)
{
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    if (pId == 0) {
        _refreshPowerStatus(1);
        _refreshPowerStatus(2);
        return;
    }
    std::string buf = _comm->query("OP" + std::to_string(3 - pId) + "?");
    
    bool changed = _power[pId - 1] == (buf[0] == '1');
    _power[pId - 1] = buf[0] == '1';
    
    if (changed)
        emit powerStateChanged(_power[pId - 1], pId);
}

double ControlTTiPower::getVolt(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    return _volt[pId - 1];
}

double ControlTTiPower::getCurr(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    return _curr[pId - 1];
}

double ControlTTiPower::getVoltApp(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    return _voltApp[pId - 1];
}

double ControlTTiPower::getCurrApp(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    return _currApp[pId - 1];
}

void ControlTTiPower::refreshAppliedValues() {
    double voltapp0, currapp0, voltapp1, currapp1;
    try {
        voltapp0 = std::stof(_comm->query("V2?").substr(3));
        currapp0 = std::stof(_comm->query("I2?").substr(3));
        voltapp1 = std::stof(_comm->query("V1?").substr(3));
        currapp1 = std::stof(_comm->query("I1?").substr(3));
    } catch (std::invalid_argument& e) {
        qCritical("Invalid response from TTi at %s", _comm->getLocDisplay().c_str());
        return;
    }
    _setAndEmitIfChanged(&(_voltApp[0]), voltapp0, 1, &ControlTTiPower::voltAppChanged);
    _setAndEmitIfChanged(&(_currApp[0]), currapp0, 1, &ControlTTiPower::currAppChanged);
    _setAndEmitIfChanged(&(_voltApp[1]), voltapp1, 2, &ControlTTiPower::voltAppChanged);
    _setAndEmitIfChanged(&(_currApp[1]), currapp1, 2, &ControlTTiPower::currAppChanged);
}

void ControlTTiPower::_setAndEmitIfChanged(double* target, double val, int id, void (ControlTTiPower::*signal)(double, int)) {
    bool changed = *target != val;
    *target = val;
    if (changed)
        emit (this->*signal)(val, id);
}

void ControlTTiPower::closeConnection() {
    _comm->close();
}
