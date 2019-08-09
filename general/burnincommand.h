#ifndef BURNINCOMMAND_H
#define BURNINCOMMAND_H

#include <QString>

#include "general/systemcontrollerclass.h"
#include "devices/power/powercontrolclass.h"
#include "devices/environment/chiller.h"

/* How to add a new command type:
 * 1. Add a command identifier to the BurnInCommandType enum
 * 2. Add a command class implementing BurnInCommand
 * 3. Fix all compiler errors that pop up
 * 5. Add an else-if-case to CommandProcessor::_parseCommands of commandprocessor.cpp
 * 6. Add the command type to the vector in CommandProcessor::getAvailableCommands() of commandprocessor.cpp
 */

enum BurnInCommandType {
    COMMAND_WAIT,
    COMMAND_VOLTAGESOURCEOUTPUT,
    COMMAND_VOLTAGESOURCESET,
    COMMAND_CHILLEROUTPUT,
    COMMAND_CHILLERSET,
    COMMAND_DAQCMD,
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
class BurnInDAQCommand;

class AbstractCommandHandler {
public:
    virtual void handleCommand(BurnInWaitCommand& command) = 0;
    virtual void handleCommand(BurnInVoltageSourceOutputCommand& command) = 0;
    virtual void handleCommand(BurnInVoltageSourceSetCommand& command) = 0;
    virtual void handleCommand(BurnInChillerOutputCommand& command) = 0;
    virtual void handleCommand(BurnInChillerSetCommand& command) = 0;
    virtual void handleCommand(BurnInDAQCommand& command) = 0;
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
    BurnInChillerOutputCommand(Chiller* chiller_, QString chillerName_, bool on_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    Chiller* chiller;
    QString chillerName;
    bool on;
};

class BurnInChillerSetCommand : public BurnInCommand {
public:
    BurnInChillerSetCommand(Chiller* chiller_, QString chillerName_, double value_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }

    Chiller* chiller;
    QString chillerName;
    double value;
};

// Potentially dangerous: Itended to run DAQ commands but allows
// running any binary the user has access to.
class BurnInDAQCommand : public BurnInCommand {
public:
    BurnInDAQCommand(QString execName_, QString opts_);
    void accept(AbstractCommandHandler& handler) override {
        handler.handleCommand(*this);
    }
    
    QString execName;
    QString opts;
};

#endif // BURNINCOMMAND_H
