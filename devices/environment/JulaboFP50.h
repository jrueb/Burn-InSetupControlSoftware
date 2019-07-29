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

#ifndef __JULABOFP50_H
#define __JULABOFP50_H

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <fstream>

#include <QMutex>

#include "devices/environment/chiller.h"

typedef const char* ioport_t;
class ComHandler;
class JulaboFP50: public Chiller
{
  Q_OBJECT
  
 public:

  JulaboFP50( ioport_t );
  JulaboFP50( const std::string& ioPort );
  
  void initialize();
  
  void refreshDeviceState();

  bool SetWorkingTemperature( const float );
  bool SetPumpPressure( const unsigned int );
  bool SetCirculatorOn( void );
  bool SetCirculatorOff( void );
  bool SetControlParameters( float, int, int );

  bool IsCommunication( void ) const { return isCommunication_; }
  float GetBathTemperature( void ) const;
  float GetSafetySensorTemperature( void ) const;
  float GetWorkingTemperature( void ) const;
  int GetHeatingPower( void ) const;
  unsigned int GetPumpPressure( void ) const;
  bool GetCirculatorStatus( void ) const;
  std::pair<int,std::string> GetStatus( void ) const;
  float GetProportionalParameter( void ) const;
  int GetIntegralParameter( void ) const;
  int GetDifferentialParameter( void ) const;
  
  bool SaveControlParameters( const std::string& ) const;
  bool LoadControlParametersAndApply( const std::string& );
  void StripBuffer( char* ) const;
  
  float GetMaxTemp(void) const;
  float GetMinTemp(void) const;
  static constexpr int FP50LowerTempLimit = -50;
  static constexpr int FP50UpperTempLimit =  55;
  
signals:
  void circulatorStatusChanged(bool on) const;
  void workingTemperatureChanged(float temperature) const;
  void bathTemperatureChanged(float temperature) const;
  void safetySensorTemperatureChanged(float temperature) const;
  void pumpPressureChanged(unsigned int pressureStage) const;

 private:
  void GetValue(const char* command, char* buffer) const;
  void SetAndConfirm(const char* first, const char* second, char* buffer) const;

  std::string ioPort_;
  void Device_Init( void );
  ComHandler* comHandler_;
  mutable QMutex comMutex_;
  bool isCommunication_;
  
  bool alwaysEmit_;
  mutable bool circulatorOn_;
  mutable float workingTemperature_;
  mutable float bathTemperature_;
  mutable float sensorTemperature_;
  mutable unsigned int pumpPressure_;

};

#endif
