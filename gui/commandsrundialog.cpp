#include "commandsrundialog.h"
#include "ui_commandsrundialog.h"
#include "commanddisplayer.h"

CommandExecuter::CommandExecuter(const QVector<BurnInCommand*>& commands, const SystemControllerClass* controller, QWidget *parent) :
    QObject(parent)
{
    _commands = commands;
    _controller = controller;
}

void CommandExecuter::start() {
    int n = 0;
    for (const auto& command: _commands) {
        CommandExecuteHandler handler(this, n, _controller);
        emit commandStarted(n, QDateTime::currentDateTime());
        command->accept(handler);
        emit commandFinished(n, QDateTime::currentDateTime());
        
        if (handler.error)
            break; // Halt on errors
        
        ++n;
    }
    
    emit allFinished();
}

CommandExecuter::CommandExecuteHandler::CommandExecuteHandler(CommandExecuter* executer, int n, const SystemControllerClass* controller) {
    _executer = executer;
    _n = n;
    _controller = controller;
    
    error = false;
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInWaitCommand& command) {
    emit _executer->commandStatusUpdate(_n, "Waiting");
    QThread::sleep(command.wait);
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
    
    //TODO
    //if (command.source->getPower())
    //    _waitForVoltage(command.source, command.output)
}

void CommandExecuter::CommandExecuteHandler::handleCommand(BurnInVoltageSourceSetCommand& command) {
    emit _executer->commandStatusUpdate(_n, "Setting voltage");
    command.source->setVolt(command.value, command.output);
    emit _executer->commandStatusUpdate(_n, "Voltage set");
    
    //TODO
    //if (command.source->getPower())
    //    _waitForVoltage(command.source, command.output)
}

void CommandExecuter::CommandExecuteHandler::_waitForVoltage(PowerControlClass* source, int output) {
    emit _executer->commandStatusUpdate(_n, "Waiting for output to reach voltage");
    
    //TODO
    //while (std::abs(source->getVolt(output) - source->getSetVolt(output)) > VOLTAGESRC_EPSILON)
    //    QThread::sleep(VOLTAGESRC_WAIT_INTERVAL);
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
        emit _executer->commandStatusUpdate(_n, "Temperature set. Bath at desired temperature");
    } else
        emit _executer->commandStatusUpdate(_n, "Temperature set. Chiller not turned on");
    
}

void CommandExecuter::CommandExecuteHandler::_waitForChiller(JulaboFP50* chiller) {
    emit _executer->commandStatusUpdate(_n, "Waiting for bath to reach temperature");
    
    while (std::abs(chiller->GetBathTemperature() - chiller->GetWorkingTemperature()) > CHILLER_TEMP_EPSILON)
        QThread::sleep(CHILLER_WAIT_INTERVAL);
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
    
    _executer_thread = new QThread;
    
    _executer.moveToThread(_executer_thread);
    connect(&_executer, SIGNAL(commandStarted(int, QDateTime)), this, SLOT(onCommandStarted(int, QDateTime)));
    connect(&_executer, SIGNAL(commandFinished(int, QDateTime)), this, SLOT(onCommandFinished(int, QDateTime)));
    connect(&_executer, SIGNAL(commandStatusUpdate(int, QString)), this, SLOT(onCommandStatusUpdate(int, QString)));
    connect(&_executer, SIGNAL(allFinished()), this, SLOT(onAllFinished()));
    connect(_executer_thread, SIGNAL(started()), &_executer, SLOT(start()));
    connect(_executer_thread, SIGNAL(finished()), _executer_thread, SLOT(deleteLater()));
    
    _executer_thread->start();
}

CommandsRunDialog::~CommandsRunDialog()
{
    delete ui;
}

void CommandsRunDialog::on_abort_button_clicked()
{
}

void CommandsRunDialog::on_pause_button_clicked()
{
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
    
}