#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QTextStream>
#include "general/systemcontrollerclass.h"
#include "burnincommand.h"

class CommandProcessor : public QObject
{
    Q_OBJECT
public:
    explicit CommandProcessor(const SystemControllerClass* controller, QObject *parent = nullptr);
    
    QVector<BurnInCommandType> getAvailableCommands() const;
    std::map<std::string, PowerControlClass*> getAvailableVoltageSources() const;
    static QString getStringForType(BurnInCommandType type);
    
    void saveCommandList(const QVector<BurnInCommand*>& commandList, const QString& filePath) const;
    QString getCommandListAsString(const QVector<BurnInCommand*>& commandList) const;
    QVector<BurnInCommand*> getCommandListFromFile(const QString& filePath, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources) const;
    QVector<BurnInCommand*> getCommandListFromString(const QString& commandString, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources) const;

signals:

public slots:

private:
    const SystemControllerClass* _controller;
    
    QVector<BurnInCommand*> _parseCommands(QTextStream& in, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources) const;
    BurnInWaitCommand* _parseWaitCommand(const QString& line, int line_count) const;
    BurnInVoltageSourceOutputCommand* _parseVoltageSourceOutputCommand(const QString& line, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, int line_count) const;
    BurnInVoltageSourceSetCommand* _parseVoltageSourceSetCommand(const QString& line, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, int line_count) const;
    BurnInChillerOutputCommand* _parseChillerOutputCommand(const QString& line, int line_count) const;
    BurnInChillerSetCommand* _parseChillerSetCommand(const QString& line, int line_count) const;
    
    static QString _escapeName(const QString& name);
    static QString _getQuotedString(QTextStream& in);
    static bool _parseOnOff(QTextStream& in, int line_count);
    
    class CommandSaver : public AbstractCommandHandler {
    public:
        CommandSaver(QTextStream* out_);
        
        void handleCommand(BurnInWaitCommand& command) override;
        void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
        void handleCommand(BurnInVoltageSourceSetCommand& command) override;
        void handleCommand(BurnInChillerOutputCommand& command) override;
        void handleCommand(BurnInChillerSetCommand& command) override;
        
    private:
        QTextStream* out;
        
    };
};

#endif // COMMANDPROCESSOR_H
