#include "commandmodifydialog.h"
#include "ui_commandmodifydialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include "general/JulaboFP50.h"

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

unsigned int CommandModifyDialog::commandWait(QWidget *parent, bool* ok, int value) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Wait for", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QSpinBox* spin = new QSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(0);
    spin->setSuffix(" s");
    spin->setValue(value);
    dialog.ui->horizontalLayout->insertWidget(1, spin);
    
    int res = dialog.exec();
    value = spin->value();
    
    *ok = res == QDialog::Accepted;
    return value;
    
}

std::tuple<QString, bool> CommandModifyDialog::commandVoltageSourceOutput(QWidget *parent, const QList<QString>& voltageSources, bool* ok, QString source, bool on) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label1 = new QLabel("Turn", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* onOffCombo = new QComboBox(&dialog);
    onOffCombo->addItem("on");
    onOffCombo->addItem("off");
    onOffCombo->setCurrentText(on ? "on" : "off");
    dialog.ui->horizontalLayout->insertWidget(1, onOffCombo);
    
    QLabel* label2 = new QLabel("voltage soruce", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QComboBox* sourceCombo = new QComboBox(&dialog);
    for (const auto& source: voltageSources)
        sourceCombo->addItem(source);
    if (source != "")
        sourceCombo->setCurrentText(source);
    dialog.ui->horizontalLayout->insertWidget(3, sourceCombo);
    
    int res = dialog.exec();
    source = sourceCombo->currentText();
    on = onOffCombo->currentText() == "on";
    
    *ok = res == QDialog::Accepted;
    return std::make_tuple(source, on);
}

std::tuple<QString, double> CommandModifyDialog::commandVoltageSourceSet(QWidget *parent, const QList<QString>& voltageSources, bool* ok, QString source, double value) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label1 = new QLabel("Set", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* sourceCombo = new QComboBox(&dialog);
    for (const auto& source: voltageSources)
        sourceCombo->addItem(source);
    if (source != "")
        sourceCombo->setCurrentText(source);
    dialog.ui->horizontalLayout->insertWidget(1, sourceCombo);
    
    QLabel* label2 = new QLabel("to", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(-1000);
    spin->setMaximum(1000);
    spin->setSuffix(" V");
    spin->setValue(value);
    dialog.ui->horizontalLayout->insertWidget(3, spin);
    
    int res = dialog.exec();
    source = sourceCombo->currentText();
    value = spin->value();
    
    *ok = res == QDialog::Accepted;
    return std::make_tuple(source, value);
}

bool CommandModifyDialog::commandChillerOutput(QWidget *parent, bool* ok, bool on) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Turn chiller output", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QComboBox* onOffCombo = new QComboBox(&dialog);
    onOffCombo->addItem("on");
    onOffCombo->addItem("off");
    onOffCombo->setCurrentText(on ? "on" : "off");
    dialog.ui->horizontalLayout->insertWidget(1, onOffCombo);
    
    int res = dialog.exec();
    on = onOffCombo->currentText() == "on";
    
    *ok = res == QDialog::Accepted;
    return on;
}

double CommandModifyDialog::commandChillerSet(QWidget *parent, bool* ok, double value) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Set chiller working temperature to", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(JulaboFP50::FP50LowerTempLimit);
    spin->setMaximum(JulaboFP50::FP50UpperTempLimit);
    spin->setSuffix(" °C");
    spin->setValue(value);
    dialog.ui->horizontalLayout->insertWidget(1, spin);
    
    int res = dialog.exec();
    value = spin->value();
    
    *ok = res == QDialog::Accepted;
    return value;
}

std::tuple<QString, QString> CommandModifyDialog::commandDAQCmd(QWidget *parent, const QStringList& executeables, bool* ok, QString execName, QString switches) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label1 = new QLabel("Execute DAQ ACF command", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* execCombo = new QComboBox(&dialog);
    for (const auto& exec: executeables)
        execCombo->addItem(exec);
    if (execName != "")
        execCombo->setCurrentText(execName);
    dialog.ui->horizontalLayout->insertWidget(1, execCombo);
    
    QLabel* label2 = new QLabel("Options:");
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QLineEdit* switchesEdit = new QLineEdit(&dialog);
    switchesEdit->setMinimumWidth(100);
    switchesEdit->setText(switches);
    dialog.ui->horizontalLayout->insertWidget(3, switchesEdit);
    
    int res = dialog.exec();
    execName = execCombo->currentText();
    switches = switchesEdit->text();
    
    *ok = res == QDialog::Accepted;
    return std::make_tuple(execName, switches);
}

CommandModifyDialog::ModifyCommandHandler::ModifyCommandHandler(QWidget *parent_, bool* ok_, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources_,  const QStringList& daqExecutables_):
    parent(parent_),
    ok(ok_),
    voltageSources(voltageSources_),
    daqExecutables(daqExecutables_) {

}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInWaitCommand& command) {
    unsigned int wait = CommandModifyDialog::commandWait(parent, ok, command.wait);
    if (not *ok)
        return;
    
    command.wait = wait;
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInVoltageSourceOutputCommand& command) {
    QPair<int, PowerControlClass*> voltageSource = qMakePair(command.output, command.source);
    QString name = voltageSources.key(voltageSource);
    Q_ASSERT(name != ""); // false, if name not found or empty string in map
    
    auto ret = CommandModifyDialog::commandVoltageSourceOutput(parent, voltageSources.keys(), ok, name, command.on);
    if (not *ok)
        return;
    name = std::get<0>(ret);
    command.output = voltageSources[name].first;
    command.source = voltageSources[name].second;
    command.sourceName = name;
    command.on = std::get<1>(ret);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInVoltageSourceSetCommand& command) {
    QPair<int, PowerControlClass*> voltageSource = qMakePair(command.output, command.source);
    QString name = voltageSources.key(voltageSource);
    Q_ASSERT(name != ""); // false, if name not found or empty string in map
    
    auto ret = CommandModifyDialog::commandVoltageSourceSet(parent, voltageSources.keys(), ok, name, command.value);
    if (not *ok)
        return;
    name = std::get<0>(ret);
    command.output = voltageSources[name].first;
    command.source = voltageSources[name].second;
    command.sourceName = name;
    command.value = std::get<1>(ret);
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInChillerOutputCommand& command) {
    bool on = CommandModifyDialog::commandChillerOutput(parent, ok, command.on);
    if (not *ok)
        return;
    
    command.on = on;
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInChillerSetCommand& command) {
    double value = CommandModifyDialog::commandChillerSet(parent, ok, command.value);
    if (not *ok)
        return;
    
    command.value = value;
}

void CommandModifyDialog::ModifyCommandHandler::handleCommand(BurnInDAQCommand& command) {
    std::tuple<QString, QString> ret = CommandModifyDialog::commandDAQCmd(parent, daqExecutables, ok, command.execName, command.opts);
    if (not *ok)
        return;
    
    command.execName = std::get<0>(ret);
    command.opts = std::get<1>(ret);
}

void CommandModifyDialog::modifyCommand(QWidget *parent, bool* ok, BurnInCommand* command, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources, const QStringList& daqExecutables) {
    CommandModifyDialog::ModifyCommandHandler handler(parent, ok, voltageSources, daqExecutables);
    command->accept(handler);
}
