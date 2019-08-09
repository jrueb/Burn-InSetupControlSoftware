#include "commandmodifydialog.h"
#include "ui_commandmodifydialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include "devices/environment/JulaboFP50.h"

CommandModifyDialog::CommandModifyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandModifyDialog)
{
    ui->setupUi(this);

}

CommandModifyDialog::~CommandModifyDialog()
{
    delete ui;
}

bool CommandModifyDialog::commandWait(QWidget *parent, BurnInWaitCommand *command) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Wait for", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QSpinBox* spin = new QSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(0);
    spin->setSuffix(" s");
    spin->setValue(command->wait);
    dialog.ui->horizontalLayout->insertWidget(1, spin);
    
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        command->wait = spin->value();
        return true;
    } else {
        return false;
    }
}

std::map<QString, QPair<int, PowerControlClass*>> CommandModifyDialog::_getAvailableVoltageSources(const SystemControllerClass *controller) {
    std::map<QString, QPair<int, PowerControlClass*>> availSources;
    for (const auto& source: controller->getVoltageSources()) {
        int numOutputs = source->getNumOutputs();
        QString id = QString::fromStdString(controller->getId(source));
        if (numOutputs == 1)
            availSources[id] = qMakePair(0, source);
        else {
            for (int i = 0; i < numOutputs; ++i) {
                QString name = id + " output no. " + QString::number(i + 1);
                availSources[name] = qMakePair(i, source);
            }
        }
    }
    
    return availSources;
}

bool CommandModifyDialog::commandVoltageSourceOutput(QWidget *parent, BurnInVoltageSourceOutputCommand *command, const SystemControllerClass *controller) {
    CommandModifyDialog dialog(parent);
    std::map<QString, QPair<int, PowerControlClass*>> sources = _getAvailableVoltageSources(controller);
    
    QLabel* label1 = new QLabel("Turn", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* onOffCombo = new QComboBox(&dialog);
    onOffCombo->addItem("on");
    onOffCombo->addItem("off");
    onOffCombo->setCurrentText(command->on ? "on" : "off");
    dialog.ui->horizontalLayout->insertWidget(1, onOffCombo);
    
    QLabel* label2 = new QLabel("voltage soruce", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QComboBox* sourceCombo = new QComboBox(&dialog);
    QString currentSource;
    for (const auto& source: sources) {
        sourceCombo->addItem(source.first);
        if (command->source == source.second.second and command->output == source.second.first)
            currentSource = source.first;
    }
    if (currentSource != "")
        sourceCombo->setCurrentText(currentSource);
    dialog.ui->horizontalLayout->insertWidget(3, sourceCombo);
    
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        command->source = sources[sourceCombo->currentText()].second;
        command->sourceName = QString::fromStdString(controller->getId(command->source));
        command->output = sources[sourceCombo->currentText()].first;
        command->on = onOffCombo->currentText() == "on";
        return true;
    } else {
        return false;
    }
        
}

bool CommandModifyDialog::commandVoltageSourceSet(QWidget *parent, BurnInVoltageSourceSetCommand *command, const SystemControllerClass *controller) {
    CommandModifyDialog dialog(parent);
    std::map<QString, QPair<int, PowerControlClass*>> sources = _getAvailableVoltageSources(controller);
    
    QLabel* label1 = new QLabel("Set", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* sourceCombo = new QComboBox(&dialog);
    QString currentSource;
    for (const auto& source: sources) {
        sourceCombo->addItem(source.first);
        if (command->source == source.second.second and command->output == source.second.first)
            sourceCombo->setCurrentText(currentSource);
    }
    dialog.ui->horizontalLayout->insertWidget(1, sourceCombo);
    
    QLabel* label2 = new QLabel("to", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(-1000);
    spin->setMaximum(1000);
    spin->setSuffix(" V");
    spin->setValue(command->value);
    dialog.ui->horizontalLayout->insertWidget(3, spin);
    
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        command->source = sources[sourceCombo->currentText()].second;
        command->sourceName = QString::fromStdString(controller->getId(command->source));
        command->output = sources[sourceCombo->currentText()].first;
        command->value = spin->value();
        return true;
    } else {
        return false;
    }
}

std::map<QString, Chiller*> CommandModifyDialog::_getAvailableChillers(const SystemControllerClass* controller) {
    std::map<QString, Chiller*> chillers;
    for (auto& chiller: controller->getChillers())
        chillers[QString::fromStdString(controller->getId(chiller))] = chiller;
    
    return chillers;
}

bool CommandModifyDialog::commandChillerOutput(QWidget *parent, BurnInChillerOutputCommand *command, const SystemControllerClass* controller) {
    CommandModifyDialog dialog(parent);
    std::map<QString, Chiller*> chillers = _getAvailableChillers(controller);
    
    QLabel* label = new QLabel("Turn", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QComboBox* chillerCombo = new QComboBox(&dialog);
    for (const auto& chiller: chillers) {
        chillerCombo->addItem(chiller.first);
        if (chiller.second == command->chiller)
            chillerCombo->setCurrentText(chiller.first);
    }
    dialog.ui->horizontalLayout->insertWidget(1, chillerCombo);
    
    QLabel* label2 = new QLabel("output", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QComboBox* onOffCombo = new QComboBox(&dialog);
    onOffCombo->addItem("on");
    onOffCombo->addItem("off");
    onOffCombo->setCurrentText(command->on ? "on" : "off");
    dialog.ui->horizontalLayout->insertWidget(3, onOffCombo);
    
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        command->on = onOffCombo->currentText() == "on";
        command->chiller = chillers[chillerCombo->currentText()];
        command->chillerName = chillerCombo->currentText();
        return true;
    } else {
        return false;
    }
}

bool CommandModifyDialog::commandChillerSet(QWidget *parent, BurnInChillerSetCommand *command, const SystemControllerClass* controller) {
    CommandModifyDialog dialog(parent);
    std::map<QString, Chiller*> chillers = _getAvailableChillers(controller);
    
    QLabel* label = new QLabel("Set", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QComboBox* chillerCombo = new QComboBox(&dialog);
    for (const auto& chiller: chillers) {
        chillerCombo->addItem(chiller.first);
        if (chiller.second == command->chiller)
            chillerCombo->setCurrentText(chiller.first);
    }
    dialog.ui->horizontalLayout->insertWidget(1, chillerCombo);
    
    QLabel* label2 = new QLabel("working temperature to", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(JulaboFP50::FP50LowerTempLimit);
    spin->setMaximum(JulaboFP50::FP50UpperTempLimit);
    spin->setSuffix(" °C");
    spin->setValue(command->value);
    dialog.ui->horizontalLayout->insertWidget(3, spin);
    
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        command->value = spin->value();
        command->chiller = chillers[chillerCombo->currentText()];
        command->chillerName = chillerCombo->currentText();
        return true;
    } else {
        return false;
    }
}

QStringList CommandModifyDialog::_getDaqExecutables(const SystemControllerClass* controller) {
    std::vector<DAQModule*> modules = controller->getDaqModules();
    if (modules.size() == 0)
        return QStringList();
        
    return modules[0]->getAvailableACFBinaries();
}

bool CommandModifyDialog::commandDAQCmd(QWidget *parent, BurnInDAQCommand *command, const SystemControllerClass* controller) {
    CommandModifyDialog dialog(parent);
    QStringList executables = _getDaqExecutables(controller);
    
    QLabel* label1 = new QLabel("Execute DAQ ACF command", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* execCombo = new QComboBox(&dialog);
    for (const auto& exec: executables) {
        execCombo->addItem(exec);
        if (exec == command->execName)
            execCombo->setCurrentText(exec);
    }
    dialog.ui->horizontalLayout->insertWidget(1, execCombo);
    
    QLabel* label2 = new QLabel("Options:");
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QLineEdit* switchesEdit = new QLineEdit(&dialog);
    switchesEdit->setMinimumWidth(100);
    switchesEdit->setText(command->opts);
    dialog.ui->horizontalLayout->insertWidget(3, switchesEdit);
    
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        command->execName = execCombo->currentText();
        command->opts = switchesEdit->text();
        return true;
    } else {
        return false;
    }
}

CommandModifyDialog::ModifyCommandHandler::ModifyCommandHandler(QWidget *parent_, bool* ok_, const SystemControllerClass* controller_):
    parent(parent_),
    ok(ok_),
    controller(controller_) {

}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInWaitCommand& command) {
    *ok = CommandModifyDialog::commandWait(parent, &command);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    *ok = CommandModifyDialog::commandVoltageSourceOutput(parent, &command, controller);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInVoltageSourceSetCommand& command) {
    *ok = CommandModifyDialog::commandVoltageSourceSet(parent, &command, controller);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInChillerOutputCommand& command) {
    *ok = CommandModifyDialog::commandChillerOutput(parent, &command, controller);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInChillerSetCommand& command) {
    *ok = CommandModifyDialog::commandChillerSet(parent, &command, controller);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInDAQCommand& command) {
    *ok = CommandModifyDialog::commandDAQCmd(parent, &command, controller);
}

bool CommandModifyDialog::modifyCommand(QWidget *parent, BurnInCommand* command, const SystemControllerClass* controller) {
    bool ok;
    CommandModifyDialog::ModifyCommandHandler handler(parent, &ok, controller);
    command->accept(handler);
    return ok;
}
