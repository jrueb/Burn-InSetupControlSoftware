#ifndef COMMANDMODIFYDIALOG_H
#define COMMANDMODIFYDIALOG_H

#include <QDialog>
#include <tuple>
#include <map>
#include <QPair>
#include <QString>
#include <QStringList>
#include "general/commandprocessor.h"
#include "general/systemcontrollerclass.h"
#include "devices/environment/chiller.h"

namespace Ui {
class CommandModifyDialog;
}

class CommandModifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandModifyDialog(QWidget *parent);
    virtual ~CommandModifyDialog();
    
    static bool commandWait(QWidget *parent, BurnInWaitCommand *command);
    
    /* Voltage source dialogs */
    static bool commandVoltageSourceOutput(QWidget *parent, BurnInVoltageSourceOutputCommand *command, const SystemControllerClass *controller);
    static bool commandVoltageSourceSet(QWidget *parent, BurnInVoltageSourceSetCommand *command, const SystemControllerClass *controller);
    
    /* Chiller dialogs */
    static bool commandChillerOutput(QWidget *parent, BurnInChillerOutputCommand *command, const SystemControllerClass* controller);
    static bool commandChillerSet(QWidget *parent, BurnInChillerSetCommand *command, const SystemControllerClass* controller);
    
    /* DAQ dialogs */
    static bool commandDAQCmd(QWidget *parent, BurnInDAQCommand *command, const SystemControllerClass* controller);
    
    static bool modifyCommand(QWidget *parent, BurnInCommand* command, const SystemControllerClass* controller);

private:
    Ui::CommandModifyDialog *ui;
    
    class ModifyCommandHandler : public AbstractCommandHandler {
    public:
        ModifyCommandHandler(QWidget *parent_, bool* ok_, const SystemControllerClass* controller_);
        
        void handleCommand(BurnInWaitCommand& command) override;
        void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
        void handleCommand(BurnInVoltageSourceSetCommand& command) override;
        void handleCommand(BurnInChillerOutputCommand& command) override;
        void handleCommand(BurnInChillerSetCommand& command) override;
        void handleCommand(BurnInDAQCommand& command) override;
        
        QWidget* parent;
        bool* ok;
        const SystemControllerClass* controller;
    };
    
    static std::map<QString, QPair<int, PowerControlClass*>> _getAvailableVoltageSources(const SystemControllerClass *controller);
    static std::map<QString, Chiller*> _getAvailableChillers(const SystemControllerClass* controller);
    static QStringList _getDaqExecutables(const SystemControllerClass* controller);
};

#endif // COMMANDMODIFYDIALOG_H
