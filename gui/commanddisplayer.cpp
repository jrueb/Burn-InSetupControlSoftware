#include "commanddisplayer.h"

void CommandDisplayer::handleCommand(BurnInWaitCommand& command) {
    if (command.wait == 1)
        display = "Wait for 1 second";
    else
        display = "Wait for " + QString::number(command.wait) + " seconds";
}

void CommandDisplayer::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    QString output_name = "";
    if (command.source->getNumOutputs() > 1)
        output_name = " output no. " + QString::number(command.output);
    if (command.on)
        display = "Turn on source " + command.sourceName + output_name;
    else
        display = "Turn off source " + command.sourceName + output_name;
}

void CommandDisplayer::handleCommand(BurnInVoltageSourceSetCommand& command) {
    QString output_name = "";
    if (command.source->getNumOutputs() > 1)
        output_name = " output no. " + QString::number(command.output);
    display = "Set source " + command.sourceName + output_name + " to " + QString::number(command.value) + " volts";
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
