#include "commandsrundialog.h"
#include "ui_commandsrundialog.h"

CommandsRunDialog::CommandsRunDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandsRunDialog)
{
    ui->setupUi(this);
}

CommandsRunDialog::~CommandsRunDialog()
{
    delete ui;
}
