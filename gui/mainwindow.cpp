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
    fRowMax = 1;
    fRowClicked = 0;


    ui->setupUi(this);
    daqPage = new DAQPage(ui->DAQControl);

    fControl = nullptr;

    model = new QStandardItemModel(this);

    QStandardItem *cItem1 = new QStandardItem("Added commands:");
    model->setItem( 0 , 0 , cItem1);
    model->index(0 , 0);
    QStandardItem *cItem2 = new QStandardItem("Start time:");
    model->setItem( 0 , 1 , cItem2);
    model->index(0 , 1);
    QStandardItem *cItem3 = new QStandardItem("End time:");
    model->setItem( 0 , 2 , cItem3);
    model->index(0 , 2);
    QStandardItem *cItem4 = new QStandardItem("Status:");
    model->setItem( 0 , 3 , cItem4);
    model->index(0 , 3);

    ui->AddedComands_tabelView->setModel(model);
    ui->AddedComands_tabelView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->AddedComands_tabelView->horizontalHeader()->setStretchLastSection(true);
    ui->AddedComands_tabelView->resizeColumnsToContents();
    ui->AddedComands_tabelView->verticalHeader()->setVisible(false);
    ui->AddedComands_tabelView->horizontalHeader()->setVisible(false);
    ui->AddedComands_tabelView->setDragEnabled(true);
    ui->AddedComands_tabelView->setAcceptDrops(true);
    ui->AddedComands_tabelView->setDropIndicatorShown(true);

    // default disabled everything
    ui->tabWidget->setEnabled(false);
    ui->Down_pushButton->setEnabled(false);
    ui->Up_pushButton->setEnabled(false);
    ui->Start_pushButton->setEnabled(false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

output_pointer_t MainWindow::SetSourceOutputLayout(std::string pType)
{
    // create pointer list
    output_pointer_t cOutputPointers;

    // create layout
    cOutputPointers.layout = new QVBoxLayout;

    // type of the power source
    QLabel *type = new QLabel(pType.c_str());
    type->setMaximumHeight(20);
    cOutputPointers.layout->addWidget(type);

    // i/v set
    cOutputPointers.i_set = new QDoubleSpinBox();
    cOutputPointers.i_set->setMaximumHeight(20);
    cOutputPointers.i_set->setDecimals(3);
    cOutputPointers.i_set->setSuffix(" A");
    cOutputPointers.layout->addWidget(cOutputPointers.i_set);
    cOutputPointers.v_set = new QDoubleSpinBox();
    cOutputPointers.v_set->setMaximumHeight(20);
    cOutputPointers.v_set->setMinimum(-100000);
    cOutputPointers.v_set->setDecimals(3);
    cOutputPointers.v_set->setSuffix(" V");
    cOutputPointers.layout->addWidget(cOutputPointers.v_set);

    // applied
    cOutputPointers.i_applied = new QLCDNumber();
    cOutputPointers.i_applied->setMaximumHeight(20);
    cOutputPointers.i_applied->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.layout->addWidget(cOutputPointers.i_applied);
    cOutputPointers.v_applied = new QLCDNumber();
    cOutputPointers.v_applied->setMaximumHeight(20);
    cOutputPointers.v_applied->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.layout->addWidget(cOutputPointers.v_applied);

    // on off
    cOutputPointers.onoff_button = new QCheckBox("On");
    cOutputPointers.onoff_button->setMaximumHeight(20);
    cOutputPointers.layout->addWidget(cOutputPointers.onoff_button);

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

    cOutputPointers.layout = new QVBoxLayout;


    cOutputPointers.setTemperature = new QDoubleSpinBox();
    cOutputPointers.setTemperature->setMaximumHeight(20);
    cOutputPointers.setTemperature->setMinimum(-100000);
    cOutputPointers.layout->addWidget(cOutputPointers.setTemperature);


    cOutputPointers.bathTemperature = new QLCDNumber();
    cOutputPointers.bathTemperature->setMaximumHeight(20);
    cOutputPointers.bathTemperature->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.layout->addWidget(cOutputPointers.bathTemperature);

    cOutputPointers.workingTemperature = new QLCDNumber();
    cOutputPointers.workingTemperature->setMaximumHeight(20);
    cOutputPointers.workingTemperature->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.layout->addWidget(cOutputPointers.workingTemperature);

    cOutputPointers.sensorTemperature = new QLCDNumber();
    cOutputPointers.sensorTemperature->setMaximumHeight(20);
    cOutputPointers.sensorTemperature->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.layout->addWidget(cOutputPointers.sensorTemperature);

    cOutputPointers.pressure = new QLCDNumber();
    cOutputPointers.pressure->setMaximumHeight(20);
    cOutputPointers.pressure->setSegmentStyle(QLCDNumber::Flat);
    cOutputPointers.layout->addWidget(cOutputPointers.pressure);


    // on off
    cOutputPointers.onoff_button = new QCheckBox("On");
    cOutputPointers.onoff_button->setMaximumHeight(20);
    cOutputPointers.layout->addWidget(cOutputPointers.onoff_button);

    return cOutputPointers;
}

output_Chiller* MainWindow::SetChillerOutput(QLayout *pMainLayout, string pName)
{
    // output pointers
    output_Chiller *cOutputPointers = new output_Chiller[1];

    // horizontal layout
    QGroupBox *group_box = new QGroupBox(pName.c_str());
    QHBoxLayout *group_box_layout = new QHBoxLayout;

    QSize size(80,20);

    // set the labels
    QVBoxLayout *label_layout = new QVBoxLayout;
    label_layout->setMargin(10);


    QLabel *label_t_set = new QLabel("T(set), °C:");
    label_t_set->setMinimumSize(size);
    label_layout->addWidget(label_t_set);
    QLabel *label_t_bath = new QLabel("T(bath), °C:");
    label_t_bath->setMinimumSize(size);
    label_layout->addWidget(label_t_bath);
    QLabel *label_t_working = new QLabel("T(working), °C:");
    label_t_working->setMinimumSize(size);
    label_layout->addWidget(label_t_working);
    QLabel *label_t_sensor = new QLabel("T(sensor), °C:");
    label_t_sensor->setMinimumSize(size);
    label_layout->addWidget(label_t_sensor);
    QLabel *label_pressure = new QLabel("P, Pa:");
    label_pressure->setMinimumSize(size);
    label_layout->addWidget(label_pressure);
    QLabel *label_on_off = new QLabel("On/Off:");
    label_on_off->setMinimumSize(size);
    label_layout->addWidget(label_on_off);
    // add stretchconnect(fControl, SIGNAL(sendOnOff(string,bool)) , this , SLOT(receiveOnOff(string,bool)));
    label_layout->addSpacing(15);

    // now set
    group_box_layout->addItem(label_layout);

    cOutputPointers[0] = setChilerLayout();
    group_box_layout->addItem(cOutputPointers[0].layout);

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
    QHBoxLayout *group_box_layout = new QHBoxLayout;

    QSize size(80,20);

    // set the labels
    QVBoxLayout *label_layout = new QVBoxLayout;
    QLabel *label_type = new QLabel("Type:");
    label_type->setMinimumSize(size);
    label_layout->addWidget(label_type);
    QLabel *label_i_set = new QLabel("Current limit:");
    label_i_set->setMinimumSize(size);
    label_layout->addWidget(label_i_set);
    QLabel *label_v_set = new QLabel("Voltage:");
    label_v_set->setMinimumSize(size);
    label_layout->addWidget(label_v_set);
    QLabel *label_i_applied = new QLabel("Current limit applied:");
    label_i_applied->setMinimumSize(size);
    label_layout->addWidget(label_i_applied);
    QLabel *label_v_applied = new QLabel("Voltage applied:");
    label_v_applied->setMinimumSize(size);
    label_layout->addWidget(label_v_applied);
    QLabel *label_onoff = new QLabel("On/Off:");
    label_onoff->setMinimumSize(size);
    label_layout->addWidget(label_onoff);
    // now set
    group_box_layout->addItem(label_layout);

    // set the outputs
    for (int i = 0; i < pNoutputs; i++) {
        cOutputPointers[i] = SetSourceOutputLayout(pType);
        group_box_layout->addItem(cOutputPointers[i].layout);
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

//creates a List with all commands
void MainWindow::doListOfCommands()
{
    QStandardItemModel *cModel = new QStandardItemModel(this);

    QStandardItem *cItem1 = new QStandardItem("Set Temperature (°C)");
    cModel->setItem( 0 , cItem1);
    cModel->index( 0 , 0);

    QStandardItem *cItem2 = new QStandardItem("Wait (Sec)");
    cModel->setItem( 1 , cItem2);
    cModel->index( 1 , 0);

    for(std::vector<string>::size_type i = 2 ; i != fSources.size()+2 ; i++){
        QString cStr = QString::fromStdString(fSources[i-2]);
        QStandardItem *cItem3 = new QStandardItem("On  " + cStr + "  power supply");
        cModel->setItem(i, cItem3);
        cModel->index(i , 0);

    }
    for(std::vector<string>::size_type i = fSources.size() + 2 ; i != 2*fSources.size() + 2 ; i++){
        QString cStr = QString::fromStdString(fSources[i-fSources.size() - 2]);
        QStandardItem *cItem4 = new QStandardItem("Off  " + cStr + "  power supply");
        cModel->setItem(i, cItem4);
        cModel->index(i , 0);
    }

    ui->listOfCommands->setModel(cModel);
    ui->listOfCommands->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
        PowerControlClass::fVACvalues* vals = ttidev->getVoltAndCurr();
        gui_pointers_low_voltage[dev_num][1].i_set->setValue(vals->pISet1);
        gui_pointers_low_voltage[dev_num][1].v_set->setValue(vals->pVSet1);
        gui_pointers_low_voltage[dev_num][0].i_set->setValue(vals->pISet2);
        gui_pointers_low_voltage[dev_num][0].v_set->setValue(vals->pVSet2);
        
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
    connect(cThread, SIGNAL(sendToThread(PowerControlClass::fVACvalues*, int)),this,
            SLOT(updateTTiIWidget(PowerControlClass::fVACvalues*, int)));
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
    connect(cThread, SIGNAL(updatedThermorasp(QMap<QString, QString>)), this, SLOT(updateRaspWidget(QMap<QString, QString>)));
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
    PowerControlClass::fVACvalues* vals = keihleydev->getVoltAndCurr();
    
    bool blocked = gui_pointers_high_voltage[0]->i_set->signalsBlocked();
    gui_pointers_high_voltage[0]->i_set->blockSignals(true);
    gui_pointers_high_voltage[0]->i_set->setValue(vals->pISet1);
    gui_pointers_high_voltage[0]->i_set->blockSignals(blocked);
    
    blocked = gui_pointers_high_voltage[0]->v_set->signalsBlocked();
    gui_pointers_high_voltage[0]->v_set->blockSignals(true);
    gui_pointers_high_voltage[0]->v_set->setValue(vals->pVSet1);
    gui_pointers_high_voltage[0]->v_set->blockSignals(blocked);
    
    AdditionalThread *cThread  = new AdditionalThread("C", fControl);
    QThread *cQThread = new QThread();
    connect(cQThread , SIGNAL(started()), cThread, SLOT(getVACKeithley()));
    connect(cThread, SIGNAL(sendToThreadKeithley(PowerControlClass::fVACvalues*)),this,
            SLOT(updateKeithleyWidget(PowerControlClass::fVACvalues*)));
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

//func which adds command to the list and vector from listOfCommands to AddedWidget
void MainWindow::on_listOfCommands_doubleClicked(const QModelIndex &pIndex)
{
    SystemControllerClass::fParameters cObj;
    QString cValue;
    QString cStr = pIndex.data().toString();
    //another window to set the value
    fAddWnd = new AdditionalWindow(this);
    if(cStr == "Set Temperature (°C)" || cStr == "Wait (Sec)"){
        fAddWnd->show();
        fAddWnd->exec();
        cValue = fAddWnd->getValue();
        cObj.cName = cStr.toStdString();
        cObj.cValue = cValue.toDouble();
        fControl->fListOfCommands.push_back(cObj);
        cStr = cStr + " =" + cValue;
    }
    //
    else{
        cObj.cName = cStr.toStdString();
        cObj.cValue = 0;
        fControl->fListOfCommands.push_back(cObj);
    }
    //addes object to the added commands
    QStandardItem *cItem = new QStandardItem(cStr);
    model->setItem(fRowMax , 0, cItem);
    model->index(fRowMax, 0);
    fRowMax++;
    ui->AddedComands_tabelView->resizeColumnsToContents();
    ui->Down_pushButton->setEnabled(true);
    ui->Up_pushButton->setEnabled(true);
}

//reads file and writes it to AddWidget, QString into name field of fParam and string after = to value field
void MainWindow::on_readConfig_push_button_clicked()
{
    SystemControllerClass::fParameters cObj;
    QString cStr;
    vector<QString> *cVec = new vector<QString>();

    cVec = fControl->readFile();
    for(vector<QString>::iterator iter = cVec->begin(); iter != cVec->end(); ++iter){
        cStr = (*iter);
        string cString = cStr.toStdString();
        size_t cPos = cString.find("=");
        if( cPos ){
            string cPosStr = cString.substr(cPos+1);
            QString cStrTemp = QString::fromStdString(cPosStr);
            cString = cString.substr(0 , cPos);
            cObj.cName = cString;
            cObj.cValue = cStrTemp.toDouble();
            fControl->fListOfCommands.push_back(cObj);
        }
        else{
            cObj.cName = cStr.toStdString();
            cObj.cValue = 0;
            fControl->fListOfCommands.push_back(cObj);
        }
    }
    for(vector<QString>::iterator iter = cVec->begin(); iter != cVec->end(); ++iter){
        cStr = (*iter);
        QStandardItem *cItem = new QStandardItem(cStr);
        model->setItem(fRowMax , 0, cItem);
        model->index(fRowMax , 0);
        fRowMax++;
        ui->AddedComands_tabelView->setModel(model);
        ui->AddedComands_tabelView->resizeColumnsToContents();
    }
}

//starts making list of commands
void MainWindow::on_Start_pushButton_clicked()
{
    fControl->startDoingList();
}

//reads sensor on rasp and sets info to Raspberry sensors
void MainWindow::updateRaspWidget(QMap<QString, QString> readings) {
    int i = 0;
    for (const string& name: fControl->fRaspberrySensorsNames) {
        gui_raspberry[i].value->display(readings[QString::fromStdString(name)]);
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

//deletes highlighted item from the table(double-click on item)
void MainWindow::on_AddedComands_tabelView_doubleClicked(const QModelIndex &pIndex)
{
    int cRow = pIndex.row();
    QMessageBox cMsgBox;
    cMsgBox.setText("Are you sure you want to delete this command?");
    cMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    cMsgBox.setIcon(QMessageBox::Warning);
    cMsgBox.setDefaultButton(QMessageBox::Ok);
    int res = cMsgBox.exec();
    if (res == QMessageBox::Ok)
        removeRow(cRow);
}

//removes row from the table with added commands
void MainWindow::removeRow(int pRow)
{
    ui->AddedComands_tabelView->model()->removeRow(pRow);
    vector<SystemControllerClass::fParameters>::iterator iter = fControl->fListOfCommands.begin();
    fControl->fListOfCommands.erase(iter + pRow - 1);
    fRowMax--;
}

void MainWindow::on_AddedComands_tabelView_clicked(const QModelIndex &pIndex)
{
    fIndex = pIndex;
}

//moves command up when up push button clicked
void MainWindow::on_Up_pushButton_clicked()
{
    //get an index of the row
    int cRowIndex = fIndex.row();

    if( cRowIndex > 1 ){
        //take text from clicked item
        QString cData = ui->AddedComands_tabelView->model()->data(fIndex).toString();

        //take text from item that above than clicked item
        QString cTempData = ui->AddedComands_tabelView->model()->data(ui->AddedComands_tabelView->
                                                                      model()->index(cRowIndex-1, 0)).toString();
        //make new items
        QStandardItem *cItem = new QStandardItem(cData);
        QStandardItem *cItemTemp = new QStandardItem(cTempData);

        //set items to the model
        model->setItem(cRowIndex-1 , cItem);
        model->index(cRowIndex-1 , 0);
        model->setItem(cRowIndex , cItemTemp);
        model->index(cRowIndex , 0);

        //set information from items to fObjParam struct(changes position of items in vector)
        SystemControllerClass::fParameters cObject;
        vector<SystemControllerClass::fParameters>::iterator cIter = fControl->fListOfCommands.begin() + cRowIndex -1;
        vector<SystemControllerClass::fParameters>::iterator cIterTemp = fControl->fListOfCommands.begin() +cRowIndex -2;

        cObject.cName = (*cIter).cName;
        cObject.cValue = (*cIter).cValue;

        (*cIter).cName = (*cIterTemp).cName;
        (*cIter).cValue = (*cIterTemp).cValue;

        (*cIterTemp).cName = cObject.cName;
        (*cIterTemp).cValue = cObject.cValue;
    }
}

//moves the command down when down push button clicked
void MainWindow::on_Down_pushButton_clicked()
{

    int cRowIndex = fIndex.row();

    if(cRowIndex < fRowMax - 1 && cRowIndex > 0){
        QString cData = ui->AddedComands_tabelView->model()->data(fIndex).toString();
        QString cTempData = ui->AddedComands_tabelView->model()->data(ui->AddedComands_tabelView->
                                                                      model()->index(cRowIndex+1, 0)).toString();
        QStandardItem *cItem = new QStandardItem(cData);
        QStandardItem *cItemTemp = new QStandardItem(cTempData);
        model->setItem(cRowIndex+1 , cItem);
        model->index(cRowIndex+1 , 0);
        model->setItem(cRowIndex , cItemTemp);
        model->index(cRowIndex , 0);

        SystemControllerClass::fParameters cObject;
        vector<SystemControllerClass::fParameters>::iterator cIter = fControl->fListOfCommands.begin() + cRowIndex -1;
        vector<SystemControllerClass::fParameters>::iterator cIterTemp = fControl->fListOfCommands.begin() +cRowIndex;

        cObject.cName = (*cIter).cName;
        cObject.cValue = (*cIter).cValue;

        (*cIter).cName = (*cIterTemp).cName;
        (*cIter).cValue = (*cIterTemp).cValue;

        (*cIterTemp).cName = cObject.cName;
        (*cIterTemp).cValue = cObject.cValue;
    }
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

void MainWindow::receiveOnOff(string pSourceName, bool pArg)
{
    if(pSourceName == "Keithley2410")
        gui_pointers_high_voltage[0]->onoff_button->setChecked(pArg);
    if(pSourceName == "TTI1"){
        gui_pointers_low_voltage[0][0].onoff_button->setChecked(pArg);
        gui_pointers_low_voltage[0][1].onoff_button->setChecked(pArg);
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

void MainWindow::updateTTiIWidget(PowerControlClass::fVACvalues* pObject, int dev_num)
{
    // The TTis can not output negative voltages. Sometimes they report small
    // negative values like -0.005 V. Use abs refresh the display anyway.
    gui_pointers_low_voltage[dev_num][1].i_applied->display(abs(pObject->pIApp1));
    gui_pointers_low_voltage[dev_num][1].v_applied->display(abs(pObject->pVApp1));

    gui_pointers_low_voltage[dev_num][0].i_applied->display(abs(pObject->pIApp2));
    gui_pointers_low_voltage[dev_num][0].v_applied->display(abs(pObject->pVApp2));
    
    delete pObject;
}

void MainWindow::updateKeithleyWidget(PowerControlClass::fVACvalues* pObject)
{
    gui_pointers_high_voltage[0]->i_applied->display(pObject->pIApp1);
    gui_pointers_high_voltage[0]->v_applied->display(pObject->pVApp1);
    
    delete pObject;
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

    connect(fControl, SIGNAL(sendOnOff(string,bool)) , this , SLOT(receiveOnOff(string,bool)));
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
                gui_raspberry = SetRaspberryOutput(rasp_layout , fControl->fRaspberrySensorsNames , i.first);
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
        
        if (fControl->getDaqModule() != nullptr)
            daqPage->setDAQModule(fControl->getDaqModule());

        //make a list with commands( depends on the devices we have read )
        this->doListOfCommands();
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
        ui->Start_pushButton->setEnabled(true);
        ui->read_conf_button->setEnabled(false);
        
        if (fControl->getDaqModule() != nullptr)
            ui->DAQControl->setEnabled(true);
            
    } catch (BurnInException e) {
        cerr << "Error: " << e.what() << endl;
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
        connect(keithley, SIGNAL(outputStateChanged(bool)), &loop, SLOT(quit()));
        keithley->offPower();
        
        // Wait for power off
        loop.exec();
    }
}
