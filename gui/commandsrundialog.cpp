#include "commandsrundialog.h"
#include "ui_commandsrundialog.h"
#include "commanddisplayer.h"

#include <QMessageBox>

CommandExecuter::CommandExecuter(const QVector<BurnInCommand*>& commands, const SystemControllerClass* controller, QWidget *parent) :
    QObject(parent)
{
    _commands = commands;
    _controller = controller;
    _shouldAbort = false;
    _shouldPause = false;
    _isRunning = false;
}

void CommandExecuter::start() {
    _shouldAbort = false;
    _isRunning = true;
    int n = 0;
    for (const auto& command: _commands) {
        CommandExecuteHandler handler(this, n, _controller);
        emit commandStarted(n, QDateTime::currentDateTime());
        command->accept(handler);
        emit commandFinished(n, QDateTime::currentDateTime());
        
        if (handler.error)
            break; // Halt on errors
        
        if (_shouldAbort)
            break;
        if (_shouldPause)
            emit commandStatusUpdate(n, "Paused");
        while (_shouldPause)
            QThread::msleep(100);
        emit commandStatusUpdate(n, "Unpaused");
        
        ++n;
    }
    
    _isRunning = false;
    emit allFinished();
}

bool CommandExecuter::isPaused() const {
    return _shouldPause;
}

bool CommandExecuter::isRunning() const {
    return _isRunning;
}

void CommandExecuter::togglePause() {
    _shouldPause = not _shouldPause;
}

void CommandExecuter::abort() {
    _shouldAbort = true;
    _isRunning = false;
}

CommandExecuter::CommandExecuteHandler::CommandExecuteHandler(CommandExecuter* executer, int n, const SystemControllerClass* controller) {
    _executer = executer;
    _n = n;
    _controller = controller;
    
    error = false;
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInWaitCommand& command) {
    unsigned int wait = command.wait;
    emit _executer->commandStatusUpdate(_n, "Waiting");
    while (not _executer->_shouldAbort and wait > 0) {
        QThread::sleep(WAIT_INTERVAL);
        wait -= WAIT_INTERVAL;
    }
    emit _executer->commandStatusUpdate(_n, "Wait finished");
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    if (command.on) {
        emit _executer->commandStatusUpdate(_n, "Turning on output");
        command.source->onPower(command.output);
        emit _executer->commandStatusUpdate(_n, "Turned on output");
    } else {
        emit _executer->commandStatusUpdate(_n, "Turning off output");
        command.source->offPower(command.output);
        emit _executer->commandStatusUpdate(_n, "Turned off output");
    }
    
    if (command.source->getPower(command.output)) {
        if (not _executer->_shouldAbort)
            _waitForVoltage(command.source, command.output);
        emit _executer->commandStatusUpdate(_n, "Voltage source turned on. Voltage at set value.");
    } else
        emit _executer->commandStatusUpdate(_n, "Voltage source turned off.");
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInVoltageSourceSetCommand& command) {
    emit _executer->commandStatusUpdate(_n, "Setting voltage");
    command.source->setVolt(command.value, command.output);
    emit _executer->commandStatusUpdate(_n, "Voltage set");
    
    if (command.source->getPower(command.output)) {
        if (not _executer->_shouldAbort)
            _waitForVoltage(command.source, command.output);
        emit _executer->commandStatusUpdate(_n, "Voltage applied");
    } else
        emit _executer->commandStatusUpdate(_n, "Voltage set. Voltage source output not turned on.");
}

void CommandExecuter::CommandExecuteHandler::_waitForVoltage(PowerControlClass* source, int output) {
    emit _executer->commandStatusUpdate(_n, "Waiting for output to reach voltage");
    
    while (not _executer->_shouldAbort and 
            std::abs(source->getVolt(output) - source->getVoltApp(output)) > VOLTAGESRC_EPSILON)
        QThread::sleep(WAIT_INTERVAL);
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInChillerOutputCommand& command) {
    JulaboFP50* chiller = _controller->getChiller();
    if (chiller == nullptr) {
        emit _executer->commandStatusUpdate(_n, "Error: No chiller connected");
        error = true;
        return;
    }
    
    if (command.on) {
        emit _executer->commandStatusUpdate(_n, "Turning chiller on");
        if (not chiller->SetCirculatorOn()) {
            emit _executer->commandStatusUpdate(_n, "Error: Could not turn on chiller");
            error = true;
            return;
        }
    } else {
        emit _executer->commandStatusUpdate(_n, "Turning chiller off");
        if (not chiller->SetCirculatorOff()) {
            emit _executer->commandStatusUpdate(_n, "Error: Could not turn off chiller");
            error = true;
            return;
        }
    }
    
    if (chiller->GetCirculatorStatus()) {
        _waitForChiller(chiller);
        if (not _executer->_shouldAbort)
            emit _executer->commandStatusUpdate(_n, "Chiller turned on. Bath at desired temperature");
    } else
        emit _executer->commandStatusUpdate(_n, "Chiller turned off");
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInChillerSetCommand& command) {
    JulaboFP50* chiller = _controller->getChiller();
    if (chiller == nullptr) {
        emit _executer->commandStatusUpdate(_n, "Error: No chiller connected");
        error = true;
        return;
    }
    
    emit _executer->commandStatusUpdate(_n, "Setting chiller temperature");
    if (not chiller->SetWorkingTemperature(command.value)) {
        emit _executer->commandStatusUpdate(_n, "Error: Could not set temperature");
        error = true;
        return;
    }
    
    if (chiller->GetCirculatorStatus()) {
        _waitForChiller(chiller);
        if (not _executer->_shouldAbort)
            emit _executer->commandStatusUpdate(_n, "Temperature set. Bath at desired temperature");
    } else
        emit _executer->commandStatusUpdate(_n, "Temperature set. Chiller not turned on");
    
}

void CommandExecuter::CommandExecuteHandler::_waitForChiller(JulaboFP50* chiller) {
    emit _executer->commandStatusUpdate(_n, "Waiting for bath to reach temperature");
    
    while (not _executer->_shouldAbort and 
            std::abs(chiller->GetBathTemperature() - chiller->GetWorkingTemperature()) > CHILLER_TEMP_EPSILON)
        QThread::sleep(WAIT_INTERVAL);
}


CommandsRunDialog::CommandsRunDialog(const QVector<BurnInCommand*>& commands, const SystemControllerClass* controller, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandsRunDialog),
    _executer(commands, controller)
{
    _commands = commands;
    
    ui->setupUi(this);
    
    ui->commands_table->setRowCount(_commands.length());
    int r = 0;
    CommandDisplayer displayer;
    for (const auto& command: _commands) {
        command->accept(displayer);
        ui->commands_table->setItem(r, 0, new QTableWidgetItem(displayer.display));
        ++r;
    }
    
    ui->commands_table->resizeColumnsToContents();
    ui->commands_table->resizeRowsToContents();
    
    _executer.moveToThread(&_executer_thread);
    connect(&_executer, SIGNAL(commandStarted(int, QDateTime)), this, SLOT(onCommandStarted(int, QDateTime)));
    connect(&_executer, SIGNAL(commandFinished(int, QDateTime)), this, SLOT(onCommandFinished(int, QDateTime)));
    connect(&_executer, SIGNAL(commandStatusUpdate(int, QString)), this, SLOT(onCommandStatusUpdate(int, QString)));
    connect(&_executer, SIGNAL(allFinished()), this, SLOT(onAllFinished()));
    connect(&_executer_thread, SIGNAL(started()), &_executer, SLOT(start()));
    
    _executer_thread.start();
}

CommandsRunDialog::~CommandsRunDialog()
{
    delete ui;
}

void CommandsRunDialog::on_abort_button_clicked()
{
    _executer.abort();
    _executer_thread.quit();
    ui->pause_button->setEnabled(false);
    ui->abort_button->setEnabled(false);
}

void CommandsRunDialog::on_pause_button_clicked()
{
    _executer.togglePause();
    if (_executer.isPaused())
        ui->pause_button->setText("Unpause");
    else
        ui->pause_button->setText("Pause");
}

void CommandsRunDialog::onCommandStarted(int n, QDateTime dt) {
    ui->commands_table->setItem(n, 1, new QTableWidgetItem(dt.toString("hh:mm:ss")));
    ui->commands_table->setCurrentCell(n, 0);
}

void CommandsRunDialog::onCommandFinished(int n, QDateTime dt) {
    ui->commands_table->setItem(n, 2, new QTableWidgetItem(dt.toString("hh:mm:ss")));
}

void CommandsRunDialog::onCommandStatusUpdate(int n, QString status) {
    ui->commands_table->setItem(n, 3, new QTableWidgetItem(status));
}

void CommandsRunDialog::onAllFinished() {
    _executer_thread.quit();
}

void CommandsRunDialog::reject() {
    if (_executer.isRunning()) {
        QMessageBox::StandardButton button = QMessageBox::question(this,
            "Command execution running",
            "Commands are still being executed. Abort?");
            
        if (button != QMessageBox::Yes)
            return;
    }
    
    ui->pause_button->setEnabled(false);
    ui->abort_button->setEnabled(false);
    
    _executer.abort();
    _executer_thread.quit();
    _executer_thread.wait();
    QDialog::reject();
}
