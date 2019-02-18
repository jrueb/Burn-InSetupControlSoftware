#include "daqpage.h"

#include <QCheckBox>
#include <QPushButton>

DAQPage::DAQPage(QWidget* daqPageWidget)
{
    _daqPageWidget = daqPageWidget;
    _module = nullptr;
    
    QCheckBox* fc7power_check = _daqPageWidget->findChild<QCheckBox*>("fc7power_check");
    connect(fc7power_check, SIGNAL(stateChanged(int)), this, SLOT(onFc7powerState(int)));
    
    QPushButton* loadfirmware_button = _daqPageWidget->findChild<QPushButton*>("loadfirmware_button");
    connect(loadfirmware_button, SIGNAL(clicked()), this, SLOT(onLoadfirmwareClicked()));
    
    QPushButton* systemtest_button = _daqPageWidget->findChild<QPushButton*>("systemtest_button");
    connect(systemtest_button, SIGNAL(clicked()), this, SLOT(onSystemtestClicked()));
    
    QPushButton* calibrate_button = _daqPageWidget->findChild<QPushButton*>("calibrate_button");
    connect(calibrate_button, SIGNAL(clicked()), this, SLOT(onCalibrateClicked()));
    
    QPushButton* datatest_button = _daqPageWidget->findChild<QPushButton*>("datatest_button");
    connect(datatest_button, SIGNAL(clicked()), this, SLOT(onDatatestClicked()));
    
    QPushButton* hybridtest_button = _daqPageWidget->findChild<QPushButton*>("hybridtest_button");
    connect(hybridtest_button, SIGNAL(clicked()), this, SLOT(onHybridtestClicked()));
    
    QPushButton* cmtest_button = _daqPageWidget->findChild<QPushButton*>("cmtest_button");
    connect(cmtest_button, SIGNAL(clicked()), this, SLOT(onCmtestClicked()));
    
    QPushButton* noisemeasurement_button = _daqPageWidget->findChild<QPushButton*>("noisemeasurement_button");
    connect(noisemeasurement_button, SIGNAL(clicked()), this, SLOT(onNoiseMeasurementClicked()));
}

void DAQPage::setDAQModule(DAQModule* module) {
    if (_module != nullptr)
        disconnect(_module, SIGNAL(fc7PowerChanged(bool)), this, SLOT(onFc4PowerChanged(bool)));
    
    _module = module;
    if (module == nullptr)
        return;
    
    connect(_module, SIGNAL(fc7PowerChanged(bool)), this, SLOT(onFc4PowerChanged(bool)));
}

void DAQPage::onFc4PowerChanged(bool state) {
    QCheckBox* fc7power_check = _daqPageWidget->findChild<QCheckBox*>("fc7power_check");
    bool blocked = fc7power_check->blockSignals(true);
    fc7power_check->setChecked(state);
    fc7power_check->blockSignals(blocked);
}

void DAQPage::onFc7powerState(int state) {
    _module->setFC7Power(state);
}

void DAQPage::onLoadfirmwareClicked() {
    _module->loadFirmware();
}

void DAQPage::onSystemtestClicked() {
    _module->runSystemTest();
}

void DAQPage::onCalibrateClicked() {
    _module->runCalibrate();
}

void DAQPage::onDatatestClicked() {
    _module->runDatatest();
}

void DAQPage::onHybridtestClicked() {
    _module->runHybridtest();
}

void DAQPage::onCmtestClicked() {
    _module->runCmtest();
}

void DAQPage::onNoiseMeasurementClicked() {
    _module->runNoiseMeasurement();
}


