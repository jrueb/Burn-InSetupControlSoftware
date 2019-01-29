#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <QObject>
#include <QVector>
#include "general/systemcontrollerclass.h"
#include "burnincommand.h"

class CommandProcessor : public QObject
{
    Q_OBJECT
public:
    explicit CommandProcessor(const SystemControllerClass* controller, QObject *parent = nullptr);
    
    QVector<BurnInCommandType> getAvailableCommands() const;
    std::map<std::string, PowerControlClass*> getAvailableVoltageSources() const;

signals:

public slots:

private:
    const SystemControllerClass* _controller;
};

#endif // COMMANDPROCESSOR_H
