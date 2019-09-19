#include "lxicommunicator.h"
#include "general/BurnInException.h"

#include <QtGlobal>
#include <QThread>
#include <QMutexLocker>
#include <cstring>

LXICommunicator::LXICommunicator(const std::string& address, int port, bool raw) {
    _address = address;
    _port = port;
    _raw = raw;
    _lxidev = LXI_ERROR;
}

LXICommunicator::~LXICommunicator() {
    QMutexLocker locker(&_mutex);
    if (_lxidev != LXI_ERROR)
        lxi_disconnect(_lxidev);
}

void LXICommunicator::open() {
    // Currently installed LXI version accepts char* instead of const char*
    // Make a non-const copy as a workaround
    char* address_cstr = _get_cstr_copy(_address);
    
    QMutexLocker locker(&_mutex);
    if (_raw)
        _lxidev = lxi_connect(address_cstr, _port, nullptr, timeout, RAW);
    else
        _lxidev = lxi_connect(address_cstr, _port, nullptr, timeout, VXI11);
    delete[] address_cstr;
    
    if (_lxidev == LXI_ERROR)
        throw(BurnInException("Error while establishing LXI connection"));
}

void LXICommunicator::close() {
    QMutexLocker locker(&_mutex);
    lxi_disconnect(_lxidev);
    _lxidev = LXI_ERROR;
}

bool LXICommunicator::isOpen() const {
    return _lxidev != LXI_ERROR;
}

void LXICommunicator::send(const std::string& buf) const {
    Q_ASSERT_X(_lxidev != LXI_ERROR, "LXICommunicator::send", "connection must be open");
    char* cstr = _get_cstr_copy(buf + getSuffix());
    size_t len = buf.length() + getSuffix().length();
    QMutexLocker locker(&_mutex);
    qDebug("Send to %s %i: %s", _address.c_str(), _port, cstr);
    if (lxi_send(_lxidev, cstr, len, timeout) == LXI_ERROR) {
        delete[] cstr;
        throw(BurnInException("Error while sending through LXI connection"));
    }
    delete[] cstr;
}

std::string LXICommunicator::receive() const {
    Q_ASSERT_X(_lxidev != LXI_ERROR, "LXICommunicator::send", "connection must be open");
    char buf[1024];
    int num_bytes = sizeof(buf);
    std::string received;
    
    QMutexLocker locker(&_mutex);
    while (num_bytes == sizeof(buf)) {
        num_bytes = lxi_receive(_lxidev, buf, sizeof(buf), timeout);
        if (num_bytes == LXI_ERROR) {
            qCritical("Error when receiving from LXI connection %s %i", _address.c_str(), _port);
            return received;
        }
        received.append(buf, num_bytes);
    }
    qDebug("Received from %s %i: %s", _address.c_str(), _port, received.c_str());
    return received;
}

std::string LXICommunicator::query(const std::string& buf, int sleep_time) const {
    send(buf);
    QThread::msleep(sleep_time);
    return receive();
}

std::string LXICommunicator::getLocDisplay() const {
    std::string ret = "LXI ";
    ret += _address;
    ret += " ";
    ret += _port;
    return ret;
}

char* LXICommunicator::_get_cstr_copy(const std::string& str) const {
    char* cstr = new char[str.length() + 1];
    memcpy(cstr, str.c_str(), str.length() + 1);
    return cstr;
}
