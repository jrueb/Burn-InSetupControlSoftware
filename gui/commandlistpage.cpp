#include "commandlistpage.h"

#include <QtGlobal>
#include "commandmodifydialog.h"

CommandListItem::CommandListItem(std::shared_ptr<BurnInCommand> command_, QListWidget *parent)
    : QListWidgetItem(parent), command(command_) {

    updateText();
}

void CommandListItem::updateText() {
    DisplayCommandHandler handler;
    command->accept(handler);
    setText(handler.display);
}

void CommandListItem::DisplayCommandHandler::handleCommand(BurnInWaitCommand& command) {
    if (command.wait == 1)
        display = "Wait for 1 second";
    else
        display = "Wait for " + QString::number(command.wait) + " seconds";
}

void CommandListItem::DisplayCommandHandler::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    if (command.on)
        display = "Turn on " + command.sourceName;
    else
        display = "Turn off " + command.sourceName;
}

void CommandListItem::DisplayCommandHandler::handleCommand(BurnInVoltageSourceSetCommand& command) {
    display = "Set " + command.sourceName + " to " + QString::number(command.value) + " volts";
}

void CommandListItem::DisplayCommandHandler::handleCommand(BurnInChillerOutputCommand& command) {
    if (command.on)
        display = "Turn chiller output on";
    else
        display = "Turn chiller output off";
}

void CommandListItem::DisplayCommandHandler::handleCommand(BurnInChillerSetCommand& command) {
    display = "Set chiller working temperature to " + QString::number(command.value) + " °C";
}

CommandListPage::CommandListPage(QWidget* commandListWidget, QObject *parent) : QObject(parent)
{
    _commandListWidget = commandListWidget;
    
    _add_command_button = _commandListWidget->findChild<QPushButton*>("add_command_button");
    _add_command_menu = new QMenu(_commandListWidget);
    _add_command_button->setMenu(_add_command_menu);
    _add_command_button->setEnabled(false);
    
    _commands_list = _commandListWidget->findChild<QListWidget*>("commands_list");
    connect(_commands_list, SIGNAL(itemSelectionChanged()),
        this, SLOT(onItemSelectionChanged()));
    
    _alter_command_buttons = _commandListWidget->findChild<QWidget*>("alter_command_buttons");
    _alter_command_buttons->setEnabled(false);
    
    QPushButton* delete_command_button = _commandListWidget->findChild<QPushButton*>("delete_command_button");
    connect(delete_command_button, SIGNAL(pressed()), this, SLOT(onDeleteButtonPressed()));
    
    _change_params_button = _commandListWidget->findChild<QPushButton*>("change_params_button");
    connect(_change_params_button, SIGNAL(pressed()), this, SLOT(onChangeParamsButtonPressed()));
    
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

void CommandListPage::onItemSelectionChanged() {
    int num_selected = _commands_list->selectedItems().size();
    
    _alter_command_buttons->setEnabled(num_selected > 0);
    _change_params_button->setEnabled(num_selected == 1);
}

void CommandListPage::onDeleteButtonPressed() {
    qDeleteAll(_commands_list->selectedItems());
}

void CommandListPage::onChangeParamsButtonPressed() {
    CommandListItem* item = dynamic_cast<CommandListItem*>(_commands_list->currentItem());
    bool ok;
    QMap<QString, QPair<int, PowerControlClass*>> voltageSources = _buildVoltageSourcesVector();
    CommandModifyDialog::modifyCommand(_commandListWidget->window(), &ok, item->command.get(), voltageSources);
    item->updateText();
    
}

void CommandListPage::onAddWait() {
    bool ok;
    int wait = CommandModifyDialog::commandWait(_commandListWidget->window(), &ok);
    if (not ok)
        return;
        
    auto command = std::make_shared<BurnInWaitCommand>(wait);
    
    CommandListItem* item = new CommandListItem(command);
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
    
    QString name = std::get<0>(ret);
    PowerControlClass* dev = voltageSources[name].second;
    int output = voltageSources[name].first;
    auto command = std::make_shared<BurnInVoltageSourceOutputCommand>(dev, name, output, std::get<1>(ret));
    
    CommandListItem* item = new CommandListItem(command);
    _commands_list->addItem(item);
}

void CommandListPage::onAddVoltageSourceAdd() {
    bool ok;
    QMap<QString, QPair<int, PowerControlClass*>> voltageSources = _buildVoltageSourcesVector();
    std::tuple<QString, double> ret = CommandModifyDialog::commandVoltageSourceSet(_commandListWidget->window(), voltageSources.keys(), &ok);
    if (not ok)
        return;
    
    QString name = std::get<0>(ret);
    PowerControlClass* dev = voltageSources[name].second;
    int output = voltageSources[name].first;
    double value = std::get<1>(ret);
    auto command = std::make_shared<BurnInVoltageSourceSetCommand>(dev, name, output, value);

    CommandListItem* item = new CommandListItem(command);
    _commands_list->addItem(item);
}

void CommandListPage::onAddChillerOutput() {
    bool ok;
    bool on = CommandModifyDialog::commandChillerOutput(_commandListWidget->window(), &ok);
    if (not ok)
        return;
        
    auto command = std::make_shared<BurnInChillerOutputCommand>(on);
    
    CommandListItem* item = new CommandListItem(command);
    _commands_list->addItem(item);
}

void CommandListPage::onAddChillerSet() {
    bool ok;
    double value = CommandModifyDialog::commandChillerSet(_commandListWidget->window(), &ok);
    if (not ok)
        return;
        
    auto command = std::make_shared<BurnInChillerSetCommand>(value);
    
    CommandListItem* item = new CommandListItem(command);
    _commands_list->addItem(item);
}