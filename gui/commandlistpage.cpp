#include "commandlistpage.h"

#include <QtGlobal>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFileDialog>
#include <QMessageBox>
#include <algorithm>
#include "commandmodifydialog.h"
#include "commandsrundialog.h"
#include "commanddisplayer.h"
#include "general/BurnInException.h"

CommandListItem::CommandListItem(std::shared_ptr<BurnInCommand> command_, QListWidget *parent)
    : QListWidgetItem(parent), command(command_) {

    updateText();
}

void CommandListItem::updateText() {
    CommandDisplayer displayer;
    command->accept(displayer);
    setText(displayer.display);
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
    connect(_commands_list, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(onCommandsListContextMenu(const QPoint&)));
    
    QAction* cutAction = new QAction(_commands_list);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, SIGNAL(triggered()), this, SLOT(onCommandsListCut()));
    _commands_list->addAction(cutAction);
    
    QAction* copyAction = new QAction(_commands_list);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(onCommandsListCopy()));
    _commands_list->addAction(copyAction);
    
    QAction* pasteAction = new QAction(_commands_list);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(onCommandsListPaste()));
    _commands_list->addAction(pasteAction);
    
    _commands_list_modified = false;
    
    _alter_command_buttons = _commandListWidget->findChild<QWidget*>("alter_command_buttons");
    _alter_command_buttons->setEnabled(false);
    
    QPushButton* delete_command_button = _commandListWidget->findChild<QPushButton*>("delete_command_button");
    connect(delete_command_button, SIGNAL(pressed()), this, SLOT(onDeleteButtonPressed()));
    
    _change_params_button = _commandListWidget->findChild<QPushButton*>("change_params_button");
    connect(_change_params_button, SIGNAL(pressed()), this, SLOT(onChangeParamsButtonPressed()));
    
    QPushButton* command_up_button = _commandListWidget->findChild<QPushButton*>("command_up_button");
    connect(command_up_button, SIGNAL(pressed()), this, SLOT(onCommandUpPressed()));
    
    QPushButton* command_down_button = _commandListWidget->findChild<QPushButton*>("command_down_button");
    connect(command_down_button, SIGNAL(pressed()), this, SLOT(onCommandDownPressed()));
    
    QPushButton* save_list_button = _commandListWidget->findChild<QPushButton*>("save_list_button");
    connect(save_list_button, SIGNAL(pressed()), this, SLOT(onSaveListPressed()));
    
    QPushButton* open_list_button = _commandListWidget->findChild<QPushButton*>("open_list_button");
    connect(open_list_button, SIGNAL(pressed()), this, SLOT(onOpenListPressed()));
    
    _run_button = _commandListWidget->findChild<QPushButton*>("run_button");
    connect(_run_button, SIGNAL(pressed()), this, SLOT(onRunPressed()));
    
    _proc = nullptr;
    
}

CommandListPage::~CommandListPage() {
    delete _add_command_menu;
    if (_proc != nullptr)
        delete _proc;
}

void CommandListPage::setSystemController(const SystemControllerClass* controller) {
    _controller = controller;
    _add_command_menu->clear();
    
    if (_proc != nullptr) {
        delete _proc;
        _proc = nullptr;
    }
    if (controller == nullptr)
        return;
    
    _proc = new CommandProcessor(controller);
    
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
        case COMMAND_DAQCMD:
            action = _add_command_menu->addAction("Execute a DAQ command");
            connect(action, SIGNAL(triggered()), this, SLOT(onAddDAQCmd()));
            break;
        }
    }
}

void CommandListPage::onItemSelectionChanged() {
    int num_selected = _commands_list->selectedItems().size();
    
    _alter_command_buttons->setEnabled(num_selected > 0);
    _change_params_button->setEnabled(num_selected == 1);
}

void CommandListPage::onCommandsListContextMenu(const QPoint& pos) {
    QList<QListWidgetItem*> items = _commands_list->selectedItems();
    QWidget* win = _commandListWidget->window();
    QMenu contextMenu("Context menu", win);
    
    QAction cut("Cut", win);
    connect(&cut, SIGNAL(triggered()), this, SLOT(onCommandsListCut()));
    cut.setShortcut(QKeySequence::Cut);
    contextMenu.addAction(&cut);
    
    QAction copy("Copy", win);
    connect(&copy, SIGNAL(triggered()), this, SLOT(onCommandsListCopy()));
    copy.setShortcut(QKeySequence::Copy);
    contextMenu.addAction(&copy);
    
    QAction paste("Paste", win);
    connect(&paste, SIGNAL(triggered()), this, SLOT(onCommandsListPaste()));
    paste.setShortcut(QKeySequence::Paste);
    contextMenu.addAction(&paste);
    
    QAction del("Delete", win);
    connect(&del, SIGNAL(triggered()), this, SLOT(onDeleteButtonPressed()));
    del.setShortcut(QKeySequence::Delete);
    contextMenu.addAction(&del);
    
    contextMenu.addSeparator();
    
    QAction change("Change", win);
    connect(&change, SIGNAL(triggered()), this, SLOT(onChangeParamsButtonPressed()));
    contextMenu.addAction(&change);
    if (items.length() > 1)
        change.setEnabled(false);
    
    QAction moveUp("Move up", win);
    connect(&moveUp, SIGNAL(triggered()), this, SLOT(onCommandUpPressed()));
    contextMenu.addAction(&moveUp);
    
    QAction moveDown("Move down", win);
    connect(&moveDown, SIGNAL(triggered()), this, SLOT(onCommandDownPressed()));
    contextMenu.addAction(&moveDown);
    
    if (items.length() == 0) {
        cut.setEnabled(false);
        copy.setEnabled(false);
        del.setEnabled(false);
        change.setEnabled(false);
        moveUp.setEnabled(false);
        moveDown.setEnabled(false);
    } else if (items.length() > 1) {
        change.setEnabled(false);
    }
    
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    if (not mimeData->hasText())
        paste.setEnabled(false);
    
    contextMenu.exec(_commands_list->mapToGlobal(pos));
}

void CommandListPage::onCommandsListCut() {
    QList<QListWidgetItem*> items = _commands_list->selectedItems();
    if (items.length() == 0)
        return;
        
    onCommandsListCopy();
    
    qDeleteAll(items);
    _commands_list_modified = true;
}

void CommandListPage::onCommandsListCopy() {
    QList<QListWidgetItem*> items = _commands_list->selectedItems();
    if (items.length() == 0)
        return;
    QClipboard* clipboard = QApplication::clipboard();
        
    QVector<BurnInCommand*> commands;
    for (const auto& item: items)
        commands.push_back(dynamic_cast<CommandListItem*>(item)->command.get());
    clipboard->setText(_proc->getCommandListAsString(commands));
}

void CommandListPage::onCommandsListPaste() {
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    if (not mimeData->hasText())
        return;
    
    const QMap<QString, QPair<int, PowerControlClass*>> voltageSources = _buildVoltageSourcesVector();
    QVector<BurnInCommand*> commands;
    try {
        commands = _proc->getCommandListFromString(mimeData->text(), voltageSources, _getAvailableACFBinaries());
    } catch (const BurnInException& e) {
        // Error during parsing of clipboard text. Do nothing
        return;
    }
    if (commands.length() > 0) {
        _addCommands(commands);
        _commands_list_modified = true;
    }
}

void CommandListPage::onDeleteButtonPressed() {
    QList<QListWidgetItem*> items = _commands_list->selectedItems();
    _commands_list_modified = items.length() > 0;
    qDeleteAll(items);
}

void CommandListPage::onChangeParamsButtonPressed() {
    CommandListItem* item = dynamic_cast<CommandListItem*>(_commands_list->currentItem());
    bool ok;
    QMap<QString, QPair<int, PowerControlClass*>> voltageSources = _buildVoltageSourcesVector();
    QStringList daqExecuteables = _getAvailableACFBinaries();
    CommandModifyDialog::modifyCommand(_commandListWidget->window(), &ok, item->command.get(), voltageSources, daqExecuteables);
    if (ok) {
        item->updateText();
        _commands_list_modified = true;
    }
}

void CommandListPage::onCommandUpPressed() {
    QList<QListWidgetItem*> items = _commands_list->selectedItems();
    
    // Sort items by their row, lowest row number first
    std::sort(items.begin(), items.end(), [this](QListWidgetItem* a, QListWidgetItem* b) -> bool {
        return _commands_list->row(a) < _commands_list->row(b);
    });
    
    int topRow = 0; // Do not move items past this row
    for (const auto& item : items) {
        int row = _commands_list->row(item);
        int newRow = row - 1;
        if (topRow >= newRow) {
            newRow = topRow;
            ++topRow;
        }
            
        _commands_list->takeItem(row);
        _commands_list->insertItem(newRow, item);
        
        // Select item again
        _commands_list->setCurrentRow(newRow, QItemSelectionModel::Select);
    }
    _commands_list_modified = true;
}

void CommandListPage::onCommandDownPressed() {
    QList<QListWidgetItem*> items = _commands_list->selectedItems();
    
    // Sort items by their row, higest row number first
    std::sort(items.begin(), items.end(), [this](QListWidgetItem* a, QListWidgetItem* b) -> bool {
        return _commands_list->row(a) > _commands_list->row(b);
    });
    
    int bottomRow = _commands_list->count() - 1; // Do not move items past this row
    for (const auto& item : items) {
        int row = _commands_list->row(item);
        int newRow = row + 1;
        if (bottomRow <= newRow) {
            newRow = bottomRow;
            --bottomRow;
        }
            
        _commands_list->takeItem(row);
        _commands_list->insertItem(newRow, item);
        
        // Select item again
        _commands_list->setCurrentRow(newRow, QItemSelectionModel::Select);
    }
    _commands_list_modified = true;
}

void CommandListPage::onSaveListPressed() {
    QString fileName = QFileDialog::getSaveFileName(_commandListWidget->window(), "Save command list");
    if (fileName.isEmpty())
        return;
    
    QVector<BurnInCommand*> commands;
    
    for (int i = 0; i < _commands_list->count(); ++i) {
        CommandListItem* item = dynamic_cast<CommandListItem*>(_commands_list->item(i));
        commands.push_back(item->command.get());
    }
    
    _proc->saveCommandList(commands, fileName);
    _commands_list_modified = false;
}

void CommandListPage::onOpenListPressed() {
    if (_commands_list_modified and _commands_list->count() != 0) {
        QMessageBox dialog(_commandListWidget->window());
        QMessageBox::StandardButton button = dialog.question(_commandListWidget->window(),
            "Current command list not empty",
            "The current command list is not empty and will be lost"
            " unless it's saved. Open different list anyway?");
        if (button != QMessageBox::Yes)
            return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(_commandListWidget->window(), "Open command list");
    if (fileName.isEmpty())
        return;
    
    QVector<BurnInCommand*> commands;
    try {
        commands = _proc->getCommandListFromFile(fileName, _buildVoltageSourcesVector(), _getAvailableACFBinaries());
    } catch (BurnInException& e) {
        qWarning("%s", e.what());
        QMessageBox dialog(_commandListWidget->window());
        dialog.critical(_commandListWidget->window(), "Error", "Error while reading commands: " + QString::fromStdString(e.what()));
        return;
    }
    
    _commands_list->clear();
    _addCommands(commands);
    
    _commands_list_modified = false;
}

void CommandListPage::_addCommands(const QVector<BurnInCommand*>& commands) {
    for (const auto& command: commands) {
        std::shared_ptr<BurnInCommand> command_ptr(command);
        CommandListItem* item = new CommandListItem(command_ptr);
        _commands_list->addItem(item);
    }
}

void CommandListPage::onRunPressed() {
    QVector<BurnInCommand*> commands;
    for (int i = 0; i < _commands_list->count(); ++i) {
        CommandListItem* item = dynamic_cast<CommandListItem*>(_commands_list->item(i));
        commands.push_back(item->command.get());
    }
    
    CommandsRunDialog* dialog = new CommandsRunDialog(commands, _controller, _commandListWidget->window());
    
    connect(dialog, SIGNAL(finished(int)), this, SLOT(onRunFinished()));
    
    dialog->show();
    _run_button->setText("Running...");
    _run_button->setEnabled(false);
}

void CommandListPage::onRunFinished() {
    _run_button->setEnabled(true);
    _run_button->setText("Run");
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
    for (const auto& source: _controller->getVoltageSources()) {
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

QStringList CommandListPage::_getAvailableACFBinaries() const {
    DAQModule* module = _controller->getDaqModule();
    if (module == nullptr)
        return QStringList();
    else
        return module->getAvailableACFBinaries();
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

void CommandListPage::onAddDAQCmd() {
    bool ok;
    std::tuple<QString, QString> ret = CommandModifyDialog::commandDAQCmd(_commandListWidget->window(), _getAvailableACFBinaries(), &ok);
    if (not ok)
        return;
        
    QString execName = std::get<0>(ret);
    QString opts = std::get<1>(ret);
    auto command = std::make_shared<BurnInDAQCommand>(execName, opts);
    
    CommandListItem* item = new CommandListItem(command);
    _commands_list->addItem(item);
}
