#include "commanddisplayer.h"

void CommandDisplayer::handleCommand(BurnInWaitCommand& command) {
    if (command.wait == 1)
        display = "Wait for 1 second";
    else
        display = "Wait for " + QString::number(command.wait) + " seconds";
}

void CommandDisplayer::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    if (command.on)
        display = "Turn on " + command.sourceName;
    else
        display = "Turn off " + command.sourceName;
}

void CommandDisplayer::handleCommand(BurnInVoltageSourceSetCommand& command) {
    display = "Set " + command.sourceName + " to " + QString::number(command.value) + " volts";
}

void CommandDisplayer::handleCommand(BurnInChillerOutputCommand& command) {
    if (command.on)
        display = "Turn chiller output on";
    else
        display = "Turn chiller output off";
}

void CommandDisplayer::handleCommand(BurnInChillerSetCommand& command) {
    display = "Set chiller working temperature to " + QString::number(command.value) + " °C";
}
