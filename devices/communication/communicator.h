#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <string>

class Communicator {
public:
    Communicator();
    virtual ~Communicator() {};
    
    virtual void open() = 0;
    virtual void close() = 0;
    
    virtual void send(const std::string& buf) const = 0;
    virtual std::string receive() const = 0;
    virtual std::string query(const std::string& buf, int sleep_time = 0) const = 0;
    
    virtual std::string getLocDisplay() const = 0;
    
    std::string getSuffix() const;
    void setSuffix(const std::string& suffix);
    
    int timeout = 1000;

private:
    std::string _suffix;
};

#endif // COMMUNICATOR_H
