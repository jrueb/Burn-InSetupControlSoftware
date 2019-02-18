#ifndef GENERICINSTRUMENTCLASS_H
#define GENERICINSTRUMENTCLASS_H

#include <QObject>
#include <string>

using namespace std;

class GenericInstrumentClass: public QObject
{
    Q_OBJECT
public:
    GenericInstrumentClass();
    
    virtual ~GenericInstrumentClass() {
    
    };

    virtual void initialize() = 0;


};

#endif // GENERICINSTRUMENTCLASS_H
