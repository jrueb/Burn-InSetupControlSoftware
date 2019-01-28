#include "commandprocessor.h"

BurnInCommand::BurnInCommand(BurnInCommandType type) {
    _type = type;
}

BurnInCommandType BurnInCommand::getType() const {
    return _type;
}

BurnInWaitCommand::BurnInWaitCommand(unsigned int wait):
    BurnInCommand(COMMAND_WAIT) {
    
    _wait = wait;
}

BurnInVoltageSourceOutputCommand::BurnInVoltageSourceOutputCommand(PowerControlClass* source, int output, bool on):
    BurnInCommand(COMMAND_VOLTAGESOURCEOUTPUT) {
    
    _source = source;
    _output = output;
    _on = on;
}

BurnInVoltageSourceSetCommand::BurnInVoltageSourceSetCommand(PowerControlClass* source, int output, double value):
    BurnInCommand(COMMAND_VOLTAGESOURCESET) {
    
    _source = source;
    _output = output;
    _value = value;
}

BurnInChillerOutputCommand::BurnInChillerOutputCommand(bool on):
    BurnInCommand(COMMAND_CHILLEROUTPUT) {
        
    _on = on;
}

BurnInChillerSetCommand::BurnInChillerSetCommand(double value):
    BurnInCommand(COMMAND_CHILLERSET) {
        
    _value = value;
}

CommandProcessor::CommandProcessor(const SystemControllerClass* controller, QObject *parent) : QObject(parent)
{
    _controller = controller;
}

QVector<BurnInCommandType> CommandProcessor::getAvailableCommands() const {
    QVector<BurnInCommandType> avail;
    
    avail.push_back(COMMAND_WAIT);
    
    if (_controller->getNumVoltageSources() > 0) {
        avail.push_back(COMMAND_VOLTAGESOURCEOUTPUT);
        avail.push_back(COMMAND_VOLTAGESOURCESET);
    }
        
    if (_controller->countInstrument("JulaboFP50") > 0) {
        avail.push_back(COMMAND_CHILLEROUTPUT);
        avail.push_back(COMMAND_CHILLERSET);
    }
        
    return avail;
}

std::map<std::string, PowerControlClass*> CommandProcessor::getAvailableVoltageSources() const {
    return _controller->getVoltageSources();
}
