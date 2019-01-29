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

class AbstractCommandHandler;

class BurnInCommand {
public:
    BurnInCommandType getType() const;
    virtual void accept(AbstractCommandHandler& handler) = 0;
    
protected:
    BurnInCommand(BurnInCommandType type);

private:
    BurnInCommandType _type;
};

class BurnInWaitCommand;
class BurnInVoltageSourceOutputCommand;
class BurnInVoltageSourceSetCommand;
class BurnInChillerOutputCommand;
class BurnInChillerSetCommand;

class AbstractCommandHandler {
public:
    virtual void handleCommand(BurnInWaitCommand& command) = 0;
    virtual void handleCommand(BurnInVoltageSourceOutputCommand& command) = 0;
    virtual void handleCommand(BurnInVoltageSourceSetCommand& command) = 0;
    virtual void handleCommand(BurnInChillerOutputCommand& command) = 0;
    virtual void handleCommand(BurnInChillerSetCommand& command) = 0;
};

class BurnInWaitCommand : public BurnInCommand {
public:
    BurnInWaitCommand(unsigned int wait_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    unsigned int wait;
};

class BurnInVoltageSourceOutputCommand: public BurnInCommand {
public:
    BurnInVoltageSourceOutputCommand(PowerControlClass* source_, QString sourceName_, int output_, bool on_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    PowerControlClass* source;
    QString sourceName;
    int output;
    bool on;
};

class BurnInVoltageSourceSetCommand: public BurnInCommand {
public:
    BurnInVoltageSourceSetCommand(PowerControlClass* source_, QString sourceName_, int output_, double value_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    PowerControlClass* source;
    QString sourceName;
    int output;
    double value;
};

class BurnInChillerOutputCommand : public BurnInCommand {
public:
    BurnInChillerOutputCommand(bool on_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    bool on;
};

class BurnInChillerSetCommand : public BurnInCommand {
public:
    BurnInChillerSetCommand(double value_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    double value;
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
