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
#include "additional/additionalthread.h"
#include "general/JulaboFP50.h"
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
    cOutputPointers.v_applied = new QLCDNumber();
    cOutputPointers.v_applied->setMaximumHeight(20);
    cOutputPointers.v_applied->setSegmentStyle(QLCDNumber::Flat);

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

    cOutputPointers.bathTemperature = new QLCDNumber();
    cOutputPointers.bathTemperature->setMaximumHeight(20);
    cOutputPointers.bathTemperature->setSegmentStyle(QLCDNumber::Flat);

    cOutputPointers.workingTemperature = new QLCDNumber();
    cOutputPointers.workingTemperature->setMaximumHeight(20);
    cOutputPointers.workingTemperature->setSegmentStyle(QLCDNumber::Flat);

    cOutputPointers.sensorTemperature = new QLCDNumber();
    cOutputPointers.sensorTemperature->setMaximumHeight(20);
    cOutputPointers.sensorTemperature->setSegmentStyle(QLCDNumber::Flat);

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
    QLabel *label_t_set = new QLabel("T(set), °C:");
    label_t_set->setMinimumSize(size);
    group_box_layout->addWidget(label_t_set, 0, 0);
    QLabel *label_t_bath = new QLabel("T(bath), °C:");
    label_t_bath->setMinimumSize(size);
    group_box_layout->addWidget(label_t_bath, 1, 0);
    QLabel *label_t_working = new QLabel("T(working), °C:");
    label_t_working->setMinimumSize(size);
    group_box_layout->addWidget(label_t_working, 2, 0);
    QLabel *label_t_sensor = new QLabel("T(sensor), °C:");
    label_t_sensor->setMinimumSize(size);
    group_box_layout->addWidget(label_t_sensor, 3, 0);
    QLabel *label_pressure = new QLabel("P, Pa:");
    label_pressure->setMinimumSize(size);
    group_box_layout->addWidget(label_pressure, 4, 0);
    QLabel *label_on_off = new QLabel("On/Off:");
    label_on_off->setMinimumSize(size);
    group_box_layout->addWidget(label_on_off, 5, 0);

    cOutputPointers[0] = setChilerLayout();
    group_box_layout->addWidget(cOutputPointers[0].setTemperature, 0, 1);
    group_box_layout->addWidget(cOutputPointers[0].bathTemperature, 1, 1);
    group_box_layout->addWidget(cOutputPointers[0].workingTemperature, 2, 1);
    group_box_layout->addWidget(cOutputPointers[0].sensorTemperature, 3, 1);
    group_box_layout->addWidget(cOutputPointers[0].pressure, 4, 1);
    group_box_layout->addWidget(cOutputPointers[0].onoff_button, 5, 1);

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

//reads out TTi once and creates a new thread to take info from TTi
void MainWindow::getVoltAndCurr()
{
    const vector<string> sources = fControl->getSourceNameVec();
    int dev_num = 0;
    for (const string& name: sources) {
        if (name.substr(0, 3) != "TTI")
            continue;
            
        ControlTTiPower* ttidev = dynamic_cast<ControlTTiPower*>(fControl->getGenericInstrObj(name));
        gui_pointers_low_voltage[dev_num][1].i_set->setValue(ttidev->getCurr(1));
        gui_pointers_low_voltage[dev_num][1].v_set->setValue(ttidev->getVolt(1));
        gui_pointers_low_voltage[dev_num][0].i_set->setValue(ttidev->getCurr(2));
        gui_pointers_low_voltage[dev_num][0].v_set->setValue(ttidev->getVolt(2));
        
        for (int i = 0; i < 2; ++i) {
            bool power_status = ttidev->getPower(2 - i);
            bool blocked = gui_pointers_low_voltage[dev_num][i].onoff_button->signalsBlocked();
            gui_pointers_low_voltage[dev_num][i].onoff_button->blockSignals(true);
            gui_pointers_low_voltage[dev_num][i].onoff_button->setChecked(power_status);
            gui_pointers_low_voltage[dev_num][i].onoff_button->blockSignals(blocked);
        }        
        
        ++dev_num;
    }
    if (dev_num == 0) {
        ui->groupBox->setEnabled(false);
        return;
    }
    
    AdditionalThread *cThread  = new AdditionalThread("A", fControl);
    QThread *cQThread = new QThread();
    connect(cQThread , SIGNAL(started()), cThread, SLOT(getVAC()));
    connect(cThread, SIGNAL(sendToThread(double, double, double, double, int)),this,
            SLOT(updateTTiIWidget(double, double, double, double, int)));
    cThread->moveToThread(cQThread);
    cQThread->start();
}


void MainWindow::getMeasurments()
{
    if (fControl->countInstrument("Thermorasp") == 0) {
        ui->groupBox_3->setEnabled(false);
        return;
    }
    
    AdditionalThread *cThread = new AdditionalThread("B" , fControl);
    QThread *cQThread = new QThread();
    connect(cQThread , SIGNAL(started()), cThread, SLOT(getRaspSensors()));
    connect(cThread, SIGNAL(updatedThermorasp(quint64, QMap<QString, QString>)), this, SLOT(updateRaspWidget(quint64, QMap<QString, QString>)));
    cThread->moveToThread(cQThread);
    cQThread->start();
}

void MainWindow::getVoltAndCurrKeithley()
{
    if (fControl->countInstrument("Keithley2410") == 0) {
        ui->groupBox_2->setEnabled(false);
        return;
    }
    
    ControlKeithleyPower* keihleydev = dynamic_cast<ControlKeithleyPower*>(fControl->getGenericInstrObj("Keithley2410"));
    // Keithley is supposed to turn off on init so no need to set onoff_button
    
    bool blocked = gui_pointers_high_voltage[0]->i_set->signalsBlocked();
    gui_pointers_high_voltage[0]->i_set->blockSignals(true);
    gui_pointers_high_voltage[0]->i_set->setValue(keihleydev->getCurr());
    gui_pointers_high_voltage[0]->i_set->blockSignals(blocked);
    
    blocked = gui_pointers_high_voltage[0]->v_set->signalsBlocked();
    gui_pointers_high_voltage[0]->v_set->blockSignals(true);
    gui_pointers_high_voltage[0]->v_set->setValue(keihleydev->getVolt());
    gui_pointers_high_voltage[0]->v_set->blockSignals(blocked);
    
    AdditionalThread *cThread  = new AdditionalThread("C", fControl);
    QThread *cQThread = new QThread();
    connect(cQThread , SIGNAL(started()), cThread, SLOT(getVACKeithley()));
    connect(cThread, SIGNAL(sendToThreadKeithley(double, double)), this,
            SLOT(updateKeithleyWidget(double, double)));
    cThread->moveToThread(cQThread);
    cQThread->start();
}

void MainWindow::getChillerStatus()
{
    if (fControl->countInstrument("JulaboFP50") == 0) {
        ui->groupBox_Chiller->setEnabled(false);
        return;
    }
    
    JulaboFP50* chiller = dynamic_cast<JulaboFP50*>(fControl->getGenericInstrObj("JulaboFP50"));
    bool state = chiller->GetCirculatorStatus();
    bool blocked = gui_chiller->onoff_button->signalsBlocked();
    gui_chiller->onoff_button->blockSignals(true);
    gui_chiller->onoff_button->setChecked(state);
    gui_chiller->onoff_button->blockSignals(blocked);
    if (state) {
        float temperature = chiller->GetWorkingTemperature();
        gui_chiller->setTemperature->setValue(temperature);
        gui_chiller->setTemperature->setEnabled(false);
    }
    
    AdditionalThread *cThread  = new AdditionalThread("C", fControl);
    QThread *cQThread = new QThread();
    connect(cQThread , SIGNAL(started()), cThread, SLOT(getChillerStatus()));
    connect(cThread, SIGNAL(sendFromChiller(QString)),this,
            SLOT(updateChillerWidget(QString)));
    cThread->moveToThread(cQThread);
    cQThread->start();
}

//reads sensor on rasp and sets info to Raspberry sensors
void MainWindow::updateRaspWidget(quint64 n, QMap<QString, QString> readings) {
    int i = 0;
    for (const string& name: fControl->getRaspSensorNames(n)) {
        gui_raspberrys[n][i].value->display(readings[QString::fromStdString(name)]);
        ++i;
    }
}

void MainWindow::updateChillerWidget(QString pStr)
{
    vector<QString> cVec;

    std::istringstream ist(pStr.toStdString());
    std::string tmp;

    while ( ist >> tmp )
        cVec.push_back(QString::fromStdString(tmp));

    gui_chiller->bathTemperature->display(cVec[0].toDouble());
    gui_chiller->pressure->display(cVec[1].toDouble());
    gui_chiller->sensorTemperature->display(cVec[2].toDouble());
    gui_chiller->workingTemperature->display(cVec[3].toDouble());
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
            gui_pointers_high_voltage[0]->v_set->setEnabled(false);
            gui_pointers_high_voltage[0]->i_set->setEnabled(false);
            
            fControl->getObject(pSourceName)->setVolt(gui_pointers_high_voltage[dev_num]->v_set->value(), pId);
            fControl->getObject(pSourceName)->setCurr(gui_pointers_high_voltage[dev_num]->i_set->value(), pId);
            fControl->getObject(pSourceName)->onPower(0);
        }
        else{
            gui_pointers_high_voltage[0]->v_set->setEnabled(true);
            gui_pointers_high_voltage[0]->i_set->setEnabled(true);
            
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

void MainWindow::updateTTiIWidget(double currApp1, double voltApp1, double currApp2, double voltApp2, int dev_num)
{
    // The TTis can not output negative voltages. Sometimes they report small
    // negative values like -0.005 V. Use abs refresh the display anyway.
    gui_pointers_low_voltage[dev_num][1].i_applied->display(abs(currApp1));
    gui_pointers_low_voltage[dev_num][1].v_applied->display(abs(voltApp1));

    gui_pointers_low_voltage[dev_num][0].i_applied->display(abs(currApp2));
    gui_pointers_low_voltage[dev_num][0].v_applied->display(abs(voltApp2));
}

void MainWindow::updateKeithleyWidget(double currApp, double voltApp)
{
    gui_pointers_high_voltage[0]->i_applied->display(currApp);
    gui_pointers_high_voltage[0]->v_applied->display(voltApp);
}

void MainWindow::initHard()
{
    // init hard
    fControl->Initialize();

    // init the controls and start threads
    this->getMeasurments();
    this->getVoltAndCurr();
    this->getVoltAndCurrKeithley();
    this->getChillerStatus();
}

bool MainWindow::readXmlFile()
{
    fControl = new SystemControllerClass();
    QString cFilter  = "*.xml";
    QString cFileName = QFileDialog::getOpenFileName(this, "Open hardware description file", "./settings", cFilter);

    if (cFileName.isEmpty())
        return false;
    else {
        fControl->ReadXmlFile(cFileName.toStdString());
        fSources = fControl->getSourceNameVec();

        QHBoxLayout *low_layout = new QHBoxLayout;
        QHBoxLayout *high_layout = new QHBoxLayout;
        QHBoxLayout *rasp_layout = new QHBoxLayout;
        QHBoxLayout *chiller_layout = new QHBoxLayout;

        for(auto const& i: fControl->fGenericInstrumentMap){

            if( dynamic_cast<ControlTTiPower*>(i.second) ){

                gui_pointers_low_voltage.push_back(SetVoltageSource(low_layout, i.first, "TTI", 2));
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

                gui_pointers_high_voltage.push_back(SetVoltageSource(high_layout, i.first, "Keithley2410", 1));
                ui->groupBox_2->setLayout(high_layout);

                 connect(gui_pointers_high_voltage[0]->onoff_button, &QCheckBox::toggled, [this](bool pArg)
                 {this->on_OnOff_button_stateChanged("Keithley2410", 0, 0, pArg);});
            }

            if( dynamic_cast<Thermorasp*>(i.second) ){
                Thermorasp* rasp = dynamic_cast<Thermorasp*>(i.second);
                gui_raspberrys.push_back(SetRaspberryOutput(rasp_layout, rasp->getSensorNames(), i.first));
            }

            if( dynamic_cast<JulaboFP50*>(i.second) ){
                gui_chiller = SetChillerOutput(chiller_layout , i.first);

                connect(gui_chiller->onoff_button, &QCheckBox::toggled, [this](bool pArg)
                {this->on_OnOff_button_stateChanged("JulaboFP50", 0, 0, pArg);});
            }
        }

        //set layout to group box
        ui->groupBox->setLayout(low_layout);
        ui->groupBox_2->setLayout(high_layout);
        ui->groupBox_3->setLayout(rasp_layout);
        ui->groupBox_Chiller->setLayout(chiller_layout);
        
        commandListPage->setSystemController(fControl);
        if (fControl->getDaqModule() != nullptr)
            daqPage->setDAQModule(fControl->getDaqModule());

        return true;
    }
}

void MainWindow::on_read_conf_button_clicked()
{
    bool xml_was_read = false;
    try {
        // read the xml file
        if (not readXmlFile())
            return;
        xml_was_read = true;
        
        initHard();
            
            
        // enable back
        ui->tabWidget->setEnabled(true);
        ui->read_conf_button->setEnabled(false);
        
        if (fControl->getDaqModule() != nullptr)
            ui->DAQControl->setEnabled(true);
            
    } catch (const BurnInException& e) {
        cerr << "Error: " << e.what() << endl;
        
        commandListPage->setSystemController(nullptr);
        daqPage->setDAQModule(nullptr);
        
        QMessageBox dialog(this);
        dialog.critical(this, "Error", QString::fromStdString(e.what()));
        
        if (xml_was_read) {
            // Do a clean up of things that were created already
            if (fControl != nullptr)
                delete fControl;
            for (auto& p: gui_pointers_low_voltage)
                delete p;
            for (auto& p: gui_pointers_high_voltage)
                delete p;
            gui_pointers_low_voltage.clear();
            gui_pointers_high_voltage.clear();
            delete ui->groupBox->layout();
            delete ui->groupBox_2->layout();
            delete ui->groupBox_3->layout();
            delete ui->groupBox_Chiller->layout();
        }
    }
}

void MainWindow::app_quit() {
    // Set chillder temperature and turn off
    if (fControl != nullptr and fControl->countInstrument("JulaboFP50") > 0) {
        JulaboFP50* chiller = dynamic_cast<JulaboFP50*>(fControl->getGenericInstrObj("JulaboFP50"));
        chiller->SetWorkingTemperature(20);
        chiller->SetCirculatorOff();
    }
    
    // Turn off Keithley power
    if (fControl != nullptr and fControl->countInstrument("Keithley2410")) {
        ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(fControl->getGenericInstrObj("Keithley2410"));
        QEventLoop loop;
        connect(keithley, SIGNAL(powerStateChanged(bool, int)), &loop, SLOT(quit()));
        keithley->offPower();
        
        // Wait for power off
        loop.exec();
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
