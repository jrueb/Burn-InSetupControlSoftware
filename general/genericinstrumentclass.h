#ifndef GENERICINSTRUMENTCLASS_H
#define GENERICINSTRUMENTCLASS_H

#include <string>

using namespace std;

class GenericInstrumentClass
{
public:
    GenericInstrumentClass();
    
    virtual ~GenericInstrumentClass() {
    
    };

    virtual void initialize() = 0;


};

#endif // GENERICINSTRUMENTCLASS_H
