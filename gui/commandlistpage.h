#ifndef COMMANDLISTPAGE_H
#define COMMANDLISTPAGE_H

#include <memory>
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
    CommandListItem(std::shared_ptr<BurnInCommand> command_, QListWidget *parent = 0);
    
    void updateText();
    
    std::shared_ptr<BurnInCommand> command;

signals:

public slots:

private:
    class DisplayCommandHandler : public AbstractCommandHandler {
    public:
        DisplayCommandHandler() {};
        
        void handleCommand(BurnInWaitCommand& command) override;
        void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
        void handleCommand(BurnInVoltageSourceSetCommand& command) override;
        void handleCommand(BurnInChillerOutputCommand& command) override;
        void handleCommand(BurnInChillerSetCommand& command) override;
        
        QString display;
    };
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
    QWidget* _alter_command_buttons;
    QPushButton* _change_params_button;
    
    QListWidget* _commands_list;
    bool _commands_list_modified;
    
    CommandProcessor* _proc;
    
    QMap<QString, QPair<int, PowerControlClass*>> _buildVoltageSourcesVector() const;

signals:

public slots:
    void onItemSelectionChanged();
    void onCommandsListContextMenu(const QPoint& pos);
    void onDeleteButtonPressed();
    void onChangeParamsButtonPressed();
    void onCommandUpPressed();
    void onCommandDownPressed();
    void onSaveListPressed();
    void onOpenListPressed();
    void onAddWait();
    void onAddVoltageSourceOutput();
    void onAddVoltageSourceAdd();
    void onAddChillerOutput();
    void onAddChillerSet();
};

#endif // COMMANDLISTPAGE_H
