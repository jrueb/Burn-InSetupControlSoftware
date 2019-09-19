#ifndef LXICOMMUNICATOR_H
#define LXICOMMUNICATOR_H

#include "communicator.h"

#include <string>
#include <QMutex>

extern "C" {
	#include <lxi.h>
}

class LXICommunicator : public Communicator {
public:
    LXICommunicator(const std::string& _address, int port, bool raw);
    virtual ~LXICommunicator();
    
    LXICommunicator(LXICommunicator&& other) = delete;
    LXICommunicator(const LXICommunicator& other) = delete;
    LXICommunicator& operator=(const LXICommunicator& other) = delete;
    LXICommunicator& operator=(LXICommunicator&& other) = delete;
    
    void open() override;
    void close() override;
    bool isOpen() const override;
    void send(const std::string& buf) const override;
    std::string receive() const override;
    std::string query(const std::string& buf, int sleep_time = 0) const override;
    
    std::string getLocDisplay() const override;

private:
    char* _get_cstr_copy(const std::string& str) const;

    std::string _address;
    int _port;
    bool _raw;
    int _lxidev;
    
    mutable QMutex _mutex;
};

#endif // LXICOMMUNICATOR_H
