#include "commanddisplayer.h"

void CommandDisplayer::handleCommand(BurnInWaitCommand& command) {
    if (command.wait == 1)
        display = "Wait for 1 second";
    else
        display = "Wait for " + QString::number(command.wait) + " seconds";
}

void CommandDisplayer::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    if (command.on)
        display = "Turn on source " + command.sourceName;
    else
        display = "Turn off source " + command.sourceName;
}

void CommandDisplayer::handleCommand(BurnInVoltageSourceSetCommand& command) {
    display = "Set source " + command.sourceName + " to " + QString::number(command.value) + " volts";
}

void CommandDisplayer::handleCommand(BurnInChillerOutputCommand& command) {
    if (command.on)
        display = "Turn chiller " + command.chillerName + " output on";
    else
        display = "Turn chiller " + command.chillerName + " output off";
}

void CommandDisplayer::handleCommand(BurnInChillerSetCommand& command) {
    display = "Set chiller " + command.chillerName + " working temperature to " + QString::number(command.value) + " °C";
}

void CommandDisplayer::handleCommand(BurnInDAQCommand& command) {
    display = "Execute DAQ ACF command " + command.execName + " " + command.opts;
}
