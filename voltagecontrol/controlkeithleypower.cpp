
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

KeithleyPowerSweepWorker::KeithleyPowerSweepWorker(ControlKeithleyPower* keithley):
    _keithley(keithley),
    _timer(this)
{
    connect(&_timer, SIGNAL(timeout()), this, SLOT(doSweeping()));
    _timer.start(SWEEP_INTERVAL);
    
    PowerControlClass::fVACvalues *vals = _keithley->getVoltAndCurr();
    _voltTarget = vals->pVSet1;
    _voltApplied = vals->pVApp1;
    delete vals;
    _outputState = _keithley->getKeithleyOutputState();
}

void KeithleyPowerSweepWorker::doSweeping() {
    if (not _outputState or _voltTarget == _voltApplied) {
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
    if (_voltApplied == _voltTarget)
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
    _turnOffScheduled = false;
    keithleyOutputOn = false;
    
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
	throw BurnInException("Device at address of Keithley is not a Keithley 2410");
    
    setCurr(fCurrCompliance);
    
    // check whether output is on
    comHandler_->SendCommand(":OUTPUT1:STATE?");
    usleep(1000);
    comHandler_->ReceiveString(buf);
    
    if (buf[0] == '0')
	keithleyOutputOn = false;
    else {
	double setvolt = fVoltSet;
	keithleyOutputOn = true;
	checkVAC();
	offPower();
	emit outputStateChanged(keithleyOutputOn);
	fVoltSet = setvolt;
    }
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

PowerControlClass::fVACvalues *ControlKeithleyPower::getVoltAndCurr()
{
    PowerControlClass::fVACvalues *cObject = new PowerControlClass::fVACvalues();
    memset(cObject, 0, sizeof(PowerControlClass::fVACvalues));
    
    cObject->pVSet1 = fVoltSet;
    cObject->pISet1 = fCurrCompliance;

    if ( getKeithleyOutputState ( ) )
    {
	checkVAC();

	cObject->pVApp1 = fVolt;
	cObject->pIApp1 = fCurr;
    }

    return cObject;

}

void ControlKeithleyPower::checkVAC()
{
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

    fVolt = fVoltStr.toDouble();

    fCurr = fCurrStr.toDouble();

    emit voltAppChanged(fVolt);
}

void ControlKeithleyPower::closeConnection()
{
    setKeithleyOutputState ( 0 );
}

void ControlKeithleyPower::setKeithleyOutputState ( int outputsetting )
{
    if ( outputsetting == 0 and keithleyOutputOn)
    {
	_commMutex.lock();
	comHandler_->SendCommand(":OUTPUT1:STATE OFF");
	usleep(1000);
	_commMutex.unlock();
	keithleyOutputOn = false;
    }
    else if ( outputsetting == 1 and not keithleyOutputOn)
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

	keithleyOutputOn = true;
    }
    
    emit outputStateChanged(keithleyOutputOn);
}

bool ControlKeithleyPower::getKeithleyOutputState ( )
{
    return ( keithleyOutputOn );
}
