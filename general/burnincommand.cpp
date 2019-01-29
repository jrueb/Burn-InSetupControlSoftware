#include "burnincommand.h"

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
