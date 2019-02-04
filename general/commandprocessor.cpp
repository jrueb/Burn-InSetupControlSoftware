#include "commandprocessor.h"
#include "BurnInException.h"
#include <QFile>

CommandProcessor::CommandProcessor(const SystemControllerClass* controller, QObject *parent) : QObject(parent) {
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

void CommandProcessor::saveCommandList(const QVector<BurnInCommand*>& commandList, const QString& filePath) const {
    QFile file(filePath);
    
    if (not file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw BurnInException("Could not write to file");
    QTextStream out(&file);
    CommandSaver saver(&out);
    
    for (const auto& command: commandList)
        command->accept(saver);
        
    file.close();
}

CommandProcessor::CommandSaver::CommandSaver(QTextStream* out_) {
    out = out_;
}

void CommandProcessor::CommandSaver::handleCommand(BurnInWaitCommand& command) {
    *out << "wait " << command.wait << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    *out << "voltageSourceOuput \"" << escapeName(command.sourceName) << "\" " << (command.on ? "on" : "off") << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInVoltageSourceSetCommand& command) {
    *out << "voltageSourceSet \"" << escapeName(command.sourceName) << "\" " << command.value << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInChillerOutputCommand& command) {
    *out << "chillerOutput " << (command.on ? "on" : "off") << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInChillerSetCommand& command) {
    *out << "chillerSet " << command.value << "\n";
}

QString CommandProcessor::CommandSaver::escapeName(const QString& name) const {
    QString ret = name;
    
    // Replace one backslash with two
    ret.replace("\\", "\\\\");
    // Put backshlash in front of quotes
    ret.replace("\"", "\\\"");
    
    return ret;
}
