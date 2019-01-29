#include "commandprocessor.h"

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
