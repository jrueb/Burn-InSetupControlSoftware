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

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <fstream>

#include "ComHandler.h"

#include "JulaboFP50.h"
#include "BurnInException.h"

//#####################
// TODO:
// query error codes
//#####################

const double TEMP_EPSILON = 1.e-3;

///
///
///
JulaboFP50::JulaboFP50( const ioport_t ioPort )

{
  ioPort_ = ioPort;
  comHandler_ = nullptr;
  isCommunication_ = false;
  
  alwaysEmit_ = true;
  circulatorOn_ = false;
  workingTemperature_ = 0;
  bathTemperature_ = 0;
  sensorTemperature_ = 0;
  pumpPressure_ = 0;
}

JulaboFP50::JulaboFP50(const std::string& ioPort) {
  ioPort_ = ioPort;
  comHandler_ = nullptr;
  isCommunication_ = false;
  
  alwaysEmit_ = true;
  circulatorOn_ = false;
  workingTemperature_ = 0;
  bathTemperature_ = 0;
  sensorTemperature_ = 0;
  pumpPressure_ = 0;
}

void JulaboFP50::initialize() {
  alwaysEmit_ = true;
  comHandler_ = new ComHandler( ioPort_.c_str() );
  Device_Init();
  refreshDeviceState();
  alwaysEmit_ = false;
}

void JulaboFP50::refreshDeviceState() {
  GetBathTemperature();
  GetSafetySensorTemperature();
  GetWorkingTemperature();
  GetPumpPressure();
  GetCirculatorStatus();
}

///
/// returns success flag
///
bool JulaboFP50::SetWorkingTemperature( const float workingTemp ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::SetWorkingTemp] -- DEBUG: Called." << std::endl;
  #endif

  if( workingTemp > FP50UpperTempLimit || workingTemp < FP50LowerTempLimit ) {
    std::cerr << " [JulaboFP50::SetWorkingTemp] ** ERROR: working temp T="
              << workingTemp << " exceeds soft limits." << std::endl;
    std::cerr << "  > (s. JulaboFP50 class definition)" << std::endl;
    return false;
  }

  char buffer[1000];

  std::stringstream theCommand;
  theCommand << "out_sp_00 " << workingTemp;
  SetAndConfirm(theCommand.str().c_str(), "in_sp_00", buffer);
  float newtemp = atof( buffer );

  if( fabs( workingTemp - newtemp ) > TEMP_EPSILON ) {
    std::cerr << " [JulaboFP50::SetWorkingTemp] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: T=" << workingTemp << " but received (string):"
              << buffer << "." << std::endl;
    return false;
  }
  
  if (alwaysEmit_ or std::abs(workingTemperature_ - newtemp) > TEMP_EPSILON)
    emit workingTemperatureChanged(newtemp);
  workingTemperature_ = newtemp;

  return true;
}

///
/// return success flag
/// pressureStage = 1..4
///
bool JulaboFP50::SetPumpPressure( const unsigned int pressureStage ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::SetPumpPressure] -- DEBUG: Called." << std::endl;
  #endif

  if( pressureStage < 1 || pressureStage > 4 ) {
    std::cerr << " [JulaboFP50::SetPumpPressure] ** ERROR: Invalid pressure stage: "
              << pressureStage << "." << std::endl;
    return false;
  }
  
  char buffer[1000];

  std::stringstream theCommand;
  theCommand << "out_sp_07 " << pressureStage;
  SetAndConfirm(theCommand.str().c_str(), "in_sp_07", buffer);

  if( static_cast<unsigned int>( atoi( buffer ) ) != pressureStage ) {
    std::cerr << " [JulaboFP50::SetPumpPressure] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: P=" << pressureStage << " but received (string):"
              << buffer << "." << std::endl;
    return false;
  }
  
  if (alwaysEmit_ or pumpPressure_ != pressureStage)
    emit pumpPressureChanged(pressureStage);
  pumpPressure_ = pressureStage;

  return true;
}

///
/// return success flag
///
bool JulaboFP50::SetCirculatorOn( void ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::SetCirculatorOn] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];
  
  SetAndConfirm("out_mode_05 1", "in_mode_05", buffer);
  comMutex_.lock();
  comHandler_->SendCommand( "out_mode_05 1" );
  usleep( 20000 );
  comHandler_->SendCommand( "in_mode_05" );
  usleep( 10000 );
  comHandler_->ReceiveString( buffer );
  comMutex_.unlock();
  StripBuffer( buffer );

  if( static_cast<unsigned int>( atoi( buffer ) ) != 1 ) {
    std::cerr << " [JulaboFP50::SetCirculatorOn] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: ON (1) but received (string):" << buffer << "." << std::endl;
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
bool JulaboFP50::SetCirculatorOff( void ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::SetCirculatorOf] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];
  
  SetAndConfirm("out_mode_05 0", "in_mode_05", buffer);

  if( static_cast<unsigned int>( atoi( buffer ) ) != 0 ) {
    std::cerr << " [JulaboFP50::SetCirculatorOff] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: OFF (0) but received (string):" << buffer << "." << std::endl;
    return false;
  }
  
  if (alwaysEmit_ or circulatorOn_)
    emit circulatorStatusChanged(false);
  circulatorOn_ = false;

  return true;
}

///
/// pass pid parameters to machine;
/// return success flag
/// xp = prop. / tn = int / tv = diff
///
bool JulaboFP50::SetControlParameters( float xp, int tn, int tv ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::SetControlParameters] -- DEBUG: Called." << std::endl;
  #endif

  if( xp < 0.1f || xp > 99.9f ) {
    std::cerr << " [JulaboFP50::SetControlParameters] ** ERROR: Invalid proportional parameter xp: "
              << xp << "." << std::endl;
    return false;
  }
  
  if( tn < 3 || tn > 9999 ) {
    std::cerr << " [JulaboFP50::SetControlParameters] ** ERROR: Invalid integral parameter tn: "
              << tn << "." << std::endl;
    return false;
  }
  
  if( tv < 0 || tv > 999 ) {
    std::cerr << " [JulaboFP50::SetControlParameters] ** ERROR: Invalid differential parameter tv: "
              << tv << "." << std::endl;
    return false;
  }

  std::stringstream theCommand;
  char buffer[200];
  
  // proportional
  SetAndConfirm(theCommand.str().c_str(), "in_par_06", buffer);

  if( fabs( atof( buffer ) - xp ) > 1.e-3 ) {
    std::cerr << " [JulaboFP50::SetControlParameters] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: xp=" << xp << " but received (string):"
              << buffer << "." << std::endl;
    return false;
  }

  // integral
  SetAndConfirm(theCommand.str().c_str(), "in_par_07", buffer);

  if( atoi( buffer ) !=  tn ) {
    std::cerr << " [JulaboFP50::SetControlParameters] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: tn=" << tn << " but received (string):"
              << buffer << "." << std::endl;
    return false;
  }

  // differential
  SetAndConfirm(theCommand.str().c_str(), "in_par_08", buffer);

  if( atoi( buffer ) !=  tv ) {
    std::cerr << " [JulaboFP50::SetControlParameters] ** ERROR: check failed." << std::endl;
    std::cerr << "  > Expected: tv=" << tv << " but received (string):"
              << buffer << "." << std::endl;
    return false;
  }
  
  return true;
}

///
///
///
float JulaboFP50::GetBathTemperature( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetBathTemperature] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];

  GetValue("in_pv_00", buffer);
  float temp = atof( buffer );
  
  if (alwaysEmit_ or abs(temp - bathTemperature_) > TEMP_EPSILON)
    emit bathTemperatureChanged(temp);
  bathTemperature_ = temp;
  
  return temp;
}

///
///
///
float JulaboFP50::GetSafetySensorTemperature( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetSafetySensorTemperature] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];
  
  GetValue("in_pv_03", buffer);
  float temp = atof( buffer );
  
  if (alwaysEmit_ or abs(temp - sensorTemperature_) > TEMP_EPSILON)
    emit safetySensorTemperatureChanged(temp);
  sensorTemperature_ = temp;

  return temp;
}

///
///
///
float JulaboFP50::GetWorkingTemperature( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetWorkingTemperature] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];
  
  GetValue("in_sp_00", buffer);
  float temp = atof( buffer );
  
  if (alwaysEmit_ or abs(temp - workingTemperature_) > TEMP_EPSILON)
    emit workingTemperatureChanged(temp);
  workingTemperature_ = temp;

  return temp;
}

///
///
///
int JulaboFP50::GetHeatingPower( void ) const
{
  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetHeatingPower] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];

  GetValue("in_pv_01", buffer);

  return( atoi( buffer ) );
}

///
///
///
unsigned int JulaboFP50::GetPumpPressure( void ) const {
  
  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetPumpPressure] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];
  
  GetValue("in_sp_07", buffer);
  unsigned int pressure = stoi( buffer );
  
  if (alwaysEmit_ or pressure != pumpPressure_)
    emit pumpPressureChanged(pressure);
  pumpPressure_ = pressure;

  return pressure;
}

///
/// true = on / false = off
///
bool JulaboFP50::GetCirculatorStatus( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetCirculatorStatus] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];
  
  GetValue("in_mode_05", buffer);

  bool status = atoi( buffer );

  if (alwaysEmit_ or circulatorOn_ != status)
    emit circulatorStatusChanged(status);
  circulatorOn_ = status;
  
  return status;
}

///
/// get status message &
/// separate extracted status/error number
///
std::pair<int,std::string> JulaboFP50::GetStatus( void ) const {
  
  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetStatus] -- DEBUG: Called." << std::endl;
  #endif

  std::pair<int,std::string> returnValue;

  char buffer[1000];
  GetValue("status", buffer);

  std::string message( buffer );

  if( 0 == message.size() ) {
    std::cerr << " [JulaboFP50::GetStatus] ** WARNING: Machine returned empty message."
              << std::endl;
    return std::pair<int,std::string>( 0, std::string( "---" ) );
  }

  // error type messages begin with "-" (negative number)
  if( std::string( "-" ) == message.substr( 0, 1 ) ) {
    returnValue.first  = atoi( message.substr( 0, 3 ).c_str() );
    returnValue.second = message.substr( 4, 100 );
  }

  // normal (non-error) message, positive number
  else {
    returnValue.first  = atoi( message.substr( 0, 2 ).c_str() );
    returnValue.second = message.substr( 3, 100 );
  }

  return returnValue;
}

///
///
///
float JulaboFP50::GetProportionalParameter( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetProportionalParameter] -- DEBUG: Called."
            << std::endl;
  #endif

  char buffer[1000];

  GetValue("in_par_06", buffer);
  return( atof( buffer ) );
}

///
///
///
int JulaboFP50::GetIntegralParameter( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetIntegralParameter] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];

  GetValue("in_par_07", buffer);

  return( atoi( buffer ) );
}

///
///
///
int JulaboFP50::GetDifferentialParameter( void ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::GetDifferentialParameter] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[1000];

  GetValue("in_par_08", buffer);

  return( atoi( buffer ) );
}

///
/// save pid parameters to file
///
bool JulaboFP50::SaveControlParameters( const std::string& filepath ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::SaveControlParameters] -- DEBUG: Called." << std::endl;
  #endif

  std::ofstream file( filepath.c_str() );
  if ( file.bad() ) {
    std::cerr << " [JulaboFP50::SaveControlParameters] ** ERROR: Could not save to file:"
              << std::endl;
    std::cerr << "  > \"" << filepath.c_str() << "\"." << std::endl;
    return false;
  }

  file << "<PID_PROP> " << GetProportionalParameter() << std::endl;
  file << "<PID_INTL> " << GetIntegralParameter() << std::endl;
  file << "<PID_DIFF> " << GetDifferentialParameter() << std::endl;
  
  file.close();
  return true;
}

///
/// load pid parameters from file & pass to the machine;
/// return success flag
///
bool JulaboFP50::LoadControlParametersAndApply( const std::string& filepath ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::LoadControlParametersAndApply] -- DEBUG: Called."
            << std::endl;
  #endif

  std::ifstream file( filepath.c_str(), std::ios::in );
  if ( file.bad() ) {
    std::cerr << " [JulaboFP50::LoadControlParametersAndApply] ** ERROR: Could not read from file:"
              << std::endl;
    std::cerr << "  > \"" << filepath.c_str() << "\"." << std::endl;
    return false;
  }

  std::string item;
  float propP = 0.; bool isProp = false;
  int intlP = 0; bool isIntl = false;
  int diffP = 0; bool isDiff = false;

  while( ! ( ( isProp && isIntl && isDiff ) || file.eof() ) ) {

    file >> item;

    if( std::string::npos != item.find( "<PID_PROP>" ) ) {
      file >> propP;
      isProp = true;
    }

    if( std::string::npos != item.find( "<PID_INTL>" ) ) {
      file >> intlP;
      isIntl = true;
    }

    if( std::string::npos != item.find( "<PID_DIFF>" ) ) {
      file >> diffP;
      isDiff = true;
    }

  }

  file.close();

  if( isProp && isIntl && isDiff ) {
    SetControlParameters( propP, intlP, diffP );
    return true;
  } 
  else {
    std::cerr << " [LoadControlParametersAndApply] ** ERROR: could not parse input file: "
              << std::endl;
    std::cerr << "  > " << filepath.c_str() << "." << std::endl;
    return false;
  }
}

///
/// strip trailing newlines & stuff
/// from machine answer
///
void JulaboFP50::StripBuffer( char* buffer ) const {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::StripBuffer] -- DEBUG: Called." << std::endl;
  std::cout << "[JulaboFP50::StripBuffer] -- DEBUG: Buffer is:" << std::endl;
  std::cout << " > " << buffer << std::endl;
  #endif

  for( unsigned int c = 0; c < strlen( buffer ); ++c ) {
    if( '\n' == buffer[c] || '\r' == buffer[c] ) {
      buffer[c] = 0;
      break;
    }
  }
}

void JulaboFP50::GetValue(const char* command, char* buffer) const {
  comMutex_.lock();
  comHandler_->SendCommand( command );
  usleep( 10000 );
  comHandler_->ReceiveString( buffer );
  comMutex_.unlock();
  StripBuffer( buffer );
}

void JulaboFP50::SetAndConfirm(const char* first, const char* second, char* buffer) const {
  comMutex_.lock();
  comHandler_->SendCommand( first );
  usleep( 20000 );
  comHandler_->SendCommand( second );
  usleep( 10000 );
  comHandler_->ReceiveString( buffer );
  comMutex_.unlock();
  StripBuffer( buffer );
}

///
/// read back software version
/// to check communication with device
///
void JulaboFP50::Device_Init( void ) {

  #ifdef __JULABOFP50_DEBUG
  std::cout << "[JulaboFP50::Device_Init] -- DEBUG: Called." << std::endl;
  #endif

  char buffer[200];
  comHandler_->SendCommand( "version" );
  usleep( 200000 );
  comHandler_->ReceiveString( buffer );

  // std::cout << buffer << std::endl;

  if (std::string::npos==std::string(buffer).find( "JULABO TOPTECH-SERIES MC-2 VERSION")) {
    std::cerr << " [JulaboFP50::Device_Init] ** ERROR: Device communication problem."
              << std::endl;
    std::cerr << "  > (could not read software version string)" << std::endl;
    isCommunication_ = false;
    
    throw BurnInException("Invalid or no device at address of Julabo chiller");
  }

  isCommunication_ = true;
}
