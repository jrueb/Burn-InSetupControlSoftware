#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include "QPushButton"
#include "QDoubleSpinBox"
#include "QLCDNumber"
#include "QCheckBox"
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QLabel>

#include "general/systemcontrollerclass.h"
#include "general/environmentcontrolclass.h"
#include "voltagecontrol/powercontrolclass.h"
#include "additionalwindow.h"
#include "additional/additionalthread.h"
#include "daqpage.h"

namespace Ui {
class MainWindow;
class SystemControllerClass;
}

struct output_pointer_t {
    QDoubleSpinBox *i_set;
    QDoubleSpinBox *v_set;
    QLCDNumber *i_applied;
    QLCDNumber *v_applied;
    QCheckBox *onoff_button;
};

struct output_Raspberry {
    QLayout *layout;
    QLabel *label;
    QLCDNumber *value;
};

struct output_Chiller{
     QDoubleSpinBox *setTemperature;
     QLCDNumber *bathTemperature;
     QLCDNumber *sensorTemperature;
     QLCDNumber *pressure;
     QLCDNumber *workingTemperature;
     QCheckBox *onoff_button;
     QLayout *layout;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int fRowMax;
    SystemControllerClass *fControl;
    AdditionalWindow *fAddWnd;

private slots:

    void on_listOfCommands_doubleClicked(const QModelIndex &pIndex);

    void on_readConfig_push_button_clicked();

    void on_Start_pushButton_clicked();

    void updateRaspWidget(QMap<QString, QString>);

    void updateTTiIWidget(PowerControlClass::fVACvalues *pObject, int dev_num);

    void updateKeithleyWidget(PowerControlClass::fVACvalues *pObject);

    void updateChillerWidget(QString pStr);

    void on_AddedComands_tabelView_doubleClicked(const QModelIndex &pIndex);

    void removeRow(int pRow);

    void on_AddedComands_tabelView_clicked(const QModelIndex &pIndex);

    void on_Up_pushButton_clicked();

    void on_Down_pushButton_clicked();

    void initHard();

    bool readXmlFile();

    void on_read_conf_button_clicked();
    
    void app_quit();

private:
    Ui::MainWindow *ui;
    DAQPage* daqPage;
    int fRowClicked;
    QStandardItemModel *model;
    QModelIndex fIndex;
    vector<string> fSources;
    vector<output_pointer_t*> gui_pointers_low_voltage;

    vector<output_pointer_t*> gui_pointers_high_voltage;

    void doListOfCommands();
    void getVoltAndCurr();
    void getVoltAndCurrKeithley();
    void getChillerStatus();
    void getMeasurments();
    QString transformQString(QString pStr);
    QString deleteSpaces(QString pStr);
    output_pointer_t SetSourceOutputLayout() const;
    output_pointer_t *SetVoltageSource(QLayout *pMainLayout, std::string pName, std::string pType,
                                       int pNoutputs);
    output_Raspberry* gui_raspberry;

    output_Raspberry setRaspberryLayout(string pName);

    output_Raspberry *SetRaspberryOutput(QLayout *pMainLayout , vector<string> pNames, string pNameGroupBox);

    output_Chiller *gui_chiller;

    output_Chiller setChilerLayout();

    output_Chiller* SetChillerOutput(QLayout *pMainLayout, std::string pName);

    void on_V_set_doubleSpinBox_valueChanged(string pSourceName , int pId , double pVolt);

    void on_OnOff_button_stateChanged(string pSourceName, int dev_num, int pId, bool pArg);

    void on_I_set_doubleSpinBox_valueChanged(string pSourceName , int pId, double pCurr);

private slots:
    void receiveOnOff(string pSourceName, bool pArg);
};

#endif // MAINWINDOW_H
