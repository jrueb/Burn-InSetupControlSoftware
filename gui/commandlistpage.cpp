#include "commandlistpage.h"

#include <QtGlobal>
#include "commandmodifydialog.h"

CommandListItem::CommandListItem(const BurnInCommand& command_, const QString &text, QListWidget *parent)
    : QListWidgetItem(text, parent), command(command_) {


}

CommandListPage::CommandListPage(QWidget* commandListWidget, QObject *parent) : QObject(parent)
{
    _commandListWidget = commandListWidget;
    
    _add_command_button = _commandListWidget->findChild<QPushButton*>("add_command_button");
    _add_command_menu = new QMenu(_commandListWidget);
    _add_command_button->setMenu(_add_command_menu);
    _add_command_button->setEnabled(false);
    
    _commands_list = _commandListWidget->findChild<QListWidget*>("commands_list");
    
    _proc = nullptr;
    
}

CommandListPage::~CommandListPage() {
    delete _add_command_menu;
    if (_proc != nullptr)
        delete _proc;
}

void CommandListPage::setSystemController(const SystemControllerClass* controller) {
    if (_proc != nullptr)
        delete _proc;
    _proc = new CommandProcessor(controller);
    
    _add_command_menu->clear();
    QVector<BurnInCommandType> commands = _proc->getAvailableCommands();
    if (commands.size() > 0)
        _add_command_button->setEnabled(true);
    QAction* action;
    for (const auto& command: commands) {
        switch(command) {
        case COMMAND_WAIT:
            action = _add_command_menu->addAction("Wait for specific time");
            connect(action, SIGNAL(triggered()), this, SLOT(onAddWait()));
            break;
        case COMMAND_VOLTAGESOURCEOUTPUT:
            action = _add_command_menu->addAction("Turn voltage source output on or off");
            connect(action, SIGNAL(triggered()), this, SLOT(onAddVoltageSourceOutput()));
            break;
        case COMMAND_VOLTAGESOURCESET:
            action = _add_command_menu->addAction("Set voltage source to a specified voltage");
            connect(action, SIGNAL(triggered()), this, SLOT(onAddVoltageSourceAdd()));
            break;
        case COMMAND_CHILLEROUTPUT:
            action = _add_command_menu->addAction("Turn chiller output on or off");
            connect(action, SIGNAL(triggered()), this, SLOT(onAddChillerOutput()));
            break;
        case COMMAND_CHILLERSET:
            action = _add_command_menu->addAction("Set chiller working temperature");
            connect(action, SIGNAL(triggered()), this, SLOT(onAddChillerSet()));
            break;
        default:
            Q_ASSERT(false); // Should not reach
            break;
        }
    }
}

void CommandListPage::onAddWait() {
    bool ok;
    int wait = CommandModifyDialog::commandWait(_commandListWidget->window(), &ok);
    if (not ok)
        return;
        
    BurnInWaitCommand command(wait);
    QString command_display;
    if (wait == 1)
        command_display = "Wait for 1 second";
    else
        command_display = "Wait for " + QString::number(wait) + " seconds";
    CommandListItem* item = new CommandListItem(command, command_display);
    _commands_list->addItem(item);
}

QMap<QString, QPair<int, PowerControlClass*>> CommandListPage::_buildVoltageSourcesVector() const {
    QMap<QString, QPair<int, PowerControlClass*>> availSources;
    for (const auto& source: _proc->getAvailableVoltageSources()) {
        int numOutputs = source.second->getNumOutputs();
        if (numOutputs == 1)
            availSources[QString::fromStdString(source.first)] = qMakePair(0, source.second);
        else {
            for (int i = 0; i < numOutputs; ++i) {
                QString name = QString::fromStdString(source.first) + " output no. " + QString::number(i + 1);
                availSources[name] = qMakePair(i, source.second);
            }
        }
    }
    
    return availSources;
}

void CommandListPage::onAddVoltageSourceOutput() {
    bool ok;
    QMap<QString, QPair<int, PowerControlClass*>> voltageSources = _buildVoltageSourcesVector();
    std::tuple<QString, bool> ret = CommandModifyDialog::commandVoltageSourceOutput(_commandListWidget->window(), voltageSources.keys(), &ok);
    if (not ok)
        return;
    
    PowerControlClass* dev = voltageSources[std::get<0>(ret)].second;
    int output = voltageSources[std::get<0>(ret)].first;
    BurnInVoltageSourceOutputCommand command(dev, output, std::get<1>(ret));
    QString command_display;
    if (std::get<1>(ret))
        command_display = "Turn on " + std::get<0>(ret);
    else
        command_display = "Turn off " + std::get<0>(ret);
    CommandListItem* item = new CommandListItem(command, command_display);
    _commands_list->addItem(item);
}

void CommandListPage::onAddVoltageSourceAdd() {
    bool ok;
    QMap<QString, QPair<int, PowerControlClass*>> voltageSources = _buildVoltageSourcesVector();
    std::tuple<QString, double> ret = CommandModifyDialog::commandVoltageSourceSet(_commandListWidget->window(), voltageSources.keys(), &ok);
    if (not ok)
        return;
    
    PowerControlClass* dev = voltageSources[std::get<0>(ret)].second;
    int output = voltageSources[std::get<0>(ret)].first;
    double value = std::get<1>(ret);
    BurnInVoltageSourceSetCommand command(dev, output, value);
    QString command_display = "Set " + std::get<0>(ret) + " to " + QString::number(value) + " volts";

    CommandListItem* item = new CommandListItem(command, command_display);
    _commands_list->addItem(item);
}

void CommandListPage::onAddChillerOutput() {
    bool ok;
    bool on = CommandModifyDialog::commandChillerOutput(_commandListWidget->window(), &ok);
    if (not ok)
        return;
        
    BurnInChillerOutputCommand command(on);
    QString command_display;
    if (on)
        command_display = "Turn chiller output on";
    else
        command_display = "Turn chiller output off";
    
    CommandListItem* item = new CommandListItem(command, command_display);
    _commands_list->addItem(item);
}

void CommandListPage::onAddChillerSet() {
    bool ok;
    double value = CommandModifyDialog::commandChillerSet(_commandListWidget->window(), &ok);
    if (not ok)
        return;
        
    BurnInChillerSetCommand command(value);
    QString command_display = "Set chiller working temperature to " + QString::number(value) + " °C";
    
    CommandListItem* item = new CommandListItem(command, command_display);
    _commands_list->addItem(item);
}
