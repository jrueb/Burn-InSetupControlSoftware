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
    
    _volt1 = pSetVolt1;
    _curr1 = pSetCurr1;
    _volt2 = pSetVolt2;
    _curr2 = pSetCurr2;
    
    _voltApp1 = 0;
    _currApp1 = 0;
    _voltApp2 = 0;
    _currApp2 = 0;
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

    setVolt(_volt1, 1);
    setCurr(_curr1, 1);
    setVolt(_volt2, 2);
    setCurr(_curr2, 2);
}

void ControlTTiPower::setVolt(double pVoltage , int pId)
{
    Q_ASSERT(pId == 1 or pId == 2);
//    viPrintf(fVi , "V%d %f\n" , pId , pVoltage);
    char cCommand[BUFLEN];
    snprintf(cCommand, sizeof(cCommand), "V%d %.4f\n", pId, pVoltage);
    lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    
    switch (pId) {
    case 1:
        _volt1 = pVoltage;
        break;
    case 2:
        _volt2 = pVoltage;
        break;
    }
}

void ControlTTiPower::setCurr(double pCurrent , int pId)
{
    Q_ASSERT(pId == 1 or pId == 2);
    char cCommand[BUFLEN];
    snprintf(cCommand, sizeof(cCommand), "I%d %.4f\n", pId, pCurrent);
    lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    
    switch (pId) {
    case 1:
        _curr1 = pCurrent;
        break;
    case 2:
        _curr2 = pCurrent;
        break;
    }
}

void ControlTTiPower::onPower(int pId)
{
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    char cCommand[BUFLEN];
    if (pId)
        snprintf(cCommand, sizeof(cCommand), "OP%d 1 \n", pId);
    else
        snprintf(cCommand, sizeof(cCommand), "OPALL 1\n");
    lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
}

void ControlTTiPower::offPower(int pId)
{
    Q_ASSERT(pId == 0 or pId == 1 or pId == 2);
    char cCommand[BUFLEN];
    if(pId){
        snprintf(cCommand, sizeof(cCommand), "OP%d 0 \n", pId);
        lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    }
    else{
        snprintf(cCommand, sizeof(cCommand), "OPALL 0\n");
        lxi_send(fDevice, cCommand, strlen(cCommand), TIMEOUT);
    }
}

bool ControlTTiPower::getPower(int pId) const
{
    Q_ASSERT(pId == 1 or pId == 2);
    char buf[BUFLEN];
    snprintf(buf, sizeof(buf), "OP%d?\n", pId);
    lxi_send(fDevice, buf, strlen(buf), TIMEOUT);
    QThread::msleep(50);
    
    int len;
    if ((len = lxi_receive(fDevice, buf, sizeof(buf), TIMEOUT)) == LXI_ERROR) {
        cerr << "ControlTTiPower::getPower: Could not receive value." << endl;
        
        throw BurnInException("Could not receive TTi power status");
    }
    
    if (buf[0] == '0')
        return false;
    else
        return true;
}

double ControlTTiPower::getVolt(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    switch (pId) {
    case 1:
        return _volt1;
    case 2:
        return _volt2;
    }
    return 0;
}

double ControlTTiPower::getCurr(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    switch (pId) {
    case 1:
        return _curr1;
    case 2:
        return _curr2;
    }
    return 0;
}

double ControlTTiPower::getVoltApp(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    switch (pId) {
    case 1:
        return _voltApp1;
    case 2:
        return _voltApp2;
    }
    return 0;
}

double ControlTTiPower::getCurrApp(int pId) const {
    Q_ASSERT(pId == 1 or pId == 2);
    switch (pId) {
    case 1:
        return _currApp1;
    case 2:
        return _currApp2;
    }
    return 0;
}

void ControlTTiPower::refreshAppliedValues() {
    char cCommand[BUFLEN];
    char cBuff[256];
    
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
    
    cBuff[len] = 0;
    QString res = cBuff;
    QStringList lines = res.split(RMT);
        
    _voltApp1 = lines[0].left(lines[0].length() - 1).toDouble();
    _currApp1 = lines[1].left(lines[1].length() - 1).toDouble();
    _voltApp2 = lines[2].left(lines[2].length() - 1).toDouble();
    _currApp2 = lines[3].left(lines[3].length() - 1).toDouble();
}

void ControlTTiPower::closeConnection()
{
    lxi_disconnect(fDevice);
}
