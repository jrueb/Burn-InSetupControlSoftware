#include "commandprocessor.h"

BurnInCommand::BurnInCommand(BurnInCommandType type) {
    _type = type;
}

BurnInCommandType BurnInCommand::getType() const {
    return _type;
}

BurnInWaitCommand::BurnInWaitCommand(unsigned int wait_):
    BurnInCommand(COMMAND_WAIT) {
    
    wait = wait_;
}

BurnInVoltageSourceOutputCommand::BurnInVoltageSourceOutputCommand(PowerControlClass* source_, QString sourceName_, int output_, bool on_):
    BurnInCommand(COMMAND_VOLTAGESOURCEOUTPUT) {
    
    source = source_;
    sourceName = sourceName_;
    output = output_;
    on = on_;
}

BurnInVoltageSourceSetCommand::BurnInVoltageSourceSetCommand(PowerControlClass* source_, QString sourceName_, int output_, double value_):
    BurnInCommand(COMMAND_VOLTAGESOURCESET) {
    
    source = source_;
    sourceName = sourceName_;
    output = output_;
    value = value_;
}

BurnInChillerOutputCommand::BurnInChillerOutputCommand(bool on_):
    BurnInCommand(COMMAND_CHILLEROUTPUT) {
        
    on = on_;
}

BurnInChillerSetCommand::BurnInChillerSetCommand(double value_):
    BurnInCommand(COMMAND_CHILLERSET) {
        
    value = value_;
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
