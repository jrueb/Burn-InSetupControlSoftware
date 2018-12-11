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

#ifndef __JULABOFP50_H
#define __JULABOFP50_H

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <fstream>

#include "genericinstrumentclass.h"

typedef const char* ioport_t;
class ComHandler;
class JulaboFP50: public GenericInstrumentClass
{
 public:

  JulaboFP50( ioport_t );
  JulaboFP50( const std::string& ioPort );
  
  void initialize();

  bool SetWorkingTemperature( const float ) const;
  bool SetPumpPressure( const unsigned int ) const;
  bool SetCirculatorOn( void ) const;
  bool SetCirculatorOff( void ) const;
  bool SetControlParameters( float, int, int ) const;

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
  bool LoadControlParametersAndApply( const std::string& ) const;
  void StripBuffer( char* ) const;
  
  static constexpr int FP50LowerTempLimit = -50;
  static constexpr int FP50UpperTempLimit =  55;

 private:

  std::string ioPort_;
  void Device_Init( void );
  ComHandler* comHandler_;
  bool isCommunication_;

};

#endif
