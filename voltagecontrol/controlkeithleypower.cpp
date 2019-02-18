
#include <stdio.h>
#include <sstream>
#include <locale>

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <fstream>
#include <cmath>

#include <QDebug>
#include <QThread>

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
    _voltApplied = _keithley->getVoltApp();
    _outputState = _keithley->getKeithleyOutputState();
}

void KeithleyPowerSweepWorker::doSweeping() {
    if (not _outputState or abs(_voltApplied - _voltTarget) < SWEEP_EPSILON) {
	// No sweeping needed
	
	if (_voltTarget == _voltApplied)
	    emit targetReached(_voltTarget);
	
	_timer.start(SWEEP_INTERVAL);
	return;
    }
    
    if (abs(_voltTarget - _voltApplied) < SWEEP_STEP)
	_voltApplied = _voltTarget;
    else
	_voltApplied += copysign(SWEEP_STEP, _voltTarget - _voltApplied);
    
    _keithley->sendVoltageCommand(_voltApplied);
    if (abs(_voltApplied - _voltTarget) < SWEEP_EPSILON)
	emit targetReached(_voltTarget);
    
    _timer.start(SWEEP_INTERVAL);
}

void KeithleyPowerSweepWorker::doVoltSet(double volts) {
    _voltTarget = volts;
}

void KeithleyPowerSweepWorker::doVoltApp(double volts) {
    _voltApplied = volts;
}

void KeithleyPowerSweepWorker::doOutputState(bool state) {
    _outputState = state;
}

ControlKeithleyPower::ControlKeithleyPower(string pConnection, double pSetVolt, double pSetCurr)
{
    fConnection = pConnection;
    fVoltSet = pSetVolt;
    fCurrCompliance = pSetCurr;
    fVolt = 0;
    fCurr = 0;
    _turnOffScheduled = false;
    _outputOn = false;
    
    KeithleyPowerSweepWorker* worker = new KeithleyPowerSweepWorker(this);
    worker->moveToThread(&_sweepThread);
    connect(&_sweepThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &ControlKeithleyPower::voltSetChanged, worker, &KeithleyPowerSweepWorker::doVoltSet);
    connect(this, &ControlKeithleyPower::voltAppChanged, worker, &KeithleyPowerSweepWorker::doVoltApp);
    connect(this, &ControlKeithleyPower::outputStateChanged, worker, &KeithleyPowerSweepWorker::doOutputState);
    connect(worker, &KeithleyPowerSweepWorker::targetReached, this, &ControlKeithleyPower::onTargetVoltageReached);
    _sweepThread.start();
}

ControlKeithleyPower::~ControlKeithleyPower() {
    _sweepThread.quit();
    _sweepThread.wait();
}

void ControlKeithleyPower::initialize(){

    const ioport_t ioPort = fConnection.c_str();
    speed_t keithleybaud = B19200;
    comHandler_ = new ComHandler( ioPort, keithleybaud );
    
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    comHandler_->SendCommand("*IDN?");
    comHandler_->ReceiveString(buf);
    if (memcmp(buf, "KEITHLEY INSTRUMENTS INC.,MODEL 2410", 36) != 0)
	throw BurnInException("Invalid or no device at address of Keithley 2410");
    
    setCurr(fCurrCompliance);
    
    // check whether output is on
    comHandler_->SendCommand(":OUTPUT1:STATE?");
    usleep(1000);
    comHandler_->ReceiveString(buf);
    
    if (buf[0] == '0')
	_outputOn = false;
    else {
	double setvolt = fVoltSet;
	_outputOn = true;
	refreshAppliedValues();
	offPower();
	emit outputStateChanged(_outputOn);
	fVoltSet = setvolt;
    }
}

bool ControlKeithleyPower::getPower(int) const {
    return not _turnOffScheduled and getKeithleyOutputState();
}

void ControlKeithleyPower::onPower(int)
{
    _turnOffScheduled = false;
    setKeithleyOutputState ( 1 );
}

void ControlKeithleyPower::offPower(int)
{
    _turnOffScheduled = true;
    setVolt(0);
}

void ControlKeithleyPower::setVolt(double pVoltage , int)
{
    fVoltSet = pVoltage;
    emit voltSetChanged(fVoltSet);
}

void ControlKeithleyPower::sendVoltageCommand(double pVoltage) {
    char buf[512];
    sprintf(buf ,":SOUR:VOLT:LEV %G", pVoltage);
    _commMutex.lock();
    comHandler_->SendCommand(buf);
    QThread::msleep(100);
    _commMutex.unlock();
}

void ControlKeithleyPower::onTargetVoltageReached(double voltage) {
    if (_turnOffScheduled and voltage == 0)
	setKeithleyOutputState(0);
}

void ControlKeithleyPower::setCurr(double pCurrent, int)
{
    char stringinput[512];
    
    _commMutex.lock();
    sprintf(stringinput ,":SENS:CURR:PROT %lGE-6" , pCurrent);
    comHandler_->SendCommand(stringinput);
    _commMutex.unlock();
    fCurrCompliance = pCurrent;
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
    if (not getKeithleyOutputState())
	return;
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

    bool changed = fVolt == fVoltStr.toDouble();

    fVolt = fVoltStr.toDouble();

    fCurr = fCurrStr.toDouble();

    if (changed)
	emit voltAppChanged(fVolt);
}

void ControlKeithleyPower::closeConnection()
{
    offPower();
}

void ControlKeithleyPower::setKeithleyOutputState ( int outputsetting )
{
    if (outputsetting == 0 and _outputOn)
    {
	_commMutex.lock();
	comHandler_->SendCommand(":OUTPUT1:STATE OFF");
	usleep(1000);
	_commMutex.unlock();
	_outputOn = false;
    }
    else if (outputsetting == 1 and not _outputOn)
    {
	_commMutex.lock();
	comHandler_->SendCommand(":*RST");
	usleep(1000);

	comHandler_->SendCommand(":OUTPUT1:STATE ON");
	usleep(1000);

	comHandler_->SendCommand(":SOURCE:VOLTAGE:RANGE 1000");
	usleep(1000);
	
	comHandler_->SendCommand(":SENSE:FUNCTION 'CURRENT:DC'");
	usleep(1000);
	_commMutex.unlock();

	_outputOn = true;
    }
    
    emit outputStateChanged(_outputOn);
}

bool ControlKeithleyPower::getKeithleyOutputState() const
{
    return _outputOn;
}
