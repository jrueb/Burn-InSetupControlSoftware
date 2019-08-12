#include <iostream>
#include <typeinfo>

#include <QFileDialog>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QLCDNumber>
#include <QLabel>
#include <QFormLayout>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "general/BurnInException.h"
#include "general/hwdescriptionparser.h"

DeviceWidget::DeviceWidget(const QString& title)
    : QGroupBox(title)
{
    
}

VoltageSourceWidgetControls::VoltageSourceWidgetControls() {
    i_set = new QDoubleSpinBox();
    i_set->setDecimals(3);
    i_set->setSuffix(" A");
    v_set = new QDoubleSpinBox();
    v_set->setDecimals(3);
    v_set->setSuffix(" V");
    i_applied = new QLCDNumber();
    i_applied->setSegmentStyle(QLCDNumber::Flat);
    i_applied->setDigitCount(8);
    v_applied = new QLCDNumber();
    v_applied->setSegmentStyle(QLCDNumber::Flat);
    v_applied->setDigitCount(8);

    // on off
    onoff_button = new QCheckBox("On");
}

VoltageSourceWidget::VoltageSourceWidget(const QString& title, PowerControlClass* device, bool settersAlwaysEnabled)
    : DeviceWidget(title)
{
    _device = device;
    _settersAlwaysEnabled = settersAlwaysEnabled;
    setTitle(title);
    QGridLayout *group_box_layout = new QGridLayout(this);

    // set the labels
    QLabel *label_i_set = new QLabel("Current limit:");
    group_box_layout->addWidget(label_i_set, 0, 0);
    QLabel *label_v_set = new QLabel("Voltage:");
    group_box_layout->addWidget(label_v_set, 1, 0);
    QLabel *label_i_applied = new QLabel("Current (A):");
    group_box_layout->addWidget(label_i_applied, 2, 0);
    QLabel *label_v_applied = new QLabel("Voltage app. (V):");
    group_box_layout->addWidget(label_v_applied, 3, 0);
    QLabel *label_onoff = new QLabel("On/Off:");
    group_box_layout->addWidget(label_onoff, 4, 0);

    for (int i = 0; i < device->getNumOutputs(); i++) {
        VoltageSourceWidgetControls control;
        group_box_layout->addWidget(control.i_set, 0, i + 1);
        group_box_layout->addWidget(control.v_set, 1, i + 1);
        group_box_layout->addWidget(control.i_applied, 2, i + 1);
        group_box_layout->addWidget(control.v_applied, 3, i + 1);
        group_box_layout->addWidget(control.onoff_button, 4, i + 1);
        _controls.push_back(control);
        
        connect(control.onoff_button, &QCheckBox::toggled, this, [this, i](bool state) {
            this->onOnOffToggled(i, state);
        });
        if (settersAlwaysEnabled) {
            connect(control.v_set, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [device, i](double voltage) {
                device->setVolt(voltage, i);
            });
            connect(control.i_set, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [device, i](double current) {
               device->setCurr(current, i);
            });
        }
    }

    setLayout(group_box_layout);
}

void VoltageSourceWidget::onOnOffToggled(int output, bool state) {
    if (state) {
        _device->setVolt(_controls[output - 1].v_set->value(), output);
        _device->setCurr(_controls[output - 1].i_set->value(), output);
        _device->onPower(output);
    } else {
        _device->offPower(output);
    }
}

void VoltageSourceWidget::initialize() {
    connect(_device, &PowerControlClass::voltSetChanged, this, [this](double volt, int output) {
        QDoubleSpinBox* box = this->_controls[output - 1].v_set;
        QSignalBlocker blocker(box);
        box->setValue(volt);
    });
    connect(_device, &PowerControlClass::currSetChanged, this, [this](double curr, int output) {
        QDoubleSpinBox* box = this->_controls[output - 1].i_set;
        QSignalBlocker blocker(box);
        box->setValue(curr);
    });
    connect(_device, &PowerControlClass::voltAppChanged, this, [this](double volt, int output) {
        QLCDNumber* num = this->_controls[output - 1].v_applied;
        QSignalBlocker blocker(num);
        // If a small number needs too many digits, display() seems to do nothing.
        if (abs(volt) < 0.0001)
            num->display(0.);
        else
            num->display(volt);
    });
    connect(_device, &PowerControlClass::currAppChanged, this, [this](double curr, int output) {
        QLCDNumber* num = this->_controls[output - 1].i_applied;
        QSignalBlocker blocker(num);
        if (abs(curr) < 0.0001)
            num->display(0.);
        else
            num->display(curr);
    });
    connect(_device, &PowerControlClass::powerStateChanged, this, [this](bool on, int output) {
        QCheckBox* box = this->_controls[output - 1].onoff_button;
        QSignalBlocker blocker(box);
        box->setChecked(on);
        if (not this->_settersAlwaysEnabled) {
            this->_controls[output - 1].v_set->setEnabled(not on);
            this->_controls[output - 1].i_set->setEnabled(not on);
        }
    });
}

ThermoraspWidget::ThermoraspWidget(const QString& title, Thermorasp* device)
    : DeviceWidget(title)
{
    _device = device;
    setTitle(title);
    
    QFormLayout* layout = new QFormLayout(this);
    for (const auto& name: device->getSensorNames()) {
        QLabel* label = new QLabel(name.c_str());
        label->setMaximumHeight(20);
        
        QLCDNumber* value = new QLCDNumber();
        value->setMaximumHeight(20);
        value->setSegmentStyle(QLCDNumber::Flat);
        _values.push_back(value);
        
        layout->addRow(label, value);
    }
    
    setLayout(layout);
}

void ThermoraspWidget::initialize() {
    connect(_device, &Thermorasp::gotNewReadings, this, [this](const QMap<QString, QString>& readings) {
        int i = 0;
        for (const auto& name: this->_device->getSensorNames()) {
            this->_values[i]->display(readings[QString::fromStdString(name)]);
            ++i;
        }
    });
}

ChillerWidget::ChillerWidget(const QString& title, Chiller* device)
    : DeviceWidget(title)
{
    _device = device;
    setTitle(title);
    
    QFormLayout* layout = new QFormLayout(this);
    
    QLabel* workingTempLabel = new QLabel("Temperature set:");
    _workingTemp = new QDoubleSpinBox();
    _workingTemp->setMinimum(device->GetMinTemp());
    _workingTemp->setMaximum(device->GetMaxTemp());
    _workingTemp->setSuffix(" °C");
    layout->addRow(workingTempLabel, _workingTemp);
    
    QLabel* bathTempLabel = new QLabel("Temperature bath, °C:");
    _bathTemp = new QLCDNumber();
    _bathTemp->setSegmentStyle(QLCDNumber::Flat);
    _bathTemp->setDigitCount(6);
    layout->addRow(bathTempLabel, _bathTemp);
    
    QLabel* onoffLabel = new QLabel("On/Off:");
    _onoffButton = new QCheckBox("On");
    layout->addRow(onoffLabel, _onoffButton);
    
    connect(_onoffButton, &QCheckBox::toggled, this, &ChillerWidget::onOnOffToggled);
    
    setLayout(layout);
    
}

void ChillerWidget::onOnOffToggled(bool state) {
    _workingTemp->setEnabled(not state);
    if (state) {
        _device->SetWorkingTemperature(_workingTemp->value());
        _device->SetCirculatorOn();
    } else{
        _device->SetCirculatorOff();
    }
}

void ChillerWidget::initialize() {
    connect(_device, &Chiller::circulatorStatusChanged, this, [this](bool on) {
        QSignalBlocker blocker(this->_onoffButton);
        this->_onoffButton->setChecked(on);
        this->_workingTemp->setEnabled(not on);
    });
    connect(_device, &Chiller::workingTemperatureChanged, this, [this](float temperature) {
        QSignalBlocker blocker(this->_workingTemp);
        this->_workingTemp->setValue(temperature);
    });
    connect(_device, &Chiller::bathTemperatureChanged, this, [this](float temperature) {
        QSignalBlocker blocker(this->_bathTemp);
        this->_bathTemp->display(temperature);
    });
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    commandListPage = new CommandListPage(ui->CommandList);
    daqPage = new DAQPage(ui->DAQControl);

    fControl = nullptr;
    
    ui->tabWidget->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initialize()
{
    // Connect devices to GUI widgets
    for (auto& widget: _deviceWidgets)
        widget->initialize();
    
    // Initialize the hardware devices
    fControl->initialize();
    
    // Setup thread to refresh the readings from the devices
    fControl->startRefreshingReadings();
}

bool MainWindow::readXmlFile()
{
    QString cFilter  = "*.xml";
    QString cFileName = QFileDialog::getOpenFileName(this, "Open hardware description file", "./settings", cFilter);

    if (cFileName.isEmpty())
        return false;
        
    HWDescriptionParser cParser;
    std::vector<GenericInstrumentDescription_t> descriptions = cParser.ParseXML(cFileName);
    
    fControl->setupFromDesc(descriptions);
    for (const auto& source: fControl->getLowVoltageSources()) {
        QString name = QString::fromStdString(fControl->getId(source));
        VoltageSourceWidget* widget = new VoltageSourceWidget(name, source, true);
        ui->lowVoltageLayout->addWidget(widget);
        _lowVoltageWidgets.push_back(widget);
        _deviceWidgets.push_back(widget);
    }
    for (const auto& source: fControl->getHighVoltageSources()) {
        QString name = QString::fromStdString(fControl->getId(source));
        VoltageSourceWidget* widget = new VoltageSourceWidget(name, source, false);
        ui->highVoltageLayout->addWidget(widget);
        _highVoltageWidgets.push_back(widget);
        _deviceWidgets.push_back(widget);
    }
    for (const auto& rasp: fControl->getThermorasps()) {
        QString name = QString::fromStdString(fControl->getId(rasp));
        ThermoraspWidget* widget = new ThermoraspWidget(name, rasp);
        ui->envMonitorLayout->addWidget(widget);
        _thermoraspWidgets.push_back(widget);
        _deviceWidgets.push_back(widget);
    }
    for (const auto& chiller: fControl->getChillers()) {
        QString name = QString::fromStdString(fControl->getId(chiller));
        ChillerWidget* widget = new ChillerWidget(name, chiller);
        ui->envControlLayout->addWidget(widget);
        _chillerWidgets.push_back(widget);
        _deviceWidgets.push_back(widget);
    }
    
    commandListPage->setSystemController(fControl);
    if (fControl->getDaqModules().size() != 0)
        daqPage->setDAQModule(fControl->getDaqModules()[0]);

    return true;
}

void MainWindow::on_read_conf_button_clicked()
{
    bool xml_was_read = false;
    fControl = new SystemControllerClass();
    try {
        // read the xml file
        if (not readXmlFile())
            return;
        xml_was_read = true;
        
        initialize();
            
        // enable back
        ui->tabWidget->setEnabled(true);
        ui->read_conf_button->setEnabled(false);
        
        if (fControl->getDaqModules().size() != 0)
            ui->DAQControl->setEnabled(true);
            
    } catch (const BurnInException& e) {
        qCritical("%s", e.what());
        
        commandListPage->setSystemController(nullptr);
        daqPage->setDAQModule(nullptr);
        
        QMessageBox dialog(this);
        dialog.critical(this, "Error", QString::fromStdString(e.what()));
        
        if (fControl != nullptr) {
            delete fControl;
            fControl = nullptr;
        }
        
        if (xml_was_read) {
            // Do a clean up of things that were created already
            for (auto& widget: _deviceWidgets)
                delete widget;
            _deviceWidgets.clear();
            _lowVoltageWidgets.clear();
            _highVoltageWidgets.clear();
            _thermoraspWidgets.clear();
            _chillerWidgets.clear();

            for (const auto& child : ui->lowVoltageContents->children()) {
                if (child != ui->lowVoltageLayout)
                    delete child;
            }
            for (const auto& child : ui->highVoltageContents->children()) {
                if (child != ui->highVoltageLayout)
                    delete child;
            }
            for (const auto& child : ui->envMonitorContents->children()) {
                if (child != ui->envMonitorLayout)
                    delete child;
            }
            for (const auto& child : ui->envControlContents->children()) {
                if (child != ui->envControlLayout)
                    delete child;
            }
        }
    }
}

void MainWindow::app_quit() {
    qDebug("Qutting");
    if (fControl != nullptr) {
        for (auto& chiller: fControl->getChillers()) {
            chiller->SetWorkingTemperature(20);
            chiller->SetCirculatorOff();
        }
        for (auto& source: fControl->getVoltageSources()) {
            source->offPower(0);
            ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(source);
            if (keithley)
                keithley->waitForSafeShutdown();
        }
    }
}
