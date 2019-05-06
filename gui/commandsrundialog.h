#ifndef COMMANDSRUNDIALOG_H
#define COMMANDSRUNDIALOG_H

#include <QDialog>
#include <QThread>
#include <QDateTime>
#include <QString>
#include <QLabel>
#include <atomic>
#include "general/burnincommand.h"
#include "general/systemcontrollerclass.h"

namespace Ui {
class CommandsRunDialog;
}

class CommandExecuter : public QObject {
    Q_OBJECT

public:
    CommandExecuter(const QVector<BurnInCommand*>& commands, const SystemControllerClass* controller, QWidget *parent = 0);
    bool isPaused() const;
    bool isRunning() const;
    
public slots:
    void start();
    void togglePause();
    void abort();
    
signals:
    void commandStarted(int n, QDateTime dt);
    void commandFinished(int n, QDateTime dt);
    void commandStatusUpdate(int n, QString status);
    void allFinished();

private:
    QVector<BurnInCommand*> _commands;
    const SystemControllerClass* _controller;
    
    std::atomic<bool> _shouldAbort;
    std::atomic<bool> _shouldPause;
    std::atomic<bool> _isRunning;
    
    class CommandExecuteHandler : public AbstractCommandHandler {
    public:
        CommandExecuteHandler(CommandExecuter* executer, int n, const SystemControllerClass* controller);
        
        void handleCommand(BurnInWaitCommand& command) override;
        void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
        void handleCommand(BurnInVoltageSourceSetCommand& command) override;
        void handleCommand(BurnInChillerOutputCommand& command) override;
        void handleCommand(BurnInChillerSetCommand& command) override;
        
        bool error;
        
        const double VOLTAGESRC_EPSILON = 0.1; // V, for comparing two voltage values
        const double CHILLER_TEMP_EPSILON = 0.1; // °C, for comparing two temperature values
        const unsigned int WAIT_INTERVAL = 1; // s
        
    private:
        CommandExecuter* _executer;
        int _n;
        const SystemControllerClass* _controller;
        
        void _waitForVoltage(PowerControlClass* source, int output);
        void _waitForChiller(JulaboFP50* chiller);
    };
    
};

class CommandsRunDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandsRunDialog(const QVector<BurnInCommand*>& commands, const SystemControllerClass* controller, QWidget *parent = 0);
    ~CommandsRunDialog();
    
    void reject();

private slots:
    void on_abort_button_clicked();

    void on_pause_button_clicked();
    
    void onCommandStarted(int n, QDateTime dt);
    void onCommandFinished(int n, QDateTime dt);
    void onCommandStatusUpdate(int n, QString status);
    void onAllFinished();

private:
    Ui::CommandsRunDialog *ui;
    
    QVector<BurnInCommand*> _commands;
    CommandExecuter _executer;
    QThread _executer_thread;
    
    void _setupDisplays(const SystemControllerClass* controller);
    void _updateDisplayLabel(QLabel* label, std::string name, PowerControlClass* source);
    void _updateDisplayLabel(QLabel* label, std::string name, JulaboFP50* chiller);
    void _logMessage(QString message);
};

#endif // COMMANDSRUNDIALOG_H
