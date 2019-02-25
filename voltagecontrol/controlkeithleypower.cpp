#include <iostream>
#include <string>
#include <cstdlib>
#include <utility>
#include <cmath>

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QEventLoop>

#include "controlkeithleypower.h"
#include "general/BurnInException.h"
#include "additional/additionalthread.h"
#include "general/systemcontrollerclass.h"

using namespace std;

const int SWEEP_INTERVAL = 500; //ms
const double SWEEP_STEP = 10; //V
const double SWEEP_EPSILON = 0.00001; //V

KeithleyPowerSweepWorker::KeithleyPowerSweepWorker(ControlKeithleyPower* keithley):
    _keithley(keithley),
    _timer(this)
{
    connect(&_timer, SIGNAL(timeout()), this, SLOT(doSweeping()));
    _timer.start(SWEEP_INTERVAL);
    
    _voltTarget = _keithley->getVolt();
    _voltApplied = _voltTarget;
    _outputStateTarget = _keithley->getPower();
    _outputStateApplied = _outputStateTarget;
}

void KeithleyPowerSweepWorker::doSweeping() {
    if (not _outputStateTarget and not _outputStateApplied) {
	emit shutdownSafe();
	return;
    } else if (_outputStateTarget and not _outputStateApplied) {
	_keithley->sendVoltageCommand(0);
	_voltApplied = 0;
	_keithley->sendOutputStateCommand(true);
	_outputStateApplied = true;
    }
    
    double voltTarget = _outputStateTarget ? _voltTarget : 0;
	
    if (abs(voltTarget - _voltApplied) > SWEEP_EPSILON) {
	if (abs(voltTarget - _voltApplied) < SWEEP_STEP)
	    _voltApplied = voltTarget;
	else
	    _voltApplied += copysign(SWEEP_STEP, voltTarget - _voltApplied);
	_keithley->sendVoltageCommand(_voltApplied);
    }
    
    if (not _outputStateTarget and _voltApplied == 0) {
	_keithley->sendOutputStateCommand(false);
	_outputStateApplied = false;
    }
}

void KeithleyPowerSweepWorker::doVoltSet(double volts) {
    _voltTarget = volts;
}

void KeithleyPowerSweepWorker::doOutputState(bool state) {
    _outputStateTarget = state;
}

void KeithleyPowerSweepWorker::doDeviceStateChanged(bool state, double volt) {
    _outputStateApplied = state;
    _voltApplied = volt;
}

ControlKeithleyPower::ControlKeithleyPower(string pConnection, double pSetVolt, double pSetCurr)
{
    comHandler_ = nullptr;
    fConnection = pConnection;
    fVoltSet = pSetVolt;
    fCurrCompliance = pSetCurr;
    fVolt = 0;
    fCurr = 0;
    _outputOn = false;
    
    _worker = new KeithleyPowerSweepWorker(this);
    _worker->moveToThread(&_sweepThread);
    connect(&_sweepThread, &QThread::finished, _worker, &QObject::deleteLater);
    connect(this, &ControlKeithleyPower::voltSetChanged, _worker, &KeithleyPowerSweepWorker::doVoltSet);
    connect(this, &ControlKeithleyPower::powerStateChanged, _worker, &KeithleyPowerSweepWorker::doOutputState);
    connect(this, &ControlKeithleyPower::deviceStateChanged, _worker, &KeithleyPowerSweepWorker::doDeviceStateChanged);
    _sweepThread.start();
}

ControlKeithleyPower::~ControlKeithleyPower() {
    _sweepThread.quit();
    _sweepThread.wait();
}

void ControlKeithleyPower::initialize(){
    
    Q_ASSERT(comHandler_ == nullptr);
    comHandler_ = new ComHandler(fConnection.c_str(), B19200);
    _outputOn = false;
    
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    _commMutex.lock();
    comHandler_->SendCommand("*IDN?");
    comHandler_->ReceiveString(buf);
    _commMutex.unlock();
    if (memcmp(buf, "KEITHLEY INSTRUMENTS INC.,MODEL 2410", 36) != 0)
	throw BurnInException("Invalid or no device at address of Keithley 2410");
    
    setCurr(fCurrCompliance);
    setVolt(fVoltSet);
    
    // check whether output is on
    _commMutex.lock();
    comHandler_->SendCommand(":OUTPUT1:STATE?");
    usleep(1000);
    comHandler_->ReceiveString(buf);
    _commMutex.unlock();
    
    if (buf[0] == '1') {
	cout << "Keithley output was on during initialization. Turning off" << endl;
	refreshAppliedValues();
	emit deviceStateChanged(true, fVolt);
    }
}

bool ControlKeithleyPower::getPower(int) const {
    return _outputOn;
}

void ControlKeithleyPower::onPower(int)
{
    _outputOn = true;
    emit powerStateChanged(true, 0);
}

void ControlKeithleyPower::offPower(int)
{
    _outputOn = false;
    emit powerStateChanged(false, 0);
}

void ControlKeithleyPower::setVolt(double pVoltage , int)
{
    fVoltSet = pVoltage;
    emit voltSetChanged(fVoltSet, 0);
}

void ControlKeithleyPower::sendVoltageCommand(double pVoltage) {
    char buf[512];
    sprintf(buf ,":SOUR:VOLT:LEV %G", pVoltage);
    _commMutex.lock();
    comHandler_->SendCommand(buf);
    QThread::msleep(100);
    _commMutex.unlock();
}

void ControlKeithleyPower::sendOutputStateCommand(bool on) {
    _commMutex.lock();
    if (on) {
	comHandler_->SendCommand(":*RST");
	QThread::usleep(1000);
	
	comHandler_->SendCommand(":OUTPUT1:STATE ON");
	QThread::usleep(1000);

	comHandler_->SendCommand(":SOURCE:VOLTAGE:RANGE 1000");
	QThread::usleep(1000);
	
	comHandler_->SendCommand(":SENSE:FUNCTION 'CURRENT:DC'");
	QThread::usleep(1000);
    } else {
	comHandler_->SendCommand(":OUTPUT1:STATE OFF");
	QThread::msleep(1000);
    }
    _commMutex.unlock();
}

void ControlKeithleyPower::setCurr(double pCurrent, int)
{
    char stringinput[512];
    
    _commMutex.lock();
    sprintf(stringinput ,":SENS:CURR:PROT %lGE-6" , pCurrent);
    comHandler_->SendCommand(stringinput);
    _commMutex.unlock();
    fCurrCompliance = pCurrent;
    emit currSetChanged(fCurrCompliance, 0);
}

double ControlKeithleyPower::getVolt(int) const {
    return fVoltSet;
}

double ControlKeithleyPower::getVoltApp(int) const {
    return fVolt;
}

double ControlKeithleyPower::getCurr(int) const {
    return fCurrCompliance;
}

double ControlKeithleyPower::getCurrApp(int) const {
    return fCurr;
}

void ControlKeithleyPower::refreshAppliedValues()
{
    if (not _outputOn) {
	if (fVolt != 0) {
	    fVolt = 0;
	    emit voltAppChanged(fVolt, 0);
	}
	if (fCurr != 0) {
	    fCurr = 0;
	    emit voltAppChanged(fCurr, 0);
	}
	return;
    }
    char buffer[1024];
    buffer[0] = 0;

    _commMutex.lock();
    comHandler_->SendCommand(":READ?");
    QThread::msleep(500);

    comHandler_->ReceiveString(buffer);
    _commMutex.unlock();
    
    string str(buffer);
    size_t cPos = str.find(',');

    QString fVoltStr = QString::fromStdString(str.substr(0 , cPos));
    str = str.substr(cPos+1, cPos + 13);
    QString fCurrStr = QString::fromStdString(str.substr(0 , 13));

    bool voltchanged = fVolt != fVoltStr.toDouble();
    bool currchanged = fCurr != fCurrStr.toDouble();

    fVolt = fVoltStr.toDouble();
    fCurr = fCurrStr.toDouble();

    if (voltchanged)
	emit voltAppChanged(fVolt, 0);
    if (currchanged)
	emit currAppChanged(fCurr, 0);
}

void ControlKeithleyPower::closeConnection()
{
    offPower();
}

void ControlKeithleyPower::waitForSafeShutdown() {
    Q_ASSERT(_outputOn == false);
    QEventLoop loop;
    connect(_worker, SIGNAL(shutdownSafe()), &loop, SLOT(quit()));
    loop.exec();
}
