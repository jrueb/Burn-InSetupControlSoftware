#ifndef COMMANDSRUNDIALOG_H
#define COMMANDSRUNDIALOG_H

#include <QDialog>

namespace Ui {
class CommandsRunDialog;
}

class CommandsRunDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandsRunDialog(QWidget *parent = 0);
    ~CommandsRunDialog();

private:
    Ui::CommandsRunDialog *ui;
};

#endif // COMMANDSRUNDIALOG_H
