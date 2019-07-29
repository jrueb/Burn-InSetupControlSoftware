#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLCDNumber>
#include <QCheckBox>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QLabel>

#include "general/systemcontrollerclass.h"
#include "devices/power/powercontrolclass.h"
#include "gui/commandlistpage.h"
#include "gui/daqpage.h"

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
     QCheckBox *onoff_button;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private slots:

    void initialize();

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
    
    void _connectTTi();
    void _connectKeithley();
    void _connectJulabo();
    void _connectThermorasp();

};

#endif // MAINWINDOW_H
