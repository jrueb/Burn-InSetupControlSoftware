#ifndef COMMANDLISTPAGE_H
#define COMMANDLISTPAGE_H

#include <QAbstractListModel>
#include <QObject>
#include <QPushButton>
#include <QMenu>
#include <QListWidget>
#include "general/systemcontrollerclass.h"
#include "general/commandprocessor.h"

class CommandListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    CommandListItem(const BurnInCommand& command, const QString &text, QListWidget *parent = 0);
    
    BurnInCommand command;

signals:

public slots:

private:
};

class CommandListPage : public QObject
{
    Q_OBJECT
public:
    explicit CommandListPage(QWidget* commandListWidget, QObject *parent = nullptr);
    virtual ~CommandListPage();
    void setSystemController(const SystemControllerClass* controller);
    
private:
    QWidget* _commandListWidget;
    QPushButton* _add_command_button;
    QMenu* _add_command_menu;
    
    QListWidget* _commands_list;
    
    CommandProcessor* _proc;
    
    QMap<QString, QPair<int, PowerControlClass*>> _buildVoltageSourcesVector() const;

signals:

public slots:
    void onAddWait();
    void onAddVoltageSourceOutput();
    void onAddVoltageSourceAdd();
    void onAddChillerOutput();
    void onAddChillerSet();
};

#endif // COMMANDLISTPAGE_H
