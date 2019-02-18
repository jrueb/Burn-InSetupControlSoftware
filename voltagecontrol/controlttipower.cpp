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

ControlTTiPower::ControlTTiPower(string pAddress, int pPort, double pSetVolt1, double pSetCurr1, double pSetVolt2, double pSetCurr2)
{
    fAddress = pAddress;
    fPort = pPort;

    fDevice = 0;
    
    for (int i = 0; i < 2; ++i) {
        _power[i] = false;
        _voltApp[i] = 0;
        _currApp[i] = 0;
    }
    
    _volt[0] = pSetVolt1;
    _curr[0] = pSetCurr1;
    _volt[1] = pSetVolt2;
    _curr[1] = pSetCurr2;
}

void ControlTTiPower::initialize()
{
    // Current installed lxi version accepts char* instead of const char*
    // Make a non-const copy as a workaround
    char* pConn = new char[fAddress.length() + 1];
    strcpy(pConn, fAddress.c_str());
    fDevice = lxi_connect(pConn, fPort, NULL, TIMEOUT, RAW);
    delete[] pConn;
    
    if (fDevice == -1) {
        std::cerr << "Could not open ControlTTiPower on " << fAddress << " and port " << fPort << std::endl;
        throw BurnInException("Could not open connection to TTi");
    }
    
    for (int i = 0; i < 2; ++i) {
        _refreshPowerStatus(i + 1);
        setVolt(_volt[i], i + 1);
        setCurr(_curr[i], i + 1);
    }
}

void ControlTTiPower::setVolt(double pVoltage , int pId)
{
    Q_ASSERT(pId == 1 or pId == 2);
//    viPrintf(fVi , "V%d %f\n" , pId , pVoltage);
    char cCommand[BUFLEN];
    snprintf(cCommand, sizeof(cCommand), "V%d %.4f\n", pId, pVoltage);
    _commMutex.lock();
    lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    _commMutex.unlock();
    
    _volt[pId - 1] = pVoltage;
}

void ControlTTiPower::setCurr(double pCurrent , int pId)
{
    Q_ASSERT(pId == 1 or pId == 2);
    char cCommand[BUFLEN];
    snprintf(cCommand, sizeof(cCommand), "I%d %.4f\n", pId, pCurrent);
    _commMutex.lock();
    lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    _commMutex.unlock();
    
    _curr[pId - 1] = pCurrent;
}

void ControlTTiPower::onPower(int pId)
{
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    char cCommand[BUFLEN];
    if (pId)
        snprintf(cCommand, sizeof(cCommand), "OP%d 1 \n", pId);
    else
        snprintf(cCommand, sizeof(cCommand), "OPALL 1\n");
    _commMutex.lock();
    lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    _commMutex.unlock();
}

void ControlTTiPower::offPower(int pId)
{
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    char cCommand[BUFLEN];
    _commMutex.lock();
    if(pId){
        snprintf(cCommand, sizeof(cCommand), "OP%d 0 \n", pId);
        lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    }
    else{
        snprintf(cCommand, sizeof(cCommand), "OPALL 0\n");
        lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    }
    _commMutex.unlock();
}

bool ControlTTiPower::getPower(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    return _power[pId - 1];
}

void ControlTTiPower::_refreshPowerStatus(int pId)
{
    Q_ASSERT(pId == 1 or pId == 2);
    char buf[BUFLEN];
    snprintf(buf, sizeof(buf), "OP%d?\n", pId);
    _commMutex.lock();
    lxi_send(fDevice, buf, strlen(buf), TIMEOUT);
    QThread::msleep(50);
    
    int len;
    if ((len = lxi_receive(fDevice, buf, sizeof(buf), TIMEOUT)) == LXI_ERROR) {
        cerr << "ControlTTiPower::_refreshPowerStatus: Could not receive value." << endl;
        
        throw BurnInException("Could not receive TTi power status");
    }
    _commMutex.unlock();
    
    _power[pId - 1] = buf[0] == '1';
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
    char cCommand[BUFLEN];
    char cBuff[256];
    
    _commMutex.lock();
    for (int i = 1; i <= 2; i++) {
        snprintf(cCommand, sizeof(cCommand), "V%dO? \n", i);
        lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
        snprintf(cCommand, sizeof(cCommand), "I%dO? \n", i);
        lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    }
    QThread::msleep(50);
    
    int len;
    if ((len = lxi_receive(fDevice, cBuff, sizeof(cBuff), TIMEOUT)) == LXI_ERROR) {
        cerr << "ControlTTiPower::refreshAppliedValues: Could not receive values." << endl;
        return;
    }
    _commMutex.unlock();
    
    cBuff[len] = 0;
    QString res = cBuff;
    QStringList lines = res.split(RMT);
        
    _voltApp[0] = lines[0].left(lines[0].length() - 1).toDouble();
    _currApp[0] = lines[1].left(lines[1].length() - 1).toDouble();
    _voltApp[1] = lines[2].left(lines[2].length() - 1).toDouble();
    _currApp[1] = lines[3].left(lines[3].length() - 1).toDouble();
}

void ControlTTiPower::closeConnection()
{
    lxi_disconnect(fDevice);
}
