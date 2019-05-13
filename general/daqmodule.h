#ifndef DAQMODULE_H
#define DAQMODULE_H

#include "genericinstrumentclass.h"
#include "ComHandler.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <initializer_list>

class DAQModule : public GenericInstrumentClass
{
    Q_OBJECT
    
public:
    DAQModule(const QString& fc7Port, const QString& controlhubPath, const QString& ph2acfPath, const QString& daqHwdescPath, const QString& daqImagePath);
    ~DAQModule();
    
    void initialize();
    
    void setFC7Power(bool power);
    bool getFC7Power() const;
    
    QString getControlhubPath() const;
    QString getACFPath() const;
    QString getHwdescPath() const;
    QString getImagePath() const;
    
    QStringList getAvailableACFBinaries() const;
    
    void loadFirmware() const;
    
    /**
     *  Run a binary from the bin directory of the Ph2_ACF in an extra
     *  terminal window.
     *  execName: Name of the binary to run
     *  switches: Arguments to pass when executing, either separated by
     *            spaces or as seperate strings in a vector
     *  appendHWDesc: If true, append "-f <daqHwdescFile>" to the arguments
     */
    void runACFBinary(const QString& execName, QString switches, bool appendHWDesc) const;
    void runACFBinary(const QString& execName, const QVector<QString>& switches = {}, bool appendHWDesc = true) const;
    
    const int FC7SLEEP = 10000; //us

signals:
    void fc7PowerChanged(bool);
    
private:
    QString _controlhubPath;
    QString _contrStartPath;
    QString _ph2acfPath;
    QString _ph2SetupPath;
    QString _ph2SetupCommand;
    QString _ph2FpgaConfigPath;
    QString _daqHwdescPath;
    QString _daqImagePath;
    
    char* _fc7Port;
    ComHandler* _fc7comhandler;
    bool _fc7power;
    
    QString _pathjoin(const std::initializer_list<const QString>& parts) const;
};

#endif // DAQMODULE_H
