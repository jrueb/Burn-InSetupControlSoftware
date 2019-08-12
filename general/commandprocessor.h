#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QTextStream>
#include "general/systemcontrollerclass.h"
#include "general/burnincommand.h"

class CommandProcessor : public QObject
{
    Q_OBJECT
public:
    explicit CommandProcessor(const SystemControllerClass* controller, QObject *parent = nullptr);
    
    QVector<BurnInCommandType> getAvailableCommands() const;
    static QString getStringForType(BurnInCommandType type);
    
    void saveCommandList(const QVector<BurnInCommand*>& commandList, const QString& filePath) const;
    QString getCommandListAsString(const QVector<BurnInCommand*>& commandList) const;
    QVector<BurnInCommand*> getCommandListFromFile(const QString& filePath) const;
    QVector<BurnInCommand*> getCommandListFromString(const QString& commandString) const;

signals:

public slots:

private:
    const SystemControllerClass* _controller;
    
    QVector<BurnInCommand*> _parseCommands(QTextStream& in) const;
    BurnInWaitCommand* _parseWaitCommand(const QString& line, int line_count) const;
    BurnInVoltageSourceOutputCommand* _parseVoltageSourceOutputCommand(const QString& line, int line_count) const;
    BurnInVoltageSourceSetCommand* _parseVoltageSourceSetCommand(const QString& line, int line_count) const;
    BurnInChillerOutputCommand* _parseChillerOutputCommand(const QString& line, int line_count) const;
    BurnInChillerSetCommand* _parseChillerSetCommand(const QString& line, int line_count) const;
    BurnInDAQCommand* _parseDaqCMDCommand(const QString& line, int line_count) const;
    
    static QString _escapeName(const QString& name);
    static QString _getQuotedString(QTextStream& in);
    GenericInstrumentClass* _parseDeviceName(const QString& devName, int line_count) const;
    PowerControlClass* _parseVoltageSourceName(const QString& devName, int line_count) const;
    Chiller* _parseChillerName(const QString& devName, int line_count) const;
    static int _parseVoltageSourceOutput(QTextStream& in, int line_count, const PowerControlClass* source);
    static bool _parseOnOff(QTextStream& in, int line_count);
    
    class CommandSaver : public AbstractCommandHandler {
    public:
        CommandSaver(QTextStream* out_);
        
        void handleCommand(BurnInWaitCommand& command) override;
        void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
        void handleCommand(BurnInVoltageSourceSetCommand& command) override;
        void handleCommand(BurnInChillerOutputCommand& command) override;
        void handleCommand(BurnInChillerSetCommand& command) override;
        void handleCommand(BurnInDAQCommand& command) override;
        
    private:
        QTextStream* out;
        
    };
};

#endif // COMMANDPROCESSOR_H
