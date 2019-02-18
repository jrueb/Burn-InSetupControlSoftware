#ifndef DAQMODULE_H
#define DAQMODULE_H

#include "genericinstrumentclass.h"
#include "ComHandler.h"

#include <QObject>
#include <QString>
#include <QVector>
#include <initializer_list>

class DAQModule : public GenericInstrumentClass
{
    Q_OBJECT
    
public:
    DAQModule(const QString& fc7Port, const QString& controlhubPath, const QString& ph2acfPath, const QString& daqHwdescFile, const QString& daqImage);
    ~DAQModule();
    DAQModule(const DAQModule&) = delete;
    DAQModule& operator=(const DAQModule&) = delete;
    
    void initialize();
    
    void setFC7Power(bool power);
    bool getFC7Power() const;
    
    void loadFirmware() const;
    void runSystemTest() const;
    void runCalibrate() const;
    void runDatatest() const;
    void runHybridtest() const;
    void runCmtest() const;
    void runNoiseMeasurement() const;
    
    const int FC7SLEEP = 10000; //us

signals:
    void fc7PowerChanged(bool);
    
private:
    QString _contrStartPath;
    QString _ph2SetupPath;
    QString _ph2SetupCommand;
    QString _ph2FpgaConfigPath;
    QString _ph2SystemtestPath;
    QString _ph2DatatestPath;
    QString _ph2CalibratePath;
    QString _ph2HybridtestPath;
    QString _ph2CmtestPath;
    QString _ph2CommissionPath;
    QString _daqHwdescFile;
    QString _daqImage;
    
    char* _fc7Port;
    ComHandler* _fc7comhandler;
    bool _fc7power;
    
    QString _pathjoin(const std::initializer_list<const QString>& parts) const;
    void _run_ph2_binary(const QString& name, const QString& path, const QVector<QString>& switches = {}) const;
};

#endif // DAQMODULE_H
