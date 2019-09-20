#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <string>

/**
 * Abstract class to communicate with devices
 */
class Communicator {
public:
    Communicator();
    virtual ~Communicator() {};
    
    /**
     * Open the connection to the device to communicate with. Normally
     * done during initialization.
     */
    virtual void open() = 0;
    
    /**
     * Close the connection to the device
     */
    virtual void close() = 0;
    
    /**
     * Return whether the connection is open
     */
    virtual bool isOpen() const = 0;
    
    /**
     * Send data to the device
     * @param buf Data to send
     */
    virtual void send(const std::string& buf) const = 0;
    
    /**
     * Receive data from the device
     * @return Data received
     */
    virtual std::string receive() const = 0;
    
    /**
     * First send data to the device, wait for a specific time and then
     * receive data from the device
     * @param buf Data to send. A suffix is attached, which is set using setSuffix
     * @param sleep_time Time to wait in ms
     * @return Data received
     */
    virtual std::string query(const std::string& buf, int sleep_time = 0) const = 0;
    
    /**
     * Get a representation of the connection or device readable by a 
     * human (e.g. an address)
     * @return The representation
     */
    virtual std::string getLocDisplay() const = 0;
    
    /**
     * Get the suffix that is attached to all data that is sent
     * @return The suffix
     */
    std::string getSuffix() const;
    
    /**
     * Set the suffix which gets attached to the end of all data that is
     * being sent. In most cases it is useful to set it to \n
     * @param suffix The suffix
     */
    void setSuffix(const std::string& suffix);
    
    /**
     * ms to wait for data and for the connection to open
     */
    int timeout = 1000;

private:
    std::string _suffix;
};

#endif // COMMUNICATOR_H
