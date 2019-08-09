#include "burnincommand.h"

#include <QFileInfo> 

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

BurnInChillerOutputCommand::BurnInChillerOutputCommand(Chiller* chiller_, QString chillerName_, bool on_):
    BurnInCommand(COMMAND_CHILLEROUTPUT) {
        
    chiller = chiller_;
    chillerName = chillerName_;
    on = on_;
}

BurnInChillerSetCommand::BurnInChillerSetCommand(Chiller* chiller_, QString chillerName_, double value_):
    BurnInCommand(COMMAND_CHILLERSET) {
        
    chiller = chiller_;
    chillerName = chillerName_;
    value = value_;
}

BurnInDAQCommand::BurnInDAQCommand(QString execName_, QString opts_):
    BurnInCommand(COMMAND_DAQCMD) {
    
    execName = execName_;
    opts = opts_;
}
