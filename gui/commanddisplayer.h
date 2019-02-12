#ifndef COMMANDDISPLAYER_H
#define COMMANDDISPLAYER_H

#include <QString>
#include "general/burnincommand.h"


class CommandDisplayer : public AbstractCommandHandler
{
public:
    CommandDisplayer() {}
        
    void handleCommand(BurnInWaitCommand& command) override;
    void handleCommand(BurnInVoltageSourceOutputCommand& command) override;
    void handleCommand(BurnInVoltageSourceSetCommand& command) override;
    void handleCommand(BurnInChillerOutputCommand& command) override;
    void handleCommand(BurnInChillerSetCommand& command) override;
    
    QString display;
};

#endif // COMMANDDISPLAYER_H
