#ifndef COMMANDMODIFYDIALOG_H
#define COMMANDMODIFYDIALOG_H

#include <QDialog>
#include <tuple>
#include <QMap>
#include <QPair>
#include <QString>
#include <general/commandprocessor.h>

namespace Ui {
class CommandModifyDialog;
}

class CommandModifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandModifyDialog(QWidget *parent);
    virtual ~CommandModifyDialog();
    
    static unsigned int commandWait(QWidget *parent, bool* ok, int value = 60);
    
    /* Voltage source dialogs */
    static std::tuple<QString, bool> commandVoltageSourceOutput(QWidget *parent, const QList<QString>& voltageSources, bool* ok, QString source = "", bool on = true);
    static std::tuple<QString, double> commandVoltageSourceSet(QWidget *parent, const QList<QString>& voltageSources, bool* ok, QString source = "", double value = 0);
    
    /* Chiller dialogs. Expects only one chiller */
    static bool commandChillerOutput(QWidget *parent, bool* ok, bool on = true);
    static double commandChillerSet(QWidget *parent, bool* ok, double value = 0);
    
    static void modifyCommand(QWidget *parent, bool* ok, BurnInCommand* command, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources);

private:
    Ui::CommandModifyDialog *ui;
    
    class ModifyCommandHandler : public AbstractCommandHandler {
    public:
        ModifyCommandHandler(QWidget *parent_, bool* ok_, const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources);
        
        void handleCommand(BurnInWaitCommand& command) override;
        void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
        void handleCommand(BurnInVoltageSourceSetCommand& command) override;
        void handleCommand(BurnInChillerOutputCommand& command) override;
        void handleCommand(BurnInChillerSetCommand& command) override;
        
        QWidget* parent;
        bool* ok;
        const QMap<QString, QPair<int, PowerControlClass*>>& voltageSources;
    };
};

#endif // COMMANDMODIFYDIALOG_H
