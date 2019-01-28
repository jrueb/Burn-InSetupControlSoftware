#include "commandmodifydialog.h"
#include "ui_commandmodifydialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>

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

unsigned int CommandModifyDialog::commandWait(QWidget *parent, bool* ok) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Wait for", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QSpinBox* spin = new QSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(0);
    spin->setSuffix(" s");
    spin->setValue(60);
    dialog.ui->horizontalLayout->insertWidget(1, spin);
    
    int res = dialog.exec();
    int value = spin->value();
    
    *ok = res == QDialog::Accepted;
    return value;
    
}

std::tuple<QString, bool> CommandModifyDialog::commandVoltageSourceOutput(QWidget *parent, const QList<QString>& voltageSources, bool* ok) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label1 = new QLabel("Turn", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* onOffCombo = new QComboBox(&dialog);
    onOffCombo->addItem("on");
    onOffCombo->addItem("off");
    dialog.ui->horizontalLayout->insertWidget(1, onOffCombo);
    
    QLabel* label2 = new QLabel("voltage soruce", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QComboBox* sourceCombo = new QComboBox(&dialog);
    for (const auto& source: voltageSources)
        sourceCombo->addItem(source);
    dialog.ui->horizontalLayout->insertWidget(3, sourceCombo);
    
    int res = dialog.exec();
    QString source = sourceCombo->currentText();
    bool on = onOffCombo->currentText() == "on";
    
    *ok = res == QDialog::Accepted;
    return std::make_tuple(source, on);
}

std::tuple<QString, double> CommandModifyDialog::commandVoltageSourceSet(QWidget *parent, const QList<QString>& voltageSources, bool* ok) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label1 = new QLabel("Set", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label1);
    
    QComboBox* sourceCombo = new QComboBox(&dialog);
    for (const auto& source: voltageSources)
        sourceCombo->addItem(source);
    dialog.ui->horizontalLayout->insertWidget(1, sourceCombo);
    
    QLabel* label2 = new QLabel("to", &dialog);
    dialog.ui->horizontalLayout->insertWidget(2, label2);
    
    QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(-1000);
    spin->setSuffix(" V");
    spin->setValue(0);
    dialog.ui->horizontalLayout->insertWidget(3, spin);
    
    int res = dialog.exec();
    QString source = sourceCombo->currentText();
    double value = spin->value();
    
    *ok = res == QDialog::Accepted;
    return std::make_tuple(source, value);
}

bool CommandModifyDialog::commandChillerOutput(QWidget *parent, bool* ok) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Turn chiller output", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QComboBox* onOffCombo = new QComboBox(&dialog);
    onOffCombo->addItem("on");
    onOffCombo->addItem("off");
    dialog.ui->horizontalLayout->insertWidget(1, onOffCombo);
    
    int res = dialog.exec();
    bool on = onOffCombo->currentText() == "on";
    
    *ok = res == QDialog::Accepted;
    return on;
}

int CommandModifyDialog::commandChillerSet(QWidget *parent, bool* ok) {
    CommandModifyDialog dialog(parent);
    
    QLabel* label = new QLabel("Set chiller working temperature to", &dialog);
    dialog.ui->horizontalLayout->insertWidget(0, label);
    
    QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
    spin->setMinimumWidth(100);
    spin->setMinimum(-28); // Julabo minimum temperature is -28 °C
    spin->setSuffix(" °C");
    spin->setValue(20);
    dialog.ui->horizontalLayout->insertWidget(1, spin);
    
    int res = dialog.exec();
    double value = spin->value();
    
    *ok = res == QDialog::Accepted;
    return value;
}
