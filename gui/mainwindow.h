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
#include "additional/additionalthread.h"
#include "commandlistpage.h"
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
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private slots:

    void updateRaspWidget(quint64 n, QMap<QString, QString>);

    void updateTTiIWidget(PowerControlClass::fVACvalues *pObject, int dev_num);

    void updateKeithleyWidget(PowerControlClass::fVACvalues *pObject);

    void updateChillerWidget(QString pStr);

    void initHard();

    bool readXmlFile();

    void on_read_conf_button_clicked();
    
    void app_quit();

private:
    int fRowMax;
    SystemControllerClass *fControl;
    Ui::MainWindow *ui;
    CommandListPage* commandListPage;
    DAQPage* daqPage;
    vector<string> fSources;
    vector<output_pointer_t*> gui_pointers_low_voltage;

    vector<output_pointer_t*> gui_pointers_high_voltage;

    void getVoltAndCurr();
    void getVoltAndCurrKeithley();
    void getChillerStatus();
    void getMeasurments();
    output_pointer_t SetSourceOutputLayout() const;
    output_pointer_t *SetVoltageSource(QLayout *pMainLayout, std::string pName, std::string pType,
                                       int pNoutputs);
    vector<output_Raspberry*> gui_raspberrys;

    output_Raspberry setRaspberryLayout(string pName);

    output_Raspberry *SetRaspberryOutput(QLayout *pMainLayout , vector<string> pNames, string pNameGroupBox);

    output_Chiller *gui_chiller;

    output_Chiller setChilerLayout();

    output_Chiller* SetChillerOutput(QLayout *pMainLayout, std::string pName);

    void on_V_set_doubleSpinBox_valueChanged(string pSourceName , int pId , double pVolt);

    void on_OnOff_button_stateChanged(string pSourceName, int dev_num, int pId, bool pArg);

    void on_I_set_doubleSpinBox_valueChanged(string pSourceName , int pId, double pCurr);

};

#endif // MAINWINDOW_H
