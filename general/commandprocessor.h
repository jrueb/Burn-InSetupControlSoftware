#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <QObject>
#include <QVector>
#include "general/systemcontrollerclass.h"

enum BurnInCommandType {
    COMMAND_WAIT,
    COMMAND_VOLTAGESOURCEOUTPUT,
    COMMAND_VOLTAGESOURCESET,
    COMMAND_CHILLEROUTPUT,
    COMMAND_CHILLERSET,
};

class BurnInCommand {
public:
    BurnInCommandType getType() const;
    
protected:
    BurnInCommand(BurnInCommandType type);

private:
    BurnInCommandType _type;
};

class BurnInWaitCommand : public BurnInCommand {
public:
    BurnInWaitCommand(unsigned int wait);

private:
    unsigned int _wait;
};

class BurnInVoltageSourceOutputCommand: public BurnInCommand {
public:
    BurnInVoltageSourceOutputCommand(PowerControlClass* source, int output, bool on);
    
private:
    PowerControlClass* _source;
    int _output;
    bool _on;
};

class BurnInVoltageSourceSetCommand: public BurnInCommand {
public:
    BurnInVoltageSourceSetCommand(PowerControlClass* source, int output, double value);
    
private:
    PowerControlClass* _source;
    int _output;
    double _value;
};

class BurnInChillerOutputCommand : public BurnInCommand {
public:
    BurnInChillerOutputCommand(bool on);

private:
    bool _on;
};

class BurnInChillerSetCommand : public BurnInCommand {
public:
    BurnInChillerSetCommand(double value);

private:
    double _value;
};

class CommandProcessor : public QObject
{
    Q_OBJECT
public:
    explicit CommandProcessor(const SystemControllerClass* controller, QObject *parent = nullptr);
    
    QVector<BurnInCommandType> getAvailableCommands() const;
    std::map<std::string, PowerControlClass*> getAvailableVoltageSources() const;

signals:

public slots:

private:
    const SystemControllerClass* _controller;
};

#endif // COMMANDPROCESSOR_H
