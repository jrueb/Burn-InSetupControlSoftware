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

#ifndef __HUBERPETITEFLEUR_H
#define __HUBERPETITEFLEUR_H

#include "devices/environment/chiller.h"
#include "devices/ComHandler.h"
#include <string>
#include <QMutex>

class PetiteFleurComHandler;

class HuberPetiteFleur : public Chiller
{
  Q_OBJECT
  
 public:

  HuberPetiteFleur(const ioport_t ioPort);
  HuberPetiteFleur(const std::string& ioPort);
  
  void initialize();
  
  void refreshDeviceState();

  bool SetWorkingTemperature(const float); //SP@
  bool SetCirculatorOn(); //CA@ +00001
  bool SetCirculatorOff(); //CA@ +00000

  bool IsCommunication(void) const { return isCommunication_; }
  float GetBathTemperature(void) const; //TI?
  float GetWorkingTemperature(void) const; //SP?
  bool GetCirculatorStatus(void) const; //CA?
  
  virtual float GetMaxTemp() const;
  virtual float GetMinTemp() const;
  static constexpr int PetiteFleurLowerTempLimit = -40;
  static constexpr int PetiteFleurUpperTempLimit = 40;

 private:
  
  void StripBuffer(char*) const;
  void GetValue(const char* command, char* buffer) const;
  void SetAndConfirm(const char* command, char* buffer) const;
  int ToInteger(const char*) const;
  float ToFloat(const char*) const;

  std::string ioPort_;
  void Device_Init();
  ComHandler* comHandler_;
  mutable QMutex comMutex_;
  bool isCommunication_;
  
  bool alwaysEmit_;
  mutable bool circulatorOn_;
  mutable float workingTemperature_;
  mutable float bathTemperature_;
};

#endif
