/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2017 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

// Modified copy from https://github.com/DESY-FH-ELab/cmstkmodlab

//http://www.huber-online.com/download/manuals/archive/Manual_DataCommunication_EN.pdf

#include <string.h>
#include <cstdlib>
#include <sstream>

//#####################
// TODO:
// query error codes
//#####################

#include "HuberPetiteFleur.h"
#include "general/BurnInException.h"

const double TEMP_EPSILON = 1.e-3;

HuberPetiteFleur::HuberPetiteFleur(const ioport_t ioPort) {
  ioPort_ = ioPort;
  comHandler_ = nullptr;
  isCommunication_ = false;
  
  alwaysEmit_ = true;
  circulatorOn_ = false;
  workingTemperature_ = 0;
  bathTemperature_ = 0;
}
HuberPetiteFleur::HuberPetiteFleur(const std::string& ioPort) {
  ioPort_ = ioPort;
  comHandler_ = nullptr;
  isCommunication_ = false;
  
  alwaysEmit_ = true;
  circulatorOn_ = false;
  workingTemperature_ = 0;
  bathTemperature_ = 0;
}

void HuberPetiteFleur::initialize() {
  alwaysEmit_ = true;
  comHandler_ = new ComHandler(ioPort_.c_str());
  Device_Init();
  refreshDeviceState();
  alwaysEmit_ = false;
}

void HuberPetiteFleur::refreshDeviceState() {
  GetBathTemperature();
  GetWorkingTemperature();
  GetCirculatorStatus();
}

///
/// returns success flag
///
bool HuberPetiteFleur::SetWorkingTemperature(const float workingTemp) {
  Q_ASSERT(PetiteFleurLowerTempLimit <= workingTemp and workingTemp <= PetiteFleurUpperTempLimit);

  char buffer[1000];

  int iTemp = workingTemp * 100.;
  sprintf(buffer, "%+06d", iTemp);

  std::stringstream theCommand;
  theCommand << "SP@ " << buffer;

  memset(buffer, 0, sizeof(buffer));
  SetAndConfirm(theCommand.str().c_str(), buffer);
  
  int oTemp = ToInteger(buffer);

  if(iTemp != oTemp) {
    qCritical("HuberPetiteFleur reported wrong working temp \"%s\" after it was set to %f", buffer, workingTemp);
    return false;
  }
  
  float newtemp = static_cast<float>(oTemp) / 100;
  if (alwaysEmit_ or std::abs(workingTemperature_ - newtemp) > TEMP_EPSILON)
    emit workingTemperatureChanged(newtemp);
  workingTemperature_ = oTemp;
  
  return true;
}

///
/// return success flag
///
bool HuberPetiteFleur::SetCirculatorOn() {
  char buffer[1000];
  
  SetAndConfirm("CA@ +00001", buffer);

  if(ToInteger(buffer) != 1) {
    qCritical("HuberPetiteFleur reported wrong circulator status \"%s\" after it was set to 1", buffer);
    return false;
  }
  
  if (alwaysEmit_ or not circulatorOn_)
    emit circulatorStatusChanged(true);
  circulatorOn_ = true;

  return true;
}

///
/// return success flag
///
bool HuberPetiteFleur::SetCirculatorOff() {
  char buffer[1000];
  
  SetAndConfirm("CA@ +00000", buffer);

  if(ToInteger(buffer) != 1) {
    qCritical("HuberPetiteFleur reported wrong circulator status \"%s\" after it was set to 0", buffer);
    return false;
  }
  
  if (alwaysEmit_ or not circulatorOn_)
    emit circulatorStatusChanged(false);
  circulatorOn_ = false;

  return true;
}

///
///
///
float HuberPetiteFleur::GetBathTemperature() const {
  char buffer[1000];
  
  GetValue("TI?", buffer);
  float temp = ToFloat(buffer);
  
  if (alwaysEmit_ or abs(temp - bathTemperature_) > TEMP_EPSILON)
    emit workingTemperatureChanged(temp);
  bathTemperature_ = temp;

  return temp;
}

///
///
///
float HuberPetiteFleur::GetWorkingTemperature() const {
  char buffer[1000];
  
  GetValue("SP?", buffer);
  float temp = atof(buffer);
  
  if (alwaysEmit_ or abs(temp - workingTemperature_) > TEMP_EPSILON)
    emit workingTemperatureChanged(temp);
  workingTemperature_ = temp;

  return temp;
}

///
/// true = on / false = off
///
bool HuberPetiteFleur::GetCirculatorStatus() const {  
  char buffer[1000];
  
  GetValue("CA?", buffer);

  bool status = ToInteger(buffer);

  if (alwaysEmit_ or circulatorOn_ != status)
    emit circulatorStatusChanged(status);
  circulatorOn_ = status;
  
  return status;
}

///
/// strip trailing newlines & stuff
/// from machine answer
///
void HuberPetiteFleur::StripBuffer( char* buffer ) const {
  for( unsigned int c = 0; c < strlen( buffer ); ++c ) {
    if( '\n' == buffer[c] || '\r' == buffer[c] ) {
      buffer[c] = 0;
      break;
    }
  }
}

void HuberPetiteFleur::GetValue(const char* command, char* buffer) const {
  QMutexLocker locker( &comMutex_ );
  comHandler_->SendCommand( command );
  usleep( 250000 );
  comHandler_->ReceiveString( buffer );
  StripBuffer( buffer );
}

void HuberPetiteFleur::SetAndConfirm(const char* command, char* buffer) const {
  QMutexLocker locker( &comMutex_ );
  comHandler_->SendCommand( command );
  usleep( 250000 );
  comHandler_->ReceiveString( buffer );
  StripBuffer( buffer );
}

int HuberPetiteFleur::ToInteger(const char* buffer) const {
  std::string temp(buffer);
  temp.erase(0, 3);

  return std::atoi(temp.c_str());
}

float HuberPetiteFleur::ToFloat(const char* buffer) const {
  std::string temp(buffer);
  temp.erase(0, 3);

  return std::atof(temp.c_str()) / 100.;
}

///
/// read back software version
/// to check communication with device
///
void HuberPetiteFleur::Device_Init() {
  char buffer[1000];
  
  GetValue("CA?", buffer);
  std::string temp(buffer);

  if (temp.compare(0, 2, "CA") != 0) {
    isCommunication_ = false;
    
    throw BurnInException("Invalid or no device at address of HuberPetiteFleur chiller");
  }

  isCommunication_ = true;
}

float HuberPetiteFleur::GetMinTemp() const {
  return PetiteFleurLowerTempLimit;
}

float HuberPetiteFleur::GetMaxTemp() const {
  return PetiteFleurUpperTempLimit;
}
