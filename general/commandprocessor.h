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
    
    void saveCommandList(const QVector<BurnInCommand*>& commandList, const QString& filePath) const;

signals:

public slots:

private:
    const SystemControllerClass* _controller;
    
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
        
        QString escapeName(const QString& name) const;
    };
};

#endif // COMMANDPROCESSOR_H
