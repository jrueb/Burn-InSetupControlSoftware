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

#ifndef _COMHANDLER_H_
#define _COMHANDLER_H_

#include <termios.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#define COM1 "/dev/ttyS0"
#define COM2 "/dev/ttyS1"
#define COM3 "/dev/ttyS2"
#define COM4 "/dev/ttyS3"

#define ttyS0 "/dev/ttyS0"
#define ttyS1 "/dev/ttyS1"
#define ttyS2 "/dev/ttyS2"
#define ttyS3 "/dev/ttyS3"

typedef const char* ioport_t;
typedef struct termios termios_t;

class ComHandler {

 public:
  
  //! Constructor.
  ComHandler( ioport_t, speed_t = B9600);

  //! Destructor.
  ~ComHandler();

  //! Default bitwise copy constructor.
  ComHandler( const ComHandler& );

  void SendCommand( const char*, bool sendfeed = true );
  void ReceiveString( char* );

  static constexpr int ComHandlerDelay = 1000;

 private:

  void OpenIoPort( void );
  void InitializeIoPort( speed_t baud );
  void RestoreIoPort( void );
  void CloseIoPort( void );

  int fIoPortFileDescriptor;

  ioport_t fIoPort;
  termios_t fCurrentTermios, fThisTermios;

};



#endif



