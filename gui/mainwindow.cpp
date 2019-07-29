#include <iostream>
#include <typeinfo>

#include <QFile>
#include <QFileDialog>
#include <QThread>
#include <QStandardItemModel>
#include <QTimer>
#include <QAbstractItemModel>
#include <QMessageBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLCDNumber>
#include <QVBoxLayout>
#include <QLabel>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devices/environment/JulaboFP50.h"
#include "general/BurnInException.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    commandListPage = new CommandListPage(ui->CommandList);
    daqPage = new DAQPage(ui->DAQControl);

    fControl = nullptr;
    
    ui->tabWidget->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

output_pointer_t MainWindow::SetSourceOutputLayout() const
{
    // create pointer list
    output_pointer_t cOutputPointers;

    // i/v set
    cOutputPointers.i_set = new QDoubleSpinBox();
    cOutputPointers.i_set->setMaximumHeight(20);
    cOutputPointers.i_set->setDecimals(3);
    cOutputPointers.i_set->setSuffix(" A");
    cOutputPointers.v_set = new QDoubleSpinBox();
    cOutputPointers.v_set->setMaximumHeight(20);
    cOutputPointers.v_set->setMinimum(-100000);
    cOutputPointers.v_set->setDecimals(3);
    cOutputPointers.v_set->setSuffix(" V");

    // applied
    cOutputPointers.i_applied = new QLCDNumber();
    cOutputPointers.i_applied->setMaximumHeight(20);
    cOutputPointers.i_applied->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.i_applied->setDigitCount(8);
    cOutputPointers.v_applied = new QLCDNumber();
    cOutputPointers.v_applied->setMaximumHeight(20);
    cOutputPointers.v_applied->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.v_applied->setDigitCount(8);

    // on off
    cOutputPointers.onoff_button = new QCheckBox("On");
    cOutputPointers.onoff_button->setMaximumHeight(20);

    // return
    return cOutputPointers;
}

output_Raspberry MainWindow::setRaspberryLayout(string pName)
{
    output_Raspberry cOutputRaspberry;

    cOutputRaspberry.layout = new QHBoxLayout;

    cOutputRaspberry.label = new QLabel(QString::fromStdString(pName));
    cOutputRaspberry.label->setMaximumHeight(20);
    cOutputRaspberry.value = new QLCDNumber();
    cOutputRaspberry.value->setMaximumHeight(20);
    cOutputRaspberry.layout->addWidget(cOutputRaspberry.label);
    cOutputRaspberry.value->setSegmentStyle(QLCDNumber::Flat);

    cOutputRaspberry.layout->addWidget(cOutputRaspberry.value);

    return cOutputRaspberry;
}

output_Chiller MainWindow::setChilerLayout()
{
    output_Chiller cOutputPointers;

    cOutputPointers.setTemperature = new QDoubleSpinBox();
    cOutputPointers.setTemperature->setMaximumHeight(20);
    cOutputPointers.setTemperature->setMinimum(-100000);
    cOutputPointers.setTemperature->setSuffix(" °C");

    cOutputPointers.bathTemperature = new QLCDNumber();
    cOutputPointers.bathTemperature->setMaximumHeight(20);
    cOutputPointers.bathTemperature->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.bathTemperature->setDigitCount(6);

    cOutputPointers.sensorTemperature = new QLCDNumber();
    cOutputPointers.sensorTemperature->setMaximumHeight(20);
    cOutputPointers.sensorTemperature->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.sensorTemperature->setDigitCount(6);

    cOutputPointers.pressure = new QLCDNumber();
    cOutputPointers.pressure->setMaximumHeight(20);
    cOutputPointers.pressure->setSegmentStyle(QLCDNumber::Flat);

    // on off
    cOutputPointers.onoff_button = new QCheckBox("On");
    cOutputPointers.onoff_button->setMaximumHeight(20);

    return cOutputPointers;
}

output_Chiller* MainWindow::SetChillerOutput(QLayout *pMainLayout, string pName)
{
    // output pointers
    output_Chiller *cOutputPointers = new output_Chiller[1];

    // horizontal layout
    QGroupBox *group_box = new QGroupBox(pName.c_str());
    QGridLayout *group_box_layout = new QGridLayout;

    QSize size(80,20);

    // set the labels
    QLabel *label_t_set = new QLabel("Temperature set:");
    label_t_set->setMinimumSize(size);
    group_box_layout->addWidget(label_t_set, 0, 0);
    QLabel *label_t_bath = new QLabel("Temperature bath, °C:");
    label_t_bath->setMinimumSize(size);
    group_box_layout->addWidget(label_t_bath, 1, 0);
    QLabel *label_t_sensor = new QLabel("Temperature sensor, °C:");
    label_t_sensor->setMinimumSize(size);
    group_box_layout->addWidget(label_t_sensor, 2, 0);
    QLabel *label_pressure = new QLabel("Pressure stage:");
    label_pressure->setMinimumSize(size);
    group_box_layout->addWidget(label_pressure, 3, 0);
    QLabel *label_on_off = new QLabel("On/Off:");
    label_on_off->setMinimumSize(size);
    group_box_layout->addWidget(label_on_off, 4, 0);

    cOutputPointers[0] = setChilerLayout();
    group_box_layout->addWidget(cOutputPointers[0].setTemperature, 0, 1);
    group_box_layout->addWidget(cOutputPointers[0].bathTemperature, 1, 1);
    group_box_layout->addWidget(cOutputPointers[0].sensorTemperature, 2, 1);
    group_box_layout->addWidget(cOutputPointers[0].pressure, 3, 1);
    group_box_layout->addWidget(cOutputPointers[0].onoff_button, 4, 1);

    group_box->setLayout(group_box_layout);

    // finally add to the main layout
    pMainLayout->addWidget(group_box);

    // return everything
    return cOutputPointers;
}

output_pointer_t* MainWindow::SetVoltageSource(QLayout *pMainLayout, std::string pName, std::string pType, int pNoutputs)
{
    // create the output pointers
    output_pointer_t* cOutputPointers = new output_pointer_t[pNoutputs];

    // horizontal layout
    QGroupBox *group_box = new QGroupBox(pName.c_str());
    QGridLayout *group_box_layout = new QGridLayout;

    QSize size(300, 20);

    // set the labels
    QLabel *label_type = new QLabel("Type:");
    label_type->setMinimumSize(size);
    group_box_layout->addWidget(label_type, 0, 0);
    QLabel *label_i_set = new QLabel("Current limit:");
    label_i_set->setMinimumSize(size);
    group_box_layout->addWidget(label_i_set, 1, 0);
    QLabel *label_v_set = new QLabel("Voltage:");
    label_v_set->setMinimumSize(size);
    group_box_layout->addWidget(label_v_set, 2, 0);
    QLabel *label_i_applied = new QLabel("Current (A):");
    label_i_applied->setMinimumSize(size);
    group_box_layout->addWidget(label_i_applied, 3, 0);
    QLabel *label_v_applied = new QLabel("Voltage app. (V):");
    label_v_applied->setMinimumSize(size);
    group_box_layout->addWidget(label_v_applied, 4, 0);
    QLabel *label_onoff = new QLabel("On/Off:");
    label_onoff->setMinimumSize(size);
    group_box_layout->addWidget(label_onoff, 5, 0);

    // set the outputs
    for (int i = 0; i < pNoutputs; i++) {
        cOutputPointers[i] = SetSourceOutputLayout();
        QLabel *type = new QLabel(pType.c_str());
        type->setMaximumHeight(20);
        group_box_layout->addWidget(type, 0, i + 1);
        group_box_layout->addWidget(cOutputPointers[i].i_set, 1, i + 1);
        group_box_layout->addWidget(cOutputPointers[i].v_set, 2, i + 1);
        group_box_layout->addWidget(cOutputPointers[i].i_applied, 3, i + 1);
        group_box_layout->addWidget(cOutputPointers[i].v_applied, 4, i + 1);
        group_box_layout->addWidget(cOutputPointers[i].onoff_button, 5, i + 1);
    }

    // set the group box
    group_box->setLayout(group_box_layout);

    // finally add to the main layout
    pMainLayout->addWidget(group_box);

    // return everything
    return cOutputPointers;
}

output_Raspberry* MainWindow::SetRaspberryOutput(QLayout *pMainLayout , vector<string> pNames , string pNameGroupBox)
{
    output_Raspberry* cOutputPointers = new output_Raspberry[pNames.size()];

    QGroupBox *group_box = new QGroupBox(QString::fromStdString(pNameGroupBox));

    QHBoxLayout *cLayout_group_box = new QHBoxLayout;

    QVBoxLayout *cLayout_raspberry = new QVBoxLayout;
    for(size_t i = 0 ; i < pNames.size() ; i++){
        cOutputPointers[i] = setRaspberryLayout(pNames[i]);
        cLayout_raspberry->addItem(cOutputPointers[i].layout);
        cLayout_raspberry->addSpacing(15);
    }
    //cLayout_raspberry->addStretch();
    QSpacerItem *item = new QSpacerItem(0 , 100 ,QSizePolicy::Expanding, QSizePolicy::Fixed);
    cLayout_raspberry->addItem(item);

    cLayout_group_box->addItem(cLayout_raspberry);

    group_box->setLayout(cLayout_group_box);

    pMainLayout->addWidget(group_box);

    return cOutputPointers;
}

void MainWindow::on_OnOff_button_stateChanged(string pSourceName, int dev_num, int pId, bool pArg)
{
    if(pSourceName.substr(0, 3) == "TTI"){
        if(pArg){

            fControl->getObject(pSourceName)->setVolt(gui_pointers_low_voltage[dev_num][2 - pId].v_set->value(), pId);
            fControl->getObject(pSourceName)->setCurr(gui_pointers_low_voltage[dev_num][2 - pId].i_set->value(), pId);
            fControl->getObject(pSourceName)->onPower(pId);
        }
        else{
            fControl->getObject(pSourceName)->offPower(pId);
        }
    }
    if(pSourceName == "Keithley2410"){

        if(pArg){
            fControl->getObject(pSourceName)->setVolt(gui_pointers_high_voltage[dev_num]->v_set->value(), pId);
            fControl->getObject(pSourceName)->setCurr(gui_pointers_high_voltage[dev_num]->i_set->value(), pId);
            fControl->getObject(pSourceName)->onPower(0);
        }
        else{
            fControl->getObject(pSourceName)->offPower(0);
        }
    }
    if(pSourceName == "JulaboFP50"){
        JulaboFP50* chiller = dynamic_cast<JulaboFP50*>(fControl->getGenericInstrObj("JulaboFP50"));
        if(pArg){
            gui_chiller->setTemperature->setEnabled(false);
            
            chiller->SetWorkingTemperature(gui_chiller->setTemperature->value());
            chiller->SetCirculatorOn();
        }
        else{
            gui_chiller->setTemperature->setEnabled(true);
            
            chiller->SetCirculatorOff();
        }
    }
}

//Set func
void MainWindow::on_V_set_doubleSpinBox_valueChanged(string pSourceName, int pId, double pVolt)
{
    fControl->getObject(pSourceName)->setVolt(pVolt, pId);
    QThread::sleep(0.5);
}

void MainWindow::on_I_set_doubleSpinBox_valueChanged(string pSourceName , int pId, double pCurr)
{
    fControl->getObject(pSourceName)->setCurr(pCurr, pId);
    QThread::sleep(0.5);
}

void MainWindow::_connectTTi() {
    const vector<string> sources = fControl->getSourceNameVec();
    int dev_num = 0;
    for (const string& name: sources) {
        if (name.substr(0, 3) != "TTI")
            continue;
            
        ControlTTiPower* ttidev = dynamic_cast<ControlTTiPower*>(fControl->getGenericInstrObj(name));
        output_pointer_t* widgets = gui_pointers_low_voltage[dev_num];
        
        connect(ttidev, &ControlTTiPower::voltSetChanged, this, [widgets, dev_num](double volt, int id) {
            QDoubleSpinBox* box = widgets[2 - id].v_set;
            QSignalBlocker blocker(box);
            box->setValue(volt);
        });
        connect(ttidev, &ControlTTiPower::currSetChanged, this, [widgets, dev_num](double curr, int id) {
            QDoubleSpinBox* box = widgets[2 - id].i_set;
            QSignalBlocker blocker(box);
            box->setValue(curr);
        });
        connect(ttidev, &ControlTTiPower::voltAppChanged, this, [widgets, dev_num](double volt, int id) {
            QLCDNumber* num = widgets[2 - id].v_applied;
            QSignalBlocker blocker(num);
            // If a small number needs too many digits, display() seems to do nothing.
            if (abs(volt) < 0.0001)
                num->display(0.);
            else
                num->display(volt);
        });
        connect(ttidev, &ControlTTiPower::currAppChanged, this, [widgets, dev_num](double curr, int id) {
            QLCDNumber* num = widgets[2 - id].i_applied;
            QSignalBlocker blocker(num);
            if (abs(curr) < 0.0001)
                num->display(0.);
            else
                num->display(curr);
        });
        connect(ttidev, &ControlTTiPower::powerStateChanged, this, [widgets, dev_num](bool on, int id) {
            QCheckBox* box = widgets[2 - id].onoff_button;
            QSignalBlocker blocker(box);
            box->setChecked(on);
        });
        
        ++dev_num;
    }
    if (dev_num == 0) {
        ui->groupBox->setEnabled(false);
        return;
    }
}

void MainWindow::_connectKeithley() {
    if (fControl->countInstrument("Keithley2410") == 0) {
        ui->groupBox_2->setEnabled(false);
        return;
    }
    
    ControlKeithleyPower* keihleydev = dynamic_cast<ControlKeithleyPower*>(fControl->getGenericInstrObj("Keithley2410"));
    output_pointer_t* widget = gui_pointers_high_voltage[0];

    connect(keihleydev, &ControlKeithleyPower::voltSetChanged, this, [widget](double volt, int) {
        QSignalBlocker blocker(widget->v_set);
        widget->v_set->setValue(volt);
    });
    connect(keihleydev, &ControlKeithleyPower::currSetChanged, this, [widget](double curr, int) {
        QSignalBlocker blocker(widget->i_set);
        widget->i_set->setValue(curr);
    });
    connect(keihleydev, &ControlKeithleyPower::voltAppChanged, this, [widget](double volt, int) {
        QSignalBlocker blocker(widget->v_applied);
        if (abs(volt) < 0.0001)
            widget->v_applied->display(0.);
        else
            widget->v_applied->display(volt);
    });
    connect(keihleydev, &ControlKeithleyPower::currAppChanged, this, [widget](double curr, int) {
        QSignalBlocker blocker(widget->i_applied);
        if (abs(curr) < 0.0001)
            widget->i_applied->display(0.);
        else
            widget->i_applied->display(curr);
    });
    connect(keihleydev, &ControlKeithleyPower::powerStateChanged, this, [widget](bool on, int) {
        QSignalBlocker blocker(widget->onoff_button);
        widget->onoff_button->setChecked(on);
        widget->v_set->setEnabled(not on);
        widget->i_set->setEnabled(not on);
    });
}

void MainWindow::_connectJulabo() {
    if (fControl->countInstrument("JulaboFP50") == 0) {
        ui->groupBox_Chiller->setEnabled(false);
        return;
    }
    
    JulaboFP50* chiller = dynamic_cast<JulaboFP50*>(fControl->getGenericInstrObj("JulaboFP50"));
    output_Chiller* widget = gui_chiller;
    
    connect(chiller, &JulaboFP50::circulatorStatusChanged, this, [widget](bool on) {
        QSignalBlocker blocker(widget->onoff_button);
        widget->onoff_button->setChecked(on);
        widget->setTemperature->setEnabled(not on);
    });
    connect(chiller, &JulaboFP50::workingTemperatureChanged, this, [widget](float temperature) {
        QSignalBlocker blocker(widget->setTemperature);
        widget->setTemperature->setValue(temperature);
    });
    connect(chiller, &JulaboFP50::bathTemperatureChanged, this, [widget](float temperature) {
        QSignalBlocker blocker(widget->bathTemperature);
        widget->bathTemperature->display(temperature);
    });
    connect(chiller, &JulaboFP50::safetySensorTemperatureChanged, this, [widget](float temperature) {
        QSignalBlocker blocker(widget->sensorTemperature);
        widget->sensorTemperature->display(temperature);
    });
    connect(chiller, &JulaboFP50::pumpPressureChanged, this, [widget](unsigned int pressureStage) {
        QSignalBlocker blocker(widget->pressure);
        widget->pressure->display(QString::number(pressureStage));
    });
}

void MainWindow::_connectThermorasp() {
    for (size_t n = 0; n < fControl->getNumRasps(); ++n) {
        Thermorasp* thermorasp = fControl->getThermorasp(n);
        output_Raspberry* widget = gui_raspberrys[n];
        std::vector<std::string> names = thermorasp->getSensorNames();
        
        connect(thermorasp, &Thermorasp::gotNewReadings, this, [widget, names](const QMap<QString, QString>& readings) {
            int i = 0;
            for (const string& name: names) {
                widget[i].value->display(readings[QString::fromStdString(name)]);
                ++i;
            }
        });
    }
}

void MainWindow::initialize()
{
    // Connect devices to GUI widgets
    _connectTTi();
    _connectKeithley();
    _connectJulabo();
    _connectThermorasp();
    
    // Initialize the hardware devices
    fControl->Initialize();
    
    // Setup thread to refresh the readings from the devices
    fControl->startRefreshingReadings();
}

bool MainWindow::readXmlFile()
{
    QString cFilter  = "*.xml";
    QString cFileName = QFileDialog::getOpenFileName(this, "Open hardware description file", "./settings", cFilter);

    if (cFileName.isEmpty())
        return false;
    else {
        fControl->ReadXmlFile(cFileName.toStdString());
        fSources = fControl->getSourceNameVec();

        for(auto const& i: fControl->fGenericInstrumentMap){

            if( dynamic_cast<ControlTTiPower*>(i.second) ){

                gui_pointers_low_voltage.push_back(SetVoltageSource(ui->lowVoltageLayout, i.first, "TTI", 2));
                int dev_num = gui_pointers_low_voltage.size() - 1;

                for (int id = 0; id < 2; id++){
                    connect(gui_pointers_low_voltage[dev_num][id].onoff_button, &QCheckBox::toggled, [this, i, id, dev_num](bool pArg)
                        {this->on_OnOff_button_stateChanged(i.first, dev_num, 2 - id, pArg);});
                    connect(gui_pointers_low_voltage[dev_num][id].v_set, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, i, id](double pVolt)
                        {this->on_V_set_doubleSpinBox_valueChanged(i.first, 2 - id, pVolt);});
                    connect(gui_pointers_low_voltage[dev_num][id].i_set, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, i, id](double pCurr)
                        {this->on_I_set_doubleSpinBox_valueChanged(i.first, 2 - id, pCurr);});
                 }

            }

            if( dynamic_cast<ControlKeithleyPower*>(i.second) ){

                gui_pointers_high_voltage.push_back(SetVoltageSource(ui->highVoltageLayout, i.first, "Keithley2410", 1));

                 connect(gui_pointers_high_voltage[0]->onoff_button, &QCheckBox::toggled, [this](bool pArg)
                 {this->on_OnOff_button_stateChanged("Keithley2410", 0, 0, pArg);});
            }

            if( dynamic_cast<Thermorasp*>(i.second) ){
                Thermorasp* rasp = dynamic_cast<Thermorasp*>(i.second);
                gui_raspberrys.push_back(SetRaspberryOutput(ui->envMonitorLayout, rasp->getSensorNames(), i.first));
            }

            if( dynamic_cast<JulaboFP50*>(i.second) ){
                gui_chiller = SetChillerOutput(ui->envControlLayout , i.first);

                connect(gui_chiller->onoff_button, &QCheckBox::toggled, [this](bool pArg)
                {this->on_OnOff_button_stateChanged("JulaboFP50", 0, 0, pArg);});
            }
        }
        
        commandListPage->setSystemController(fControl);
        if (fControl->getDaqModule() != nullptr)
            daqPage->setDAQModule(fControl->getDaqModule());

        return true;
    }
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
        
        if (fControl->getDaqModule() != nullptr)
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
            for (auto& p: gui_pointers_low_voltage)
                delete p;
            for (auto& p: gui_pointers_high_voltage)
                delete p;
            gui_pointers_low_voltage.clear();
            gui_pointers_high_voltage.clear();
            for (auto& p: gui_raspberrys)
                delete p;
            gui_raspberrys.clear();
            delete gui_chiller;

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
    // Set chillder temperature and turn off
    if (fControl != nullptr and fControl->countInstrument("JulaboFP50") > 0) {
        JulaboFP50* chiller = dynamic_cast<JulaboFP50*>(fControl->getGenericInstrObj("JulaboFP50"));
        chiller->SetWorkingTemperature(20);
        chiller->SetCirculatorOff();
    }
    
    // Turn off Keithley power
    if (fControl != nullptr and fControl->countInstrument("Keithley2410")) {
        ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(fControl->getGenericInstrObj("Keithley2410"));
        keithley->offPower();
        keithley->waitForSafeShutdown();
    }
    
    // Turn off TTi power
    if (fControl != nullptr) {
        const vector<string> sources = fControl->getSourceNameVec();
        for (const string& name: sources) {
            if (name.substr(0, 3) != "TTI")
                continue;
                
            ControlTTiPower* ttidev = dynamic_cast<ControlTTiPower*>(fControl->getGenericInstrObj(name));
            ttidev->offPower(0);
        }
    }
}
