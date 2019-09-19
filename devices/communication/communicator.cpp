#include "communicator.h"

Communicator::Communicator() {

}


std::string Communicator::getSuffix() const {
    return _suffix;
}
void Communicator::setSuffix(const std::string& suffix) {
    _suffix = suffix;
}
