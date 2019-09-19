#include "kepco.h"
#include "general/BurnInException.h"

Kepco::Kepco(Communicator* comm) {
    _comm = comm;
    _comm->setSuffix("\n");
    _volt = _voltApp = _curr = _currApp = 0;
    _outputOn = false;
}

Kepco::~Kepco() {
    delete _comm;
}

void Kepco::initialize() {
    _comm->open();
    
    std::string idn = _comm->query("*IDN?", 1000);
    std::string expected_idn_prefix = "KEPCO,KLP 36-60";
    if (idn.compare(0, expected_idn_prefix.size(), expected_idn_prefix) != 0)
        throw BurnInException("Invalid or no device at address of Kepco");
    
    
    setVolt(_volt);
    setCurr(_curr);
    _outputOn = _comm->query("OUTP?") == "1";
}

double Kepco::getVolt(int) const {
    return _volt;
}
double Kepco::getVoltApp(int) const {
    return _voltApp;
}
double Kepco::getCurr(int) const {
    return _curr;
}
double Kepco::getCurrApp(int) const {
    return _currApp;
}

void Kepco::setVolt(double volt, int) {
	if (_comm->isOpen())
		_comm->send(std::string("VOLT ") + to_string(volt));
    _volt = volt;
    
    emit voltSetChanged(_volt, 1);
}

void Kepco::setCurr(double curr, int) {
	if (_comm->isOpen())
		_comm->send(std::string("CURR ") + to_string(curr));
    _curr = curr;
    
    emit currSetChanged(_curr, 1);
}

bool Kepco::getPower(int) const {
    return _outputOn;
}

void Kepco::onPower(int) {
    _comm->send("OUTP ON");
    _outputOn = true;
    emit powerStateChanged(true, 1);
}

void Kepco::offPower(int) {
    _comm->send("OUTP OFF");
    _outputOn = false;
    emit powerStateChanged(false, 1);
}

void Kepco::closeConnection() {
    _comm->close();
}

void Kepco::refreshAppliedValues() {
    double voltapp, currapp;
    try {
        voltapp = std::stof(_comm->query("MEAS:VOLT?"));
        currapp = std::stof(_comm->query("MEAS:CURR?"));
    } catch (std::invalid_argument& e) {
        qCritical("Invalid response from Kepco at %s", _comm->getLocDisplay().c_str());
        return;
    }
    _setAndEmitIfChanged(&_voltApp, voltapp, &Kepco::voltAppChanged);
    _setAndEmitIfChanged(&_currApp, currapp, &Kepco::voltAppChanged);
}

void Kepco::_setAndEmitIfChanged(double* target, double val, void (Kepco::*signal)(double, int)) {
    bool changed = *target != val;
    *target = val;
    if (changed)
        emit (this->*signal)(val, 1);
}
