#include "commandprocessor.h"
#include "BurnInException.h"
#include <QFile>
#include <QChar>
#include <cmath>
#include "JulaboFP50.h"

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
    
    if (_controller->getDaqModule() != nullptr)
        avail.push_back(COMMAND_DAQCMD);
        
    return avail;
}

QString CommandProcessor::getStringForType(BurnInCommandType type) {
    switch(type) {
    case COMMAND_WAIT:
        return "wait";
        break;
    case COMMAND_VOLTAGESOURCEOUTPUT:
        return "voltageSourceOuput";
        break;
    case COMMAND_VOLTAGESOURCESET:
        return "voltageSourceSet";
        break;
    case COMMAND_CHILLEROUTPUT:
        return "chillerOutput";
        break;
    case COMMAND_CHILLERSET:
        return "chillerSet";
        break;
    case COMMAND_DAQCMD:
        return "daqcmd";
        break;
    }
    
    Q_ASSERT(false); // Should not reach.
    return "";
}

void CommandProcessor::saveCommandList(const QVector<BurnInCommand*>& commandList, const QString& filePath) const {
    QFile file(filePath);
    
    if (not file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw BurnInException("Could not open file");
    QTextStream out(&file);
    CommandSaver saver(&out);
    
    for (const auto& command: commandList)
        command->accept(saver);
        
    file.close();
}

QString CommandProcessor::getCommandListAsString(const QVector<BurnInCommand*>& commandList) const {
    QString s;
    QTextStream out(&s);
    CommandSaver saver(&out);
    
    for (const auto& command: commandList)
        command->accept(saver);
        
    return s;
}

CommandProcessor::CommandSaver::CommandSaver(QTextStream* out_) {
    out = out_;
}

void CommandProcessor::CommandSaver::handleCommand(BurnInWaitCommand& command) {
    *out << getStringForType(COMMAND_WAIT) << " " << command.wait << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    *out << getStringForType(COMMAND_VOLTAGESOURCEOUTPUT)
         << " \"" << CommandProcessor::_escapeName(command.sourceName) << "\" "
         << (command.on ? "on" : "off")
         << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInVoltageSourceSetCommand& command) {
    *out << getStringForType(COMMAND_VOLTAGESOURCESET)
         << " \"" << CommandProcessor::_escapeName(command.sourceName) << "\" "
         << command.value
         << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInChillerOutputCommand& command) {
    *out << getStringForType(COMMAND_CHILLEROUTPUT) << " " << (command.on ? "on" : "off") << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInChillerSetCommand& command) {
    *out << getStringForType(COMMAND_CHILLERSET) << " " << command.value << "\n";
}

void CommandProcessor::CommandSaver::handleCommand(BurnInDAQCommand& command) {
    *out << getStringForType(COMMAND_DAQCMD)
         << " \"" << CommandProcessor::_escapeName(command.execName) << "\""
         << " \"" << CommandProcessor::_escapeName(command.opts) << "\""
         << "\n";
}

QString CommandProcessor::_escapeName(const QString& name) {
    QString ret = name;
    
    // Replace one backslash with two
    ret.replace("\\", "\\\\");
    // Put backshlash in front of quotes
    ret.replace("\"", "\\\"");
    
    return ret;
}

QVector<BurnInCommand*> CommandProcessor::getCommandListFromFile(const QString& filePath, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, const QStringList& daqExecuteables) const {
    QFile file(filePath);
    
    if (not file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw BurnInException("Could not open file");
    QTextStream in(&file);
    QVector<BurnInCommand*> list = _parseCommands(in, voltageSources, daqExecuteables);
    
    file.close();
    
    return list;
}

QVector<BurnInCommand*> CommandProcessor::getCommandListFromString(const QString& commandString, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, const QStringList& daqExecuteables) const {
    QString cpy = commandString;
    QTextStream in(&cpy);
    return _parseCommands(in, voltageSources, daqExecuteables);
}

QVector<BurnInCommand*> CommandProcessor::_parseCommands(QTextStream& in, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, const QStringList& daqExecuteables) const {
    QVector<BurnInCommand*> list;
    
    int line_count = 0;
    while (not in.atEnd()) {
        ++line_count;
        QString line = in.readLine();
        
        if (line.startsWith("#") or line.isEmpty())
            continue; // Comments and empty lines allowed
        else if (line.startsWith(getStringForType(COMMAND_WAIT) + " ")) {
            list.push_back(_parseWaitCommand(line, line_count));
            
        } else if (line.startsWith(getStringForType(COMMAND_VOLTAGESOURCEOUTPUT) + " ")) {
            list.push_back(_parseVoltageSourceOutputCommand(line, voltageSources, line_count));
            
        } else if (line.startsWith(getStringForType(COMMAND_VOLTAGESOURCESET) + " ")) {
            list.push_back(_parseVoltageSourceSetCommand(line, voltageSources, line_count));
            
        } else if (line.startsWith(getStringForType(COMMAND_CHILLEROUTPUT) + " ")) {
            list.push_back(_parseChillerOutputCommand(line, line_count));
            
        } else if (line.startsWith(getStringForType(COMMAND_CHILLERSET) + " ")) {
            list.push_back(_parseChillerSetCommand(line, line_count));
            
        } else if (line.startsWith(getStringForType(COMMAND_DAQCMD) + " ")) {
            list.push_back(_parseDaqCMDCommand(line, daqExecuteables, line_count));
        } else {
            QTextStream line_stream(&line);
            QString cmd;
            line_stream >> cmd;
            throw BurnInException("Line " + std::to_string(line_count) + ": Unknown command or missing arguments \"" + cmd.toStdString() + "\"");
        }
    }
    
    return list;
}

BurnInWaitCommand* CommandProcessor::_parseWaitCommand(const QString& line, int line_count) const {
    int cmdlen = getStringForType(COMMAND_WAIT).length();
    QString args = line.right(line.length() - cmdlen - 1);
    QTextStream line_stream(&args);
    
    QString wait_str;
    unsigned int wait;
    bool ok;
    
    line_stream >> wait_str;
    wait = wait_str.toUInt(&ok);
    if (not ok)
        throw BurnInException("Line " + std::to_string(line_count) + ": Invalid wait value \"" + wait_str.toStdString() + "\"");
    
    return new BurnInWaitCommand(wait);
}

BurnInVoltageSourceOutputCommand* CommandProcessor::_parseVoltageSourceOutputCommand(const QString& line, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, int line_count) const {
    int cmdlen = getStringForType(COMMAND_VOLTAGESOURCEOUTPUT).length();
    QString args = line.right(line.length() - cmdlen - 1);
    QTextStream line_stream(&args);
    
    QString sourceName;
    int output;
    PowerControlClass* dev;
    bool on;
    
    sourceName = _getQuotedString(line_stream);
    if (not voltageSources.contains(sourceName))
        throw BurnInException("Line " + std::to_string(line_count) + ": Unknown voltage source \"" + sourceName.toStdString() + "\"");
    auto output_control_pair = voltageSources[sourceName];
    output = output_control_pair.first;
    dev = output_control_pair.second;
    
    on = _parseOnOff(line_stream, line_count);
    
    return new BurnInVoltageSourceOutputCommand(dev, sourceName, output, on);
}

BurnInVoltageSourceSetCommand* CommandProcessor::_parseVoltageSourceSetCommand(const QString& line, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, int line_count) const {
    int cmdlen = getStringForType(COMMAND_VOLTAGESOURCESET).length();
    QString args = line.right(line.length() - cmdlen - 1);
    QTextStream line_stream(&args);
    
    QString sourceName;
    int output;
    PowerControlClass* dev;
    QString on_str;
    QString value_str;
    bool ok;
    double value;
    
    sourceName = _getQuotedString(line_stream);
    if (not voltageSources.contains(sourceName))
        throw BurnInException("Line " + std::to_string(line_count) + ": Unknown voltage source \"" + sourceName.toStdString() + "\"");
    auto output_control_pair = voltageSources[sourceName];
    output = output_control_pair.first;
    dev = output_control_pair.second;
    
    line_stream >> value_str;
    value = value_str.toDouble(&ok);
    if (not ok or std::isnan(value) or value < -1000 or value > 1000)
        throw BurnInException("Line " + std::to_string(line_count) + ": Invalid voltage value " + value_str.toStdString() + "");
        
    return new BurnInVoltageSourceSetCommand(dev, sourceName, output, value);
}

BurnInChillerOutputCommand* CommandProcessor::_parseChillerOutputCommand(const QString& line, int line_count) const {
    int cmdlen = getStringForType(COMMAND_CHILLEROUTPUT).length();
    QString args = line.right(line.length() - cmdlen - 1);
    QTextStream line_stream(&args);
    
    bool on = _parseOnOff(line_stream, line_count);
    
    return new BurnInChillerOutputCommand(on);
}

BurnInChillerSetCommand* CommandProcessor::_parseChillerSetCommand(const QString& line, int line_count) const {
    int cmdlen = getStringForType(COMMAND_CHILLERSET).length();
    QString args = line.right(line.length() - cmdlen - 1);
    QTextStream line_stream(&args);
    
    QString value_str;
    double value;
    bool ok;
    
    line_stream >> value_str;
    value = value_str.toDouble(&ok);
    if (not ok or std::isnan(value) or value < JulaboFP50::FP50LowerTempLimit or value >JulaboFP50::FP50UpperTempLimit)
        throw BurnInException("Line " + std::to_string(line_count) + ": Invalid temperature value " + value_str.toStdString() + "");
    
    return new BurnInChillerSetCommand(value);
}

BurnInDAQCommand* CommandProcessor::_parseDaqCMDCommand(const QString& line, const QStringList& daqExecuteables, int line_count) const {
    int cmdlen = getStringForType(COMMAND_DAQCMD).length();
    QString args = line.right(line.length() - cmdlen - 1);
    QTextStream line_stream(&args);
    
    QString execName;
    QString opts;
    
    execName = _getQuotedString(line_stream);
    if (not daqExecuteables.contains(execName))
        throw BurnInException("Line " + std::to_string(line_count) + ": Unavailable DAQ command \"" + execName.toStdString() + "\"");
    opts = _getQuotedString(line_stream);
    
    return new BurnInDAQCommand(execName, opts);
}

QString CommandProcessor::_getQuotedString(QTextStream& in) {
    QString ret;
    bool quoted = false;
    bool escaped = false;
    QChar c;
    while (not in.atEnd()) {
        in >> c;
        if (c == '\'') {
            escaped = not escaped;
            if (escaped)
                continue;
        } else {
            if (c == '"' and not escaped) {
                quoted = not quoted;
                continue;
            }
            else if (c.isSpace() and not escaped and not quoted)
                break;
            escaped = false;
        }
        
        ret += c;
    }
    
    return ret;
}

bool CommandProcessor::_parseOnOff(QTextStream& in, int line_count) {
    QString on_str;
    bool on;

    in >> on_str;
    if (on_str == "on")
        on = true;
    else if (on_str == "off")
        on = false;
    else
        throw BurnInException("Line " + std::to_string(line_count) + ": Excepted \"on\" or \"off\" but got \"" + on_str.toStdString() + "\"");
        
    return on;
}
