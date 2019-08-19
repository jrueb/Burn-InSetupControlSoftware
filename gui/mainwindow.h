#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QLCDNumber>
#include <QCheckBox>
#include <QGroupBox>

#include "general/logger.h"
#include "general/systemcontrollerclass.h"
#include "devices/power/powercontrolclass.h"
#include "devices/environment/thermorasp.h"
#include "devices/environment/chiller.h"
#include "gui/commandlistpage.h"
#include "gui/daqpage.h"

namespace Ui {
    class MainWindow;
    class SystemControllerClass;
};


class DeviceWidget : public QGroupBox {
    Q_OBJECT
public:
    DeviceWidget(const QString& title);
    virtual void initialize() = 0;
};


class VoltageSourceWidgetControls {
public:
    VoltageSourceWidgetControls();
    
    QDoubleSpinBox* i_set;
    QDoubleSpinBox* v_set;
    QLCDNumber* i_applied;
    QLCDNumber* v_applied;
    QCheckBox* onoff_button;
};


class VoltageSourceWidget : public DeviceWidget {
    Q_OBJECT

public:
    VoltageSourceWidget(const QString& title, PowerControlClass* device, bool settersAlwaysEnabled);
    void initialize();
    
private slots:
    void onOnOffToggled(int output, bool state);
    
private:
    std::vector<VoltageSourceWidgetControls> _controls;
    PowerControlClass* _device;
    bool _settersAlwaysEnabled;
};


class ThermoraspWidget : public DeviceWidget {
    Q_OBJECT

public:
    ThermoraspWidget(const QString& title, Thermorasp* device);
    void initialize();
    
private:
    Thermorasp* _device;
    std::vector<QLCDNumber*> _values;
};


class ChillerWidget : public DeviceWidget {
    Q_OBJECT
    
public:
    ChillerWidget(const QString& title, Chiller* device);
    void initialize();
    
private slots:
    void onOnOffToggled(bool state);
    
private:
    Chiller* _device;
    
    QDoubleSpinBox *_workingTemp;
    QLCDNumber *_bathTemp;
    QCheckBox *_onoffButton;
};

class PeltierWidget : public DeviceWidget {
    Q_OBJECT
    
public:
    PeltierWidget(const QString& title);
    void initialize();
    
private:
    QDoubleSpinBox* _workingTemp;
    QLCDNumber* _appliedVoltage;
    QLCDNumber* _sensorTemperature;
    QCheckBox* _onoffButton;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Logger* logger, QWidget *parent = nullptr);
    virtual ~MainWindow();

private slots:

    void initialize();
    void onNewLogMessage(QtMsgType type, const QString&);

    bool readXmlFile();

    void on_read_conf_button_clicked();
    
    void app_quit();

private:
    Ui::MainWindow *ui;
    
    Logger* _logger;
    std::vector<DeviceWidget*> _deviceWidgets;
    std::vector<VoltageSourceWidget*> _lowVoltageWidgets;
    std::vector<VoltageSourceWidget*> _highVoltageWidgets;
    std::vector<ThermoraspWidget*> _thermoraspWidgets;
    std::vector<ChillerWidget*> _chillerWidgets;

    SystemControllerClass *fControl;
    CommandListPage* commandListPage;
    DAQPage* daqPage;

};

#endif // MAINWINDOW_H
