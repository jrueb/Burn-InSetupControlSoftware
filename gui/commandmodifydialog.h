#ifndef COMMANDMODIFYDIALOG_H
#define COMMANDMODIFYDIALOG_H

#include <QDialog>
#include <tuple>
#include <QString>

namespace Ui {
class CommandModifyDialog;
}

class CommandModifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandModifyDialog(QWidget *parent);
    virtual ~CommandModifyDialog();
    
    static unsigned int commandWait(QWidget *parent, bool* ok);
    
    /* Voltage source dialogs */
    static std::tuple<QString, bool> commandVoltageSourceOutput(QWidget *parent, const QList<QString>& voltageSources, bool* ok);
    static std::tuple<QString, double> commandVoltageSourceSet(QWidget *parent, const QList<QString>& voltageSources, bool* ok);
    
    /* Chiller dialogs. Expects only one chiller */
    static bool commandChillerOutput(QWidget *parent, bool* ok);
    static int commandChillerSet(QWidget *parent, bool* ok);

private:
    Ui::CommandModifyDialog *ui;
};

#endif // COMMANDMODIFYDIALOG_H
